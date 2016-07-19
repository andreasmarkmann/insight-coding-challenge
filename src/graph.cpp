#include <iostream>      // std::cout
#include <sstream>       // std::ostringstream
#include <cassert>       // assert
#include <climits>       // std::cout
#include "stringutils.h"
#include "epochtime.h"
#include "venmodata.h"
#include "venmoio.h"
#include "hashtable.h"
#include "graph.h"

inline std::string Node::getStr() const {
  return str;
}

inline uint Node::getDeg() const {
  return deg;
}

inline void Node::incDeg() {
  ++deg;
}

inline void Node::decDeg() {
  --deg;
}

inline int Node::compare(Content* content) const {
  Node* node = dynamic_cast<Node*>(content);
  return str.compare(node->getStr());
}

inline Node* Edge::getNode(int index) const {
  return nodes[index];
}

inline void Edge::putNode(Node* node, int index) {
  nodes[index] = node;
}

inline int Edge::compare(Content* content) const {
  Edge* edge = dynamic_cast<Edge*>(content);
  int comp1 = nodes[0]->getStr().compare(edge->nodes[0]->getStr());
  if( comp1 ) {
    return comp1;
  } else {
    return nodes[1]->getStr().compare(edge->nodes[1]->getStr());
  }
}


// Database destructor
Graph::~Graph() {
  // must be executed in this order, as ~etab evicts elements in ntab
  evictAll();
  delete [] degrees;
  delete etab;
  delete ntab;
}

// decrement and increment maximum detected node degree
inline void Graph::decMaxdeg() {
  maxdeg--;
}

void Graph::incMaxdeg() {
  maxdeg++;
  // Check if we need to reallocate array holding degrees
  // and realloc the C++ way - this should be a rare occurence
  // since we straight-up double the size each time
  if( maxdeg + 1 >= degsize ) {
    if( maxdeg >= (UINT_MAX >> 1) ) {
      stu::abortf("Detected extremely high node degree, aborting.\n");
    }
    uint newdegsize = degsize << 1;
    // realloc the C++ way, i.e. without using realloc
    // alloc new array and fill with zeroes ()
    uint* temp = new uint[newdegsize]();
    // copy existing degrees array, including new maxdeg degree
    for(uint ii = 0; ii <= maxdeg; ii++) {
      temp[ii] = degrees[ii];
    }
    delete [] degrees;
    degrees = temp;
  }
}

// Processing of incoming transaction data
void Graph::process(venmodata* vdt) {
  // create objects and update data structures
  time_t timediff = vdt->epochtime - currtime;
  // Ignore MAXSEC and larger difference in the past direction
  // to keep only seconds within one minute
  if( timediff <= -MAXSEC ) {
    // new data is too old to insert, ignore
    return;
  }
  if( timediff > 0) {
    if( timediff >= MAXSEC ) {
      // It's our lucky day, new data is one minute or more newer
      // than all current data and we get to evict all data without
      // any bookkeeping! Yay!
//       std::cout << "Calling evictAll, YAY!" << std::endl;
      evictAll();
    } else {
      // New data is a few seconds newer than any existing data,
      // we get to evict whole seconds of data with only some
      // amount of bookkeeping necessary. Not bad.

      // Recall that etab array is cyclic, i.e. index
      // (currsec + 1) % MAXSEC refers to oldest data 59 seconds ago.
      // Starting at index = (currsec + 1), iterate until we reach
      // (index % MAXSEC) == vdt->sec
      // as that is just 59 seconds older than vdt->sec.
      uint newsec = vdt->sec;
      if( currsec > newsec ) {
        // As we are in else brach, wrap through zero, add just one minute
        newsec += MAXSEC;
      }
//       std::cout << "Calling evictSectab from " << currsec
//       << " to " << vdt->sec << " aka " << newsec << std::endl;
      for( uint index = currsec + 1; index <= newsec; index++ ) {
        evictSectab(index % MAXSEC);
      }
    }
    // new data estabishes new, more recent, current time
    currtime = vdt->epochtime;
    currsec = vdt->sec;
  }
  hashtype ehash = htb::mkhash2(vdt->actor, vdt->target);
  Node* mynode[EN] = {new Node(vdt->actor), new Node(vdt->target)};
  Edge* myedge = new Edge(mynode[0], mynode[1]);
//   std::cout << "Inserting at hash " << ehash << std::endl;
  insertEdge(myedge, vdt->sec, ehash);

  // test output
//   Hashtable* mysectab =
//     dynamic_cast<Hashtable*>(etab->getContent(vdt->sec));
//   List* mylist = dynamic_cast<List*>(mysectab->getContent(ehash));
//   Edge* mymyedge = dynamic_cast<Edge*>(mylist->getContent());
//   for(int ii = 0; ii < EN; ii++) {
//     std::cout << "Inserted " << mymyedge->getNode(ii)->getStr() << " ("
//     << mymyedge->getNode(ii)->getDeg() << ")" << std::endl;
//   }
}

