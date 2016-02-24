#include "vertex_set.h"

#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <stdio.h>
#include "mic.h"

// comment

/**
 * Creates an empty VertexSet with the given type and capacity.
 * numNodes is the total number of nodes in the graph.
 * 
 * Student may interpret type however they wish.  It may be helpful to
 * use different representations of VertexSets under different
 * conditions, and they different conditions can be indicated by 'type'
 */
VertexSet *newVertexSet(VertexSetType type, int capacity, int numNodes)
{
  // TODO: Implement
  VertexSet* vertexSet = (VertexSet*)malloc(sizeof(VertexSet));
  vertexSet->type = type;
  vertexSet->size = 0;
  vertexSet->capacity = capacity;
  vertexSet->numNodes = numNodes;
  
  vertexSet->verticesDense = (bool*)malloc(sizeof(bool) * numNodes);
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    vertexSet->verticesDense[i] = false;
  }
  vertexSet->verticesSparse = (Vertex*)malloc(sizeof(Vertex) * capacity);
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < capacity; i++){
    vertexSet->verticesSparse[i] = -1;
  }
  return vertexSet;
}

void freeVertexSet(VertexSet *set)
{
  if(set->type == DENSE){
    free(set->verticesDense);
  }
  else{
    free(set->verticesSparse);
  }
  free(set);
}

void addVertex(VertexSet *set, Vertex v)
{
  int size = set->size;
  int capacity = set->capacity;
  if(size >= capacity){
    return;
  }
  //assuming no duplicates ever added
  if(set->type == DENSE){
    set->verticesDense[v] = true;
    #pragma omp atomic
    set->size += 1;
  }
  else{
    set->verticesSparse[size] = v;
    set->size += 1;
    
  }
  
}

void removeVertex(VertexSet *set, Vertex v)
{
  //TODO not implemented yet because nothing calls this
  /*if(!set->vertices[v]){
    return;
  }
  set->vertices[v] = false;
  set->size -= 1;*/
}

void convertToDense(VertexSet *set){
  if(set->type == DENSE){
    return;
  }
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < set->numNodes; i++){
    set->verticesDense[i] = false; 
  }
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < set->size; i++){
    int v = set->verticesSparse[i];
    set->verticesDense[v] = true; 
  }
}

void convertToSparse(VertexSet *set){
  if(set->type == SPARSE){
    return;
  }
  int idx = 0;
  for(int i = 0; i < set->numNodes && idx < set->size; i++){
    if(set->verticesDense[i]){
      set->verticesSparse[idx] = i;
      idx++;
    }
  }
}

void scan(int length, int* output)
{
    int N = length;

    // upsweep phase.
    for (int twod = 1; twod < N; twod*=2) {
      int twod1 = twod*2;

      #pragma omp parallel for schedule(static)
      for (int i = 0; i < N; i += twod1) {
          output[i+twod1-1] += output[i+twod-1];
      }
    }
    
    output[N-1] = 0;
    
    // downsweep phase.
    for (int twod = N/2; twod >= 1; twod /= 2) {
      int twod1 = twod*2;

      #pragma omp parallel for schedule(static)
      for (int i = 0; i < N; i += twod1) {
          int t = output[i+twod-1];
          output[i+twod-1] = output[i+twod1-1];
          output[i+twod1-1] += t; // change twod1 to twod to reverse prefix sum.
      }
    }
}

/**
 * Returns the union of sets u and v. Destroys u and v.
 */
VertexSet* vertexUnion(VertexSet *u, VertexSet* v)
{
  // TODO: Implement

  // STUDENTS WILL ONLY NEED TO IMPLEMENT THIS FUNCTION IN PART 3 OF
  // THE ASSIGNMENT

  return NULL;
}

