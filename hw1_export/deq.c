// deq.c -- double-ended, doubly-linked queue of anonymous data.
//
// A queue is a Rep: end pointers ht[Head] and ht[Tail], plus a length.
// A node's np[Head] points toward the head and np[Tail] toward the
// tail; a missing neighbor is 0.  Indexing ht[] and np[] by the same
// End is what makes the ends symmetric: put, ith, get and rem are each
// written once, in terms of an end e and other(e), and the head and
// tail entry points differ only in the End they pass.
//
// The queue owns its nodes but never the data it is handed; freeing
// that data is the caller's job, via deq_del()'s map function.
//
// A zero queue or a failed malloc() is fatal.  An out-of-range index
// warns and returns 0.  Data that is simply absent -- get on an empty
// queue, rem of an unheld datum -- is not an error and returns 0.

#define _GNU_SOURCE             // asprintf(), strdup()

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "deq.h"
#include "error.h"

// indices and size of array of node pointers
typedef enum {Head,Tail,Ends} End;

typedef struct Node {
  struct Node *np[Ends];        // next/prev neighbors
  Data data;
} *Node;

typedef struct {
  Node ht[Ends];                // head/tail nodes
  int len;
} *Rep;

static Rep rep(Deq q) {
  if (!q) ERROR("zero pointer");
  return (Rep)q;
}

// the end opposite e
static End other(End e) { return e==Head ? Tail : Head; }

// Append a node holding d onto end e; len++.
static void put(Rep r, End e, Data d) {
  Node n=(Node)malloc(sizeof(*n));
  if (!n) ERROR("malloc() failed");
  n->data=d;
  n->np[e]=0;                   // nothing beyond the new end
  n->np[other(e)]=r->ht[e];     // the old end node, if any
  if (r->ht[e])
    r->ht[e]->np[e]=n;
  else
    r->ht[other(e)]=n;          // empty: n is both ends
  r->ht[e]=n;
  r->len++;
}

// Return the data i nodes in from end e, or 0 if i is out of range.
static Data ith(Rep r, End e, int i) {
  if (i<0 || i>=r->len) {
    WARN("index %d out of range [0,%d)",i,r->len);
    return 0;
  }
  Node n=r->ht[e];
  for ( ; i>0; i--)
    n=n->np[other(e)];          // step away from end e
  return n->data;
}

// Splice n out of r and free it, returning its data; len--.
static Data unput(Rep r, Node n) {
  Data d=n->data;
  for (End e=Head; e<Ends; e++) {
    Node nb=n->np[e];           // n's neighbor toward end e
    if (nb)
      nb->np[other(e)]=n->np[other(e)];
    else
      r->ht[e]=n->np[other(e)]; // n was end e
  }
  free(n);
  r->len--;
  return d;
}

// Remove the node at end e and return its data, or 0 if empty; len--.
static Data get(Rep r, End e) {
  Node n=r->ht[e];
  return n ? unput(r,n) : 0;
}

// Remove the node nearest end e whose data == d and return d,
// or 0 if there is none; len-- iff found.
static Data rem(Rep r, End e, Data d) {
  for (Node n=r->ht[e]; n; n=n->np[other(e)])
    if (n->data==d)
      return unput(r,n);
  return 0;
}

extern Deq deq_new() {
  Rep r=(Rep)malloc(sizeof(*r));
  if (!r) ERROR("malloc() failed");
  r->ht[Head]=0;
  r->ht[Tail]=0;
  r->len=0;
  return r;
}

extern int deq_len(Deq q) { return rep(q)->len; }

extern void deq_head_put(Deq q, Data d) {        put(rep(q),Head,d); }
extern Data deq_head_get(Deq q)         { return get(rep(q),Head);   }
extern Data deq_head_ith(Deq q, int i)  { return ith(rep(q),Head,i); }
extern Data deq_head_rem(Deq q, Data d) { return rem(rep(q),Head,d); }

extern void deq_tail_put(Deq q, Data d) {        put(rep(q),Tail,d); }
extern Data deq_tail_get(Deq q)         { return get(rep(q),Tail);   }
extern Data deq_tail_ith(Deq q, int i)  { return ith(rep(q),Tail,i); }
extern Data deq_tail_rem(Deq q, Data d) { return rem(rep(q),Tail,d); }

extern void deq_map(Deq q, DeqMapF f) {
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail])
    f(n->data);
}

extern void deq_del(Deq q, DeqMapF f) {
  if (f) deq_map(q,f);
  Node curr=rep(q)->ht[Head];
  while (curr) {
    Node next=curr->np[Tail];
    free(curr);
    curr=next;
  }
  free(q);
}

extern Str deq_str(Deq q, DeqStrF f) {
  char *s=strdup("");
  for (Node n=rep(q)->ht[Head]; n; n=n->np[Tail]) {
    char *d=f ? f(n->data) : n->data;
    char *t; asprintf(&t,"%s%s%s",s,(*s ? " " : ""),d);
    free(s); s=t;
    if (f) free(d);
  }
  return s;
}
