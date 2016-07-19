#include <iostream>
#include <string>
#include "graph.h"
#include "stringutils.h"


// FNV-1a hash algorithm for short strings from
// http://www.isthe.com/chongo/tech/comp/fnv/
hashtype calc_hash(std::string str) {

// I will use 32 bit version, as the array will be smaller than that
#define FNV_OFFSET32 2166136261
#define FNV_OFFSET64 14695981039346656037
#define FNV_PRIME32 16777619
#define FNV_PRIME64 1099511628211

  hashtype hash = FNV_OFFSET32;
  for(std::string::iterator it = str.begin(); it != str.end(); ++it) {
    hash ^= *it;
    hash *= FNV_PRIME32;
  }
  return hash;
}

namespace hashtable {
  // bit mask for node hash table, table allocation will be for
  // hashmask1 + 1
  const hashtype hashmask1 = 0xFFFF;
  // use hashmask == 1 to provoke hash collisions for linked list testing
  // const hashtype hashmask1 = 1;
  // bit mask for edge hash table for each of 60 seconds = node mask/8
  const hashtype hashmask2 = (hashmask1 >> 3);
  // use hashmask == 1 to provoke hash collisions for linked list testing
  // const hashtype hashmask2 = 1;

  hashtype mkhash1(std::string str) {
    return ( calc_hash(str) ) & hashmask1;
  }

  hashtype mkhash2(std::string str1, std::string str2) {
    // Concatenate actor name and target name.
    // With actor and target previously lexicographically ordered,
    // this is symmetrized for non-directional edges.
    return ( calc_hash(str1 + str2) ) & hashmask2;
  }
}

// comparison operator for abstract Content class
// gets inherited by List and Hashtable
// get overridden by Node and Edge
inline int Content::compare(Content* content) const {
  return 0;
}

inline Content* List::getContent() const {
  return content;
}
inline List* List::getPrev() const {
  return prev;
}

inline List* List::getNext() const {
  return next;
}

inline void List::putPrev(List* list) {
  prev = list;
}

inline void List::putNext(List* list) {
  next = list;
}

// find item lexicographically before content item
List* List::findBef(Content* mycon) {
  // Find "before" list item of smaller or equal content
  List* beflist = NULL;
  List* mylist = this;
  while( NULL != mylist && 0 >= mycon->compare(mylist->getContent()) ) {
    beflist = mylist;
    mylist = mylist->getNext();
  }
  return beflist;
}

Hashtable::~Hashtable() {
  free(table);
}

inline void Hashtable::putContent(Content* content, hashtype hash) {
  table[hash] = content;
}

inline Content* Hashtable::getContent(hashtype hash) const {
  return table[hash];
}

void Hashtable::evictListitem(List* mylist, hashtype ehash) {
  List* prevlist = mylist->getPrev();
  List* nextlist = mylist->getNext();
  // First item in list?
  if( NULL == prevlist ) {
    // This may be NULL, which is fine
    putContent(nextlist, ehash);
    if( NULL != nextlist ) {
      nextlist->putPrev(NULL);
    }
  } else {
    // connect up previous
    prevlist->putNext(nextlist);
    if( NULL != nextlist ) {
      nextlist->putPrev(prevlist);
    }
  }
  // Next line could go into destructor of List class but then we'd rely on
  // having each element referenced only once, making reuse of code harder.
  delete mylist->getContent();
  delete mylist;
}

Content* Hashtable::insertListContent(Content* mycon, hashtype hash) {

  List* mylist = dynamic_cast<List*>(getContent(hash));
  // If there are few hash collisions, this is often NULL
  if( NULL == mylist ) {
    putContent(new List(mycon), hash);
  } else {
    // Hash collision - we hope this is rare unless same content
    // Search through doubly linked, lexicographically sorted list
    // Find "before" list item of smaller or equal content
    List* beflist = mylist->findBef(mycon);
    if( NULL == beflist ) {
      // First item is greater than mycon, insert mycon in front of
      // existing items, we know mylist != NULL
      List* newlist = new List(mycon);
      mylist->putPrev(newlist);
      newlist->putNext(mylist);
      // register newlist as first list element with hash table
      putContent(newlist, hash);
    } else {
      // Check if we have a match
      if( 0 == mycon->compare(beflist->getContent()) ) {
        // delete redundant mycon and return existing Content
        delete mycon;
        mycon = beflist->getContent();
      } else {
        // We are somewhere in the middle or at the end. Get next list element.
        mylist = beflist->getNext();
        List* newlist = new List(mycon);
        // If we are at the end, append.
        if( NULL != mylist ) {
          // Connect forwards
          mylist->putPrev(newlist);
          newlist->putNext(mylist);
        }
        // Always connect backwards
        beflist->putNext(newlist);
        newlist->putPrev(beflist);
      }
    }
  }

  return mycon;
}
