#ifndef PROCESS_H
#define PROCESS_H
#include "epochtime.h"
#include "venmodata.h"
#include "venmoio.h"
#include "hashtable.h"

// For convenience
typedef unsigned int uint;

// node element holding string str for a person's name and node degree
class Node : public Content {
protected:
  std::string str;
  uint deg;
public:
  // Initialize new node with degree 1
  Node(std::string str, uint deg = 1): str(str), deg(deg) {};
  virtual std::string getStr() const;
  virtual uint getDeg() const;
  // increment and decrement degree ??? remove from ntab if zero
  virtual void incDeg();
  virtual void decDeg();
  virtual int compare(Content* content) const;
};

// Nodes per edge = edge nodes EN
#define EN 2

// edge element holding two nodes
class Edge : public Content {
protected:
  Node* nodes[EN];
public:
  Edge(Node* actor, Node* target)
    { nodes[0] = actor; nodes[1] = target; };
  virtual Node* getNode(int index) const;
  virtual void putNode(Node* node, int index);
  virtual int compare(Content* content) const;
};

class Graph {
protected:
  venmoio* vio;
  time_t currtime;
  uint currsec, edgenum, maxdeg, degsize;
  uint* degrees;
  Hashtable* etab;
  Hashtable* ntab;

public:
  Graph(venmoio* vio, time_t currtime = -MAXSEC, int currsec = -1, uint edgenum = 0, uint maxdeg = 1, uint degsize = 2048):
    vio(vio), currtime(currtime), edgenum(edgenum), currsec(currsec), degsize(degsize), maxdeg(maxdeg) {
    // Edge table indexed by second after the minute, 0 <= sec < MAXSEC
    // Increase to treat leap seconds separately.
    etab = new Hashtable(MAXSEC);
    // Hash table for nodes
    ntab = new Hashtable(htb::hashmask1 + 1);
    degrees = new uint[degsize]();
  };
  virtual ~Graph();
  virtual void incMaxdeg();
  virtual void decMaxdeg();
  virtual void evictExistingNode(Node* node);
  virtual void reduceEdgeNodes(Edge* edge);
  virtual void evictEdge(Edge* myedge, hashtype ehash);
  virtual void evictSectab(uint sec);
  virtual void evictAll();
  virtual void insertEdge(Edge* myedge, uint sec, hashtype ehash);
  virtual void process(venmodata* vdt);
  virtual void output();
  virtual void test_output();
};

#endif
