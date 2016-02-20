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
  
  /** TOP DOWN
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

  //BOTTOM UP
  int count = 0;
  bool* results = (bool*)malloc(sizeof(bool) * u->numNodes);
  
  #pragma omp parallel for  
  for(int i = 0; i < u->numNodes; i++) {
    results[i] = false;
  }
  
  #pragma omp parallel for
  for (int i = 0; i < u->numNodes; i++) {
    Vertex srcVertex = i;
    const Vertex* start = incoming_begin(g, srcVertex);
    const Vertex* end = incoming_end(g, srcVertex);
    
    for (const Vertex* v = start; v != end; v++) {
      for (int j = 0; j < u->size; j++) {
        if (u->vertices[j] == *v) {
          if (f.cond(i) && f.update(*v, i)) {
            results[i] = true;
            #pragma omp atomic
            count++;
          }   
        }
      }
    }
  }
  
  VertexSet* vertexSet = newVertexSet(u->type, count, u->numNodes);
  #pragma omp parallel for 
  for(int i = 0; i < u->numNodes; i++) {
    if(results[i]) {
      #pragma omp critical 
      addVertex(vertexSet, i);
    }
  }

  return vertexSet;**/
  
  int count = 0;
  bool* results = (bool*)malloc(sizeof(bool) * u->numNodes);
  //bool* dups = (bool*)malloc(sizeof(bool) * u->numNodes);
  #pragma omp parallel for 
  for(int i = 0; i < u->numNodes; i++){
    results[i] = false;
  }
  //for each vertex in the given set loop through all the out-neighbors
  //and apply f.cond and f.update
  
  #pragma omp parallel for 
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
        #pragma omp atomic
        count++;
      }
    }
  }
  VertexSet* vertexSet = newVertexSet(u->type, count, u->numNodes);

  #pragma omp parallel for 
  for(int i = 0; i < u->numNodes; i++){
      if(results[i]){
          #pragma omp critical
          addVertex(vertexSet, i);
      }
  }

  //std::cout << "end edgeMap";
  
  free(results);
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
  
  /** old implementation
  int size = u->size;

  if (!returnSet) {
    #pragma omp parallel for  
    for (int j = 0; j < size; j++) {
        f(u->vertices[j]);
    }
    return NULL;
  }
  
  bool* results = (bool*)malloc(sizeof(bool) * size);
  
  #pragma omp parallel for  
  for (int i = 0; i < size; i++) {
      results[i] = f(u->vertices[i]);
  }

  VertexSet* vertexSet = newVertexSet(u->type, size, u->numNodes);
  
  //print the vertices in the set - debug
  
  #pragma omp parallel for 
  for (int i = 0; i < size; i++) {
    if (results[i]) {
      #pragma omp critical  
      addVertex(vertexSet, u->vertices[i]);
    }
  }

  return vertexSet;**/
  
  int size = u->size;

  if (!returnSet) {
    #pragma omp parallel for  
    for (int j = 0; j < size; j++) {
        f(u->vertices[j]);
    }
    return NULL;
  }
  
  bool* dups = (bool*)malloc(sizeof(bool) * u->numNodes);
  bool* results = (bool*)malloc(sizeof(bool) * size);
  
  #pragma omp parallel for  
  for (int i = 0; i < size; i++) {
      results[i] = f(u->vertices[i]);
  }

  VertexSet* vertexSet = newVertexSet(u->type, size, u->numNodes);
  
  //print the vertices in the set - debug
  
  #pragma omp parallel for 
  for (int i = 0; i < size; i++) {
    if (results[i] && !dups[i]) {
      dups[i] = true;
      #pragma omp critical  
      addVertex(vertexSet, u->vertices[i]);
    }
  }
  
  free(dups);
  free(results);

  return vertexSet;
}

#endif /* __PARAGRAPH_H__ */