// Evict a single node from database and update degrees array
// Node must exist!!!
void Graph::evictExistingNode(Node* node) {
  hashtype nhash = htb::mkhash1(node->getStr());
  List* mylist = dynamic_cast<List*>(ntab->getContent(nhash));
  assert( NULL != mylist );
  List* beflist = mylist->findBef(node);
  assert( NULL != beflist );
  assert( 0 == node->compare(beflist->getContent()) );
  ntab->evictListitem(beflist, nhash);
}

void Graph::reduceEdgeNodes(Edge* edge) {
//   std::cout << "Found match "
//   << myedge->getNode(0)->getStr() << "("
//   << myedge->getNode(0)->getDeg() << "), "
//   << myedge->getNode(1)->getStr() << "("
//   << myedge->getNode(1)->getDeg() << "), "
//   << "at " << mysec << " secs, hash " << ehash << std::endl;
  uint mydeg;
  for(int ii = 0; ii < EN; ii++) {
    mydeg = edge->getNode(ii)->getDeg();
    degrees[mydeg]--;
    if( mydeg == maxdeg && 0 == degrees[mydeg] ) {
      decMaxdeg();
    }
    edge->getNode(ii)->decDeg();
    mydeg = edge->getNode(ii)->getDeg();
    // If node degree reaches zero, evict it, else update degrees
    if( 0 == mydeg ) {
      evictExistingNode(edge->getNode(ii));
    } else {
      degrees[edge->getNode(ii)->getDeg()]++;
    }
  }
}

// Evict from database one edge that matches edge passed as parameter
// Matching edge needs not exist but at most one may exist
void Graph::evictEdge(Edge* myedge, hashtype ehash) {

  // check if this hash/names exist at ANY PREVIOUS second
  // At every != check we can abort, gaining efficiency
  // Do for all seconds including current, it doesn't matter
  for(uint mysec = 0; mysec < MAXSEC; mysec++ ) {
    // does the older second have data?
    Hashtable* mysectab = dynamic_cast<Hashtable*>(etab->getContent(mysec));
    if( NULL != mysectab ) {
      // Does the tab at the older second have data at this hash?
      List* mylist = dynamic_cast<List*>(mysectab->getContent(ehash));
      if( NULL != mylist ) {
        // Are there items that are smaller or equal to our item?
        List* beflist = mylist->findBef(myedge);
        if( NULL != beflist ) {
          // Is it a match?
          if( 0 == myedge->compare(beflist->getContent()) ) {
            // Evict this edge
            edgenum--;
            reduceEdgeNodes( dynamic_cast<Edge*>(beflist->getContent()) );
            mysectab->evictListitem(beflist, ehash);
            // At most one match in database, so exit function here
            return;
          }
        }
      }
    }
  }

}

