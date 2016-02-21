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
static VertexSet *edgeMap(Graph g, VertexSet *u, F &f,
    bool removeDuplicates=true)
{
  // TODO: Implement
  int size = u->size;
  int numNodes = u->numNodes;
  int outgoingCount = 0;
  if(u->type == SPARSE){
    #pragma omp parallel for
    for(int i = 0; i < size; i++){
      #pragma omp atomic
      outgoingCount += outgoing_size(g, u->vertices[i]);
    }
  }  
  else{
    #pragma omp parallel for
    for(int i = 0; i < numNodes; i++){
      if(u->verticesDense[i]){
        #pragma omp atomic
        outgoingCount += outgoing_size(g,i);
      }
    }
  }
  double ratio = ((double)size)/((double)outgoingCount);
  int count = 0;
  bool* results = (bool*)malloc(sizeof(bool) * numNodes);
  VertexSet* vertexSet; 
  #pragma omp parallel for  
  for(int i = 0; i < numNodes; i++) {
    results[i] = false;
  }

  if(u->type == DENSE){
    //printf("DENSE\n");
    #pragma omp parallel for
    for (int i = 0; i < numNodes; i++) {
      Vertex srcVertex = i;
      const Vertex* start = incoming_begin(g, srcVertex);
      const Vertex* end = incoming_end(g, srcVertex);
    
      for (const Vertex* v = start; v != end; v++) {
        /*if(u->type == SPARSE){
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
        else{*/
          //if v is part of current frontier and conds pass
        if(u->verticesDense[*v] && f.cond(i) && f.update(*v, i) && !results[i]){
          results[i] = true;
          #pragma omp atomic
          count++;
        }          
        //}
      }
    }
    if(count >= u->size){ 
      vertexSet = newVertexSet(DENSE, count, u->numNodes);
    }
    else{
      vertexSet = newVertexSet(SPARSE, count, u->numNodes);
    }
    #pragma omp parallel for 
    for(int i = 0; i < u->numNodes; i++) {
      if(results[i]) {
        #pragma omp critical 
        addVertex(vertexSet, i);
      }
    }

  }
  else{
    //printf("SPARSE\n");
    //TOP DOWN
    if(ratio < 1.0){
      vertexSet = newVertexSet(DENSE, u->numNodes, u->numNodes);
    }
    else{
      vertexSet = newVertexSet(SPARSE, u->numNodes, u->numNodes);
    }
    
    //for each vertex in the given set loop through all the out-neighbors
    //and apply f.cond and f.update
  
    #pragma omp parallel for 
    for (int i = 0; i < u->capacity; i++) {
      Vertex srcVertex = u->vertices[i];
      if(i >= u->size){
        continue;
      }
      const Vertex* start = outgoing_begin(g, srcVertex);
      const Vertex* end = outgoing_end(g, srcVertex);
      for (const Vertex* v = start; v != end; v++) {
        #pragma omp critical
        if (f.cond(*v) && f.update(srcVertex, *v) && !results[*v]) {
          results[*v] = true;
          addVertex(vertexSet, *v);
        }
      }
    }
  }
  
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
static VertexSet *vertexMap(VertexSet *u, F &f, bool returnSet=true)
{
  // TODO: Implement
   
  int size = u->size;
  bool* dups = (bool*)malloc(sizeof(bool) * u->numNodes);
  VertexSet* vertexSet;
  #pragma omp parallel for 
  for(int i = 0; i < u->numNodes; i++){
    dups[i] = false;
  }
  if(u->type == SPARSE){
    if (!returnSet) {
      //#pragma omp parallel for  
      for (int j = 0; j < size; j++) {
        f(u->vertices[j]);
      }
      free(dups);
      //#pragma omp barrier
      return NULL;
    }
  
    vertexSet = newVertexSet(u->type, size, u->numNodes);
   
    #pragma omp parallel for 
    for (int i = 0; i < size; i++) {
      #pragma omp critical  
      if (f(u->vertices[i]) && !dups[i]) {
        dups[i] = true;
        addVertex(vertexSet, u->vertices[i]);
      }
    }
  }
  else{
    if(!returnSet){
      for(int j = 0; j < u->numNodes; j++){
        if(u->verticesDense[j]){
          f(j);
        }
      }
      free(dups);
      return NULL;
    }
    vertexSet = newVertexSet(u->type, u->numNodes, u->numNodes);
    #pragma omp parallel for
    for(int i = 0; i < u->numNodes; i++){
      #pragma omp critical
      if(u->verticesDense[i] && f(i) && !dups[i]){
        dups[i] = true;
        addVertex(vertexSet, i);
      }
    }
  }

  free(dups);
  return vertexSet;
}

#endif /* __PARAGRAPH_H__ */
