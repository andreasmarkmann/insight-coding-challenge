#ifndef GRAPH_H
#define GRAPH_H
#include <cstddef>
#include <string>

typedef unsigned int hashtype;

// Empty abstract class for content of linked lists with
// virtual constructor to make polymorphic for downcasting
class Content  {
public:
  virtual int compare(Content* content) const;
  virtual ~Content() {};
};

namespace hashtable {
  extern const hashtype hashmask1;
  extern const hashtype hashmask2;
  hashtype mkhash1(std::string str);
  hashtype mkhash2(std::string str1, std::string str2);
}

// provide standardized shorthand namespace to save typing
namespace htb = hashtable;

// abstract linked list type for holding any type of element
// allowing hash collision, aka bucket
class List : public Content {
protected:
  Content* content;
  List* prev;
  List* next;
public:
  List(Content* content, List* prev = NULL, List* next = NULL):
    content(content), prev(prev), next(next) {};
  virtual Content* getContent() const;
  virtual List* getPrev() const;
  virtual List* getNext() const;
  virtual void putPrev(List* list);
  virtual void putNext(List* list);
  virtual List* findBef(Content* mycon);
};

// Hash table class derived from Content, so we can have hash tables
// containing hash tables. Never need to store hash, as it can be
// computed from location in hash table by pointer arithmetic.
class Hashtable : public Content {
protected:
  std::size_t size;

public:
  Content** table;

  // initializer list instead of constructor
  Hashtable(size_t size):
    size(size) {
      // allocate table and initialize with NULL pointers
      table = new Content*[size]();
    };
  virtual ~Hashtable();
  virtual void putContent(Content* content, hashtype hash);
  virtual Content* getContent(hashtype hash) const;
  virtual Content* insertListContent(Content* mycon, hashtype hash);
  virtual void evictListitem(List* mylist, hashtype hash);
};


#endif
