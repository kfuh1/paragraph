#ifndef __PARAGRAPH_H__
#define __PARAGRAPH_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "vertex_set.h"
#include "graph.h"

#include "mic.h"

#include <stdio.h>
#include <omp.h>

// comment

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
static VertexSet *edgeMap(Graph g, VertexSet *u, F &f,
    bool removeDuplicates=true)
{
  // TODO: Implement
  int numNodes = u->numNodes;
  int size = u->size;
  int outSize = 0;

  int n = numNodes;

  /* Helper function to round up to a power of 2. */
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  int* results = (int*)malloc(sizeof(int) * num_nodes(g));
  int* scanResults = (int*)malloc(n * sizeof(int));

  //get number of outgoing edges
  if(u->type == DENSE){
    #pragma omp parallel for reduction(+:outSize) schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(u->verticesDense[i]){
        int nodeOutSize = outgoing_size(g,i);
        outSize += nodeOutSize; 
      }
    }
  }
  else{
    #pragma omp parallel for reduction(+:outSize) schedule(static)
    for(int i = 0; i < size; i++){
      int nodeOutSize = outgoing_size(g, u->verticesSparse[i]);
      outSize += nodeOutSize;
    }
  }


  VertexSet* set;
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    results[i] = 0;
  } 
  int cmpSize = outSize + 3 * size;
  //Bottom up
  if(cmpSize > numNodes / (4 * omp_get_num_threads())){
    set = newVertexSet(DENSE, numNodes, numNodes);
    convertToDense(u); 

    //mark off the vertices in results that should be added to the set
    //this will ensure no duplicates are added in the following loop
    #pragma omp parallel for schedule(dynamic, 256) 
    for(int i = 0; i < numNodes; i++){ 
      if(!f.cond(i)){
        continue;
      }     
      const Vertex* start = incoming_begin(g, i);
      const Vertex* end = incoming_end(g, i);

      for(const Vertex* v = start; v < end; v++){
        if(u->verticesDense[*v] && f.update(*v, i)){
          results[i] = 1;
        }
      }
    }
    //no need to make critical
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(results[i]){
        addVertex(set, i);
      }
    }
  }
  //Top down
  else{
    int count = 0; 
    convertToSparse(u);
    #pragma omp parallel for schedule(dynamic, 256)
    for(int i = 0; i < u->size; i++){
      Vertex src = u->verticesSparse[i];
      const Vertex* start = outgoing_begin(g, src);
      const Vertex* end = outgoing_end(g, src);

      for(const Vertex* v = start; v < end; v++){
        if(f.cond(*v) && f.update(src, *v)){
          results[*v] = 1;
          #pragma omp atomic
          count++;
        }
      }
      
    }


    set = newVertexSet(SPARSE, count, numNodes);

    /*#pragma omp parallel for schedule(static)
    for(int i = numNodes; i < n; i++) {
      scanResults[i] = 0;
    }*/

    #pragma omp parallel for schedule(static)
    for(int j = 0; j < numNodes; j++) {
      scanResults[j] = results[j];
    }
    
    scan(n, scanResults);
     
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++) {
      if(results[i]) {
        int index = scanResults[i];
        set->verticesSparse[index] = i;
      }
    }
    
    set->size = count;

  }
  free(scanResults);
  free(results);
  return set;
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
static VertexSet *vertexMap(VertexSet *u, F &f, bool returnSet=true)
{
  // TODO: Implement
  int numNodes = u->numNodes;
  int n = numNodes;
  n--;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n++;
  int* results = (int*)malloc(sizeof(int) * numNodes);
  int* scanResults = (int*)malloc(n * sizeof(int));
  if(!returnSet){
    if(u->type == DENSE){
      #pragma omp parallel for schedule(static)
      for(int i = 0; i < numNodes; i++){
        if(u->verticesDense[i]){
          f(i);
        }
      }
    }
    else{
      #pragma omp parallel for schedule(static)
      for(int i = 0; i < u->size; i++){
        f(u->verticesSparse[i]);
      }
    }
    return NULL;
  }

  VertexSet* set;
  if(u->type == DENSE){
    set = newVertexSet(u->type, numNodes, numNodes);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(u->verticesDense[i] && f(i)){
        addVertex(set, i);
      }
    }
  }
  else{
    int count = 0;
    //make a guess at size of new set; can only be as big as original
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < u->size; i++){
      if(f(u->verticesSparse[i])){
        results[u->verticesSparse[i]] = 1;
        #pragma omp atomic
        count++;
      }
    }
    set = newVertexSet(u->type, count, numNodes);
    #pragma omp parallel for schedule(static)
    for(int j = 0; j < numNodes; j++) {
      scanResults[j] = results[j];
    }
    
    scan(n, scanResults);
     
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++) {
      if(results[i]) {
        int index = scanResults[i];
        set->verticesSparse[index] = i;
      }
    }
    
    set->size = count;

  }
  free(results);
  free(scanResults);
  return set;
}

#endif /* __PARAGRAPH_H__ */