// Evict from database entire second edge database and reduceEdgeNodes
// This is faster than evicting edges individually, as we don't need to
// maintain valid linked lists
void Graph::evictSectab(uint sec) {

  Hashtable* mysectab = dynamic_cast<Hashtable*>(etab->getContent(sec));
  if( NULL == mysectab ) {
    // Nothing to do
    return;
  }
  for(hashtype hash = 0; hash <= htb::hashmask2; hash++) {
    // Does the tab at have data at this hash?
    List* mylist = dynamic_cast<List*>(mysectab->getContent(hash));
    // Iterate over list items
    while( NULL != mylist ) {
      // Evict this edge
      edgenum--;
      Edge* myedge = dynamic_cast<Edge*>(mylist->getContent());
      reduceEdgeNodes( myedge );
      delete myedge;

      List* newlist = mylist->getNext();
      delete mylist;
      mylist = newlist;
    }
    // May overwrite list pointer in hash table to NULL but will be deleted
    // mysectab->putContent(NULL, hash);
  }
  // delete and evict whole sectab from etab hash table
  delete mysectab;
  etab->putContent(NULL, sec);

}

// Evict from database entire database including edges
// This is faster than evicting edges individually, as we don't need to
// maintain valid linked lists and node data
void Graph::evictAll() {
  // Delete all nodes independently of edges
  for(hashtype hash = 0; hash <= htb::hashmask1; hash++) {
    // Does the tab at have data at this hash?
    List* mylist = dynamic_cast<List*>(ntab->getContent(hash));
    // Iterate over list items
    while( NULL != mylist ) {
      // Evict this node
      delete dynamic_cast<Node*>(mylist->getContent());

      List* newlist = mylist->getNext();
      delete mylist;
      mylist = newlist;
    }
    // May overwrite list pointer in hash table to NULL but will be deleted
    ntab->putContent(NULL, hash);
  }
  // delete all edges independently of nodes
  for(uint sec = 0; sec < MAXSEC; sec++) {
    Hashtable* mysectab = dynamic_cast<Hashtable*>(etab->getContent(sec));
    if( NULL != mysectab ) {
      for(hashtype hash = 0; hash <= htb::hashmask2; hash++) {
        // Does the tab at have data at this hash?
        List* mylist = dynamic_cast<List*>(mysectab->getContent(hash));
        // Iterate over list items
        while( NULL != mylist ) {
          // Evict this edge
          delete dynamic_cast<Edge*>(mylist->getContent());

          List* newlist = mylist->getNext();
          delete mylist;
          mylist = newlist;
        }
        // May overwrite list pointer in hash table to NULL but will be deleted
        // mysectab->putContent(NULL, hash);
      }
      // delete and evict whole sectab from etab hash table
      delete mysectab;
      etab->putContent(NULL, sec);
    }
  }
  edgenum = 0;
  // Reset degree data and maxdeg
  for(uint deg = 0; deg <= maxdeg; deg++) {
    degrees[deg] = 0;
  }
  maxdeg = 1;
}

