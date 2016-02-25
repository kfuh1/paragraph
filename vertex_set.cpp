#include "vertex_set.h"

#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <stdio.h>
#include <omp.h>
#include "mic.h"

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
  /*if(set->type == SPARSE){
    return;
  }
  int numNodes = set->numNodes;
  int* scanResults = (int*) malloc(sizeof(int) * numNodes);
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    scanResults[i] = set->verticesDense[i];
  }

  scan(numNodes, scanResults);
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    if(set->verticesDense[i]){
      //-1 because we did inclusive scan
      int idx = scanResults[i] - 1;
      set->verticesSparse[idx] = i;
    }
  }
  free(scanResults);*/
}

//our histoscan
void scan(int length, int* output)
{
  int numBins = omp_get_max_threads();
  int* bins = (int*)malloc(sizeof(int) * numBins);
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numBins; i++){
    bins[i] = 0;
  }
 
  int binSize = (length + numBins - 1) / numBins;
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numBins; i++){
    int startIdx = i * binSize;
    int sum = 0;
    for(int j = startIdx; j < startIdx + binSize; j++){
      if(j >= length){
        break;
      }
      sum += output[j];
    }
    bins[i] = sum;
  }
  
  for(int i = 1; i < numBins; i++){
    bins[i] += bins[i-1];
  }

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numBins; i++){
    int currSum = 0;
    if(i > 0){
      currSum = bins[i-1];
    }
    int startIdx = i * binSize;
    for(int j = startIdx; j < startIdx + binSize; j++){
      if(j >= length){
        break;
      }
      currSum += output[j];
      output[j] = currSum;
    }
  }

  free(bins);

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

