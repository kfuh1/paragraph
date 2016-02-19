#ifndef __PARAGRAPH_H__
#define __PARAGRAPH_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "vertex_set.h"
#include "graph.h"

#include "mic.h"

#include <stdio.h>
#include <iostream>

/*
 * edgeMap --
 * 
 * Students will implement this function.
 * 
 * The input argument f is a class with the following methods defined:
 *   bool update(Vertex src, Vertex dst)
 *   bool cond(Vertex v)
 *
 * See apps/bfs.cpp for an example of such a class definition.
 * 
 * When the argument removeDuplicates is false, the implementation of
 * edgeMap need not remove duplicate vertices from the VertexSet it
 * creates when iterating over edges.  This is a performance
 * optimization when the application knows (and can tell ParaGraph)
 * that f.update() guarantees that duplicate vertices cannot appear in
 * the output vertex set.
 * 
 * Further notes: the implementation of edgeMap is templated on the
 * type of this object, which allows for higher performance code
 * generation as these methods will be inlined.
 */
template <class F>
VertexSet *edgeMap(Graph g, VertexSet *u, F &f, bool removeDuplicates=true)
{
  // TODO: Implement
  //
  
  int count = 0;
  bool* results = (bool*)malloc(sizeof(bool) * u->numNodes);
  for(int i = 0; i < u->numNodes; i++){
    results[i] = false;
  }
  //for each vertex in the given set loop through all the out-neighbors
  //and apply f.cond and f.update
  for (int i = 0; i < u->capacity; i++) {
    Vertex srcVertex = u->vertices[i];
    if(srcVertex == -1){
      continue;
    }
    const Vertex* start = outgoing_begin(g, srcVertex);
    const Vertex* end = outgoing_end(g, srcVertex);
    for (const Vertex* v = start; v != end; v++) {
      if (f.cond(*v) && f.update(srcVertex, *v)) {
        results[*v] = true;
        count++;
      }
    }
  }
  VertexSet* vertexSet = newVertexSet(u->type, count, u->numNodes);
  for(int i = 0; i < u->numNodes; i++){
      if(results[i]){
          addVertex(vertexSet, i);
      }
  }

  //std::cout << "end edgeMap";
  
  return vertexSet;
}



/*
 * vertexMap -- 
 * 
 * Students will implement this function.
 *
 * The input argument f is a class with the following methods defined:
 *   bool operator()(Vertex v)
 *
 * See apps/kBFS.cpp for an example implementation.
 * 
 * Note that you'll call the function on a vertex as follows:
 *    Vertex v;
 *    bool result = f(v)
 *
 * If returnSet is false, then the implementation of vertexMap should
 * return NULL (it need not build and create a vertex set)
 */
template <class F>
VertexSet *vertexMap(VertexSet *u, F &f, bool returnSet=true)
{
  // TODO: Implement
  //
  int size = u->size;
  if (!returnSet) {
    for (int j = 0; j < size; j++) {
      //if (u->vertices[j] != -1) {
        f(u->vertices[j]);
      //}
    }
    return NULL;
  }
  
  bool* results = (bool*)malloc(sizeof(bool) * size);
  //int count = 0;
  
  for (int i = 0; i < size; i++) {
    //if (u->vertices[i] != -1) {
      results[i] = f(u->vertices[i]);
    /*  count++;
    } else {
      results[i] = false;
    }*/
  }

  VertexSet* vertexSet = newVertexSet(u->type, size, u->numNodes);
  
  //print the vertices in the set - debug
  for (int i = 0; i < size; i++) {
    if (results[i]) {
      //printf("vertex %d\n", u->vertices[i]);
      addVertex(vertexSet, u->vertices[i]);
    }
  }

  //std::cout << "end vertexMap";

  return vertexSet;
  
}

#endif /* __PARAGRAPH_H__ */