// Insert new incoming edge:
// Evict edge connecting the same nodes if found in database.
// Insert nodes or obtain existing nodes from node table.
// If needed, create entry in top level Hash table entry (which is
// a second level hash table), and pass on insertion task to second level
// function Hashtable::insertListContent
void Graph::insertEdge(Edge* myedge, uint sec, hashtype ehash) {

  evictEdge(myedge, ehash);
//   std::cout << "State after evicting existing:" << std::endl;
//   test_output();

  // Insert nodes and check if they pre-existed
  for(int ii = 0; ii < EN; ii++) {
    Node* resnode = dynamic_cast<Node*>( ntab->insertListContent(
      myedge->getNode(ii), htb::mkhash1( myedge->getNode(ii)->getStr() ) ) );
    // If name matches, existing node is returned and fresh node is deleted
    if( myedge->getNode(ii) != resnode ) {
      myedge->putNode(resnode, ii);
//       std::cout << "Incrementing existing node " << resnode->getStr()
//       << "(" << resnode->getDeg() << ")" << std::endl;
      // oldnode contains old degree, decrement list occupation
      degrees[resnode->getDeg()]--;
      // increment degree
      resnode->incDeg();
      // increment new degree list occupation
      degrees[resnode->getDeg()]++;
      // increment max degree if needed
      if( resnode->getDeg() > maxdeg ) {
        incMaxdeg();
      }
//       std::cout << "Updated node " << resnode->getStr()
//       << "(" << resnode->getDeg() << ")" << std::endl;
    } else {
      // we inserted a new node, increase number of deg 1 nodes
      degrees[1]++;
    }
  }
//   std::cout << "Updated myedge "
//     << myedge->getNode(0)->getStr() << "("
//     << myedge->getNode(0)->getDeg() << "), "
//     << myedge->getNode(1)->getStr() << "("
//     << myedge->getNode(1)->getDeg() << "), "
//     << std::endl;

  // Get current second's edge hash table
  Hashtable* sectab = dynamic_cast<Hashtable*>(etab->getContent(sec));
  // Create if no hashtable exists at this second
  if( NULL == sectab ) {
    // No data at this second
    edgenum++;
    sectab = new Hashtable(htb::hashmask2 + 1);
    // We know there are no entries in this new table,
    // so go ahead and create everything
    sectab->putContent(new List(myedge), ehash);
    etab->putContent(sectab, sec);
  } else {
    Edge* resedge =
      dynamic_cast<Edge*>( sectab->insertListContent( myedge, ehash ) );
    // If we didn't get back existing edge, new edge was inserted
    if( myedge == resedge ) {
      edgenum++;
    }
//     std::cout << "Inserted myedge "
//       << resedge->getNode(0)->getStr() << "("
//       << resedge->getNode(0)->getDeg() << "), "
//       << resedge->getNode(1)->getStr() << "("
//       << resedge->getNode(1)->getDeg() << "), "
//       << std::endl;

  }
}

// Output statistics on number of degrees
void Graph::output() {
  assert( 0 == degrees[0] );

  // Collect the sum of occupation numbers of degrees,
  // in effect counting nodes
  uint ii;
  unsigned long long int sum = 0;
  for(ii = 0; ii <= maxdeg; ii++) {
    sum += degrees[ii];
  }

  // Go through occupation numbers and break if we pass half
  unsigned long long int sum2 = 0;
  for(ii = 0; ii <= maxdeg; ii++) {
    sum2 += 2*degrees[ii];
    if( sum2 >= sum ) {
      break;
    }
  }

  // If we broke at exactly half sum, we are between occupation numbers,
  // so print (ii).50, otherwise we are beyond half, print (ii).00
  std::ostringstream outstr;
  outstr << ii << "." << ((sum2 > sum) ? "0" : "5") << "0" << std::endl;
  vio->outStr(outstr.str());
}

// Unit testing output function follows
// Output statistics on number of degrees
void Graph::test_output() {
  assert( 0 == degrees[0] );

  // Collect the sum of occupation numbers of degrees,
  // in effect counting nodes
  uint ii;
  unsigned long long int sum = 0;
  std::cout << "degree occupations: ";
  for(ii = 0; ii <= maxdeg; ii++) {
    sum += degrees[ii];
    std::cout << degrees[ii] << " ";
  }
  std::cout << std::endl;

  // Go through occupation numbers and break if we pass half
  unsigned long long int sum2 = 0;
  for(ii = 0; ii <= maxdeg; ii++) {
    sum2 += 2*degrees[ii];
    if( sum2 >= sum ) {
      break;
    }
  }

  // If we broke at exactly half sum, we are between occupation numbers,
  // so print (ii).50, otherwise we are beyond half, print (ii).00
  std::ostringstream outstr;
  outstr << ii << "." << ((sum2 > sum) ? "0" : "5") << "0" << std::endl;
  std::cout << "Median = " << outstr.str();
}

