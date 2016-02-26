#ifndef __PARAGRAPH_H__
#define __PARAGRAPH_H__

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <omp.h>
#include "vertex_set.h"
#include "graph.h"

#include "mic.h"

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
  //printf("edgemap %d\n", u->numNodes);
  int numNodes = u->numNodes;
  int size = u->size;
  int outSize = 0;
  //stores the results of applying functions on vertices
  //and scanning those results
  int* results = (int*) malloc(sizeof(int) * numNodes);
  //get number of outgoing edges to be used for top down - bottom up heuristic
  if(u->type == DENSE){
    #pragma omp parallel for reduction(+:outSize)
    for(int i = 0; i < numNodes; i++){
      if(u->verticesDense[i]){
        outSize += outgoing_size(g,i);
      }
    }
  }
  else{
    #pragma omp parallel for reduction(+:outSize)
    for(int i = 0; i < size; i++){
      outSize += outgoing_size(g, u->verticesSparse[i]);
    }
  }
  VertexSet* set;
  //zero out results to make sure clean before we use
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < u->numNodes; i++){
    results[i] = 0;
  }

  int threshold = 4 * omp_get_max_threads();
  //Bottom up
  if(outSize > numNodes / threshold){
  //if(false){
    //printf("bottom up\n");
    set = newVertexSet(DENSE, numNodes, numNodes);
    convertToDense(u);
    
    #pragma omp parallel for schedule(dynamic, 256)
    for(int i = 0; i < numNodes; i++){
      /*if(!f.cond(i)){
        continue;
      } */
      const Vertex* start = incoming_begin(g,i);
      const Vertex* end = incoming_end(g,i);

      for(const Vertex* v = start; v < end; v++){
        if(!f.cond(i)){
          break;
        }
        if(u->verticesDense[*v] && f.update(*v, i)){
          results[i] = 1;
          //break;
        }
      }  
    }
    //no critical section needed because dense rep
    //means all threads writing to their own space
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(results[i] == 1){
        addVertex(set, i);
      }
    }
  }
  //Top down
  else{
    //printf("top down");
    int* scanResults = (int*) malloc(sizeof(int) * numNodes);
    int count = 0;
    //printf("before\n");
    convertToSparse(u);

    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      scanResults[i] = 0;
    }

    //printf("size %d\n", size);
    #pragma omp parallel for schedule(dynamic, 256)
    for(int i = 0; i < size; i++){
      Vertex src = u->verticesSparse[i];
      const Vertex* start = outgoing_begin(g, src);
      const Vertex* end = outgoing_end(g, src);

      for(const Vertex* v = start; v < end; v++){
        if(f.cond(*v) && f.update(src, *v) && !results[*v]){
          //printf("vertex %d\n", *v);
          results[*v] = 1;
          scanResults[*v] = 1;
          #pragma omp atomic
          count++;
        }
      }
    }
    set = newVertexSet(SPARSE, count, numNodes);
    scan(numNodes, scanResults);
    //printf("count %d\n", count);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(results[i]){
        int idx = scanResults[i] - 1;
        //printf("adding %d at %d\n", i, idx);
        set->verticesSparse[idx] = i;
      }
    }
    
    
    set->size = count;
    free(scanResults);
  }
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
  //printf("vertexmap %d\n", u->numNodes);
  int numNodes = u->numNodes;
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
  //always create dense set
  VertexSet* set = newVertexSet(DENSE, u->size, numNodes);
  if(u->type == DENSE){
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(u->verticesDense[i] && f(i)){
        addVertex(set, i);
      }
    }
  }
  else{
    for(int i = 0; i < u->size; i++){
      if(f(u->verticesSparse[i])){
        addVertex(set, u->verticesSparse[i]);
      }
    }
  }
  //printf("vertexmap %d\n", set->numNodes);
  return set;
}

#endif /* __PARAGRAPH_H__ */
