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
  int numNodes = u->numNodes;
  int numEdges = num_edges(g);
  int size = u->size;
  int outSize = 0;
  
  //stores the results of applying functions on vertices
  //and scanning those results
  int* results = (int*) malloc(sizeof(int) * numNodes);

  //get number of outgoing edges to be used for top down - bottom up heuristic
  //and determining chunk size
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
  
  //determining an appropriate chunk size based on the average number of edges
  //per node
  int chunkSize;
  int avgEdgeFrontier = outSize / size;
  int avgEdgeGraph = numEdges / numNodes;

  //casing on chunk size based on the average number of edges per node
  //compared to the average degree of a node in the graph
  if (avgEdgeFrontier > avgEdgeGraph){
    chunkSize = 512;
  } else {
    chunkSize = 128;
  }
  
  //threshold for top down = bottom up heuristic
  int threshold = 20;

  //Bottom up
  if(outSize > numEdges / threshold){
    int total = 0;
    
    //dense representation for bottom up implementation
    convertToDense(u);
    
    #pragma omp parallel for reduction(+:total) schedule(dynamic, chunkSize)
    for(int i = 0; i < numNodes; i++){
      int count = 0;
      const Vertex* start = incoming_begin(g,i);
      const Vertex* end = incoming_end(g,i);

      for(const Vertex* v = start; v < end; v++){
        if(!f.cond(i)){
          break;
        }
        if(u->verticesDense[*v] && f.update(*v, i) && !results[i]){
          results[i] = 1;
          count++;
        }
      }  
      total += count;
    }
    set = newVertexSet(DENSE, total, numNodes);
    //no critical section needed because dense rep
    //means all threads writing to their own space
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      set->verticesDense[i] = (results[i] == 1);
    }
    set->size = total;
  }
  //Top down
  else{
    int* scanResults = (int*) malloc(sizeof(int) * numNodes);
    int total = 0;
    
    //concerting to sparse for top-down implementation
    convertToSparse(u);

    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      scanResults[i] = 0;
    }

    #pragma omp parallel for reduction(+:total) schedule(dynamic,chunkSize)
    for(int i = 0; i < size; i++){
      int count = 0;
      Vertex src = u->verticesSparse[i];
      const Vertex* start = outgoing_begin(g, src);
      const Vertex* end = outgoing_end(g, src);

      for(const Vertex* v = start; v < end; v++){
        if(f.cond(*v) && f.update(src, *v) && !results[*v]){
          results[*v] = 1;
          scanResults[*v] = 1;
          count++;
        }
      }
      total += count;
    }
    set = newVertexSet(SPARSE, total, numNodes);

    //scanning in order to add to our sparse set in parallel
    scan(numNodes, scanResults);
    
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(results[i]){
        int idx = scanResults[i] - 1;
        set->verticesSparse[idx] = i;
      }
    }
    
    set->size = total;
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
  bool* results = (bool*) malloc(sizeof(bool) * numNodes);

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    results[i] = false;
  }

  //always create dense set
  VertexSet* set;
  int total = 0;
  if(u->type == DENSE){
    #pragma omp parallel for reduction(+:total) schedule(static)
    for(int i = 0; i < numNodes; i++){
      int count = 0;
      if(u->verticesDense[i] && f(i)){
        results[i] = true;
        count = 1;
      }
      total += count;
    }

    set = newVertexSet(DENSE, total, numNodes);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(results[i]){
        set->verticesDense[i] = true;
      }
    }
    set->size = total;
  }
  else{
    #pragma omp parallel for reduction(+:total) schedule(static)
    for(int i = 0; i < u->size; i++){
      int count = 0;
      if(f(u->verticesSparse[i])){
        results[u->verticesSparse[i]] = true;
        count = 1;
      }
      total += count;
    }
    set = newVertexSet(DENSE, total, numNodes);
     
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      if(results[i]){
        set->verticesDense[i] = true;
      }
    }
    set->size = total;
  }
  free(results);
  return set;
}

#endif /* __PARAGRAPH_H__ */
