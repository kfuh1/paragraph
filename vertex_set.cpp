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
  //printf("newVertexSet %d\n", numNodes);
  VertexSet* vertexSet = (VertexSet*) malloc(sizeof(VertexSet));
  vertexSet->type = type;
  vertexSet->size = 0;
  vertexSet->capacity = capacity;
  vertexSet->numNodes = numNodes;

  //depending on representation, allocate correct space
  if(type == DENSE){
    vertexSet->verticesDense = (bool*)malloc(sizeof(bool) * numNodes);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      vertexSet->verticesDense[i] = false;
    }
    vertexSet->verticesSparse = NULL;
  }
  else{
    vertexSet->verticesDense = NULL;
    vertexSet->verticesSparse = (int*) malloc(sizeof(int) * capacity);
  }
  /*#pragma omp parallel for schedule(static)
  for(int i = 0; i < capacity; i++){
    vertexSet->verticesSparse[i] = -1;
  }*/
  
  //printf("newVertexSetEnd %d\n", numNodes);
  return vertexSet;
}

void freeVertexSet(VertexSet *set)
{
  //printf("freeVertexSet %d\n", set->numNodes);
  if(set == NULL){
    return;
  }
  if(set->verticesDense != NULL){
    free(set->verticesDense);
  }
  if(set->verticesSparse != NULL){
    free(set->verticesSparse);
  }
  free(set);
}

void addVertex(VertexSet *set, Vertex v)
{
  //printf("addVertex %d\n", set->numNodes);
  int size = set->size;
  int capacity = set->capacity;
  if(size >= capacity){
    return;
  }

  if(set->type == DENSE){
    set->verticesDense[v] = true;
    #pragma omp atomic
    set->size += 1;
  }
  //in the case of sparse we're most likely going to be doing the
  //adding manually in edgemap so don't need locks here
  else{
    set->verticesSparse[size] = v;
    set->size += 1;
  }
}

void removeVertex(VertexSet *set, Vertex v)
{
  //printf("%d\n", set->numNodes);
  if(set->type == DENSE){
    if(set->verticesDense[v]){
      set->verticesDense[v] = false;
      #pragma omp atomic
      set->size -= 1;
    }
  }
  //never calling add vertex so no locks here
  else{
    for(int i = 0; i < set->size; i++){
      if(set->verticesSparse[i] == v){
        //setting to -1 first will handle problem when vertex is last elem
        set->verticesSparse[i] = -1;
        //move the last element into the space where vertex was removed
        set->verticesSparse[i] = set->verticesSparse[set->size - 1]; 
        set->size -= 1;
        break;
      }
    }
  }
}

void convertToDense(VertexSet *set){
  //printf("convertDense %d\n", set->numNodes);
  if(set->type == DENSE){
    return;
  }
  set->type = DENSE;
  set->verticesDense = (bool*) malloc(sizeof(bool) * set->numNodes);
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
  //printf("convertSparse\n");
  if(set->type == SPARSE){
    return;
  }
  set->type = SPARSE;
  int numNodes = set->numNodes;
  //printf("%d\n", numNodes);
  int* scanResults = (int*) malloc(sizeof(int) * numNodes);
  set->verticesSparse = (int*) malloc(sizeof(int) * set->size);
  /*#pragma omp parallel for schedule(static)
  for(int i = 0; i < set->size; i++){
    set->verticesSparse[i] = -1;
  }*/

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    scanResults[i] = set->verticesDense[i];
  }
  scan(numNodes, scanResults);

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    if(set->verticesDense[i]){
      int idx = scanResults[i] - 1;
      set->verticesSparse[idx] = i;
    }
  }
  free(scanResults);
}

//our histoscan
void scan(int length, int* output){
  //printf("scan start\n");
  int numBins = omp_get_max_threads();
  int* bins = (int*) malloc(sizeof(int) * numBins);

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numBins; i++){
    bins[i] = 0;
  }

  int binSize = (length + numBins - 1)/numBins;

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numBins; i++){
    int startIdx = i * binSize;
    int sum = 0;
    int endIdx = startIdx + binSize;
    for(int j = startIdx; j < endIdx; j++){
      if(j >= length){
        break;
      }
      sum += output[j];
    }
    bins[i] = sum;
  }

  //sequentially get offsets for counts
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
    int end = startIdx + binSize;
    for(int j = startIdx; j < end; j++){
      if(j >= length){
        break;
      }
      currSum += output[j];
      output[j] = currSum;
    }
  }
  free(bins);
  //printf("scan end\n");
}

void unionHelper(VertexSet *set, bool* markers){
  if(set->type == DENSE){
    for(int i = 0; i < set->numNodes; i++){
      //if vertex is already marked as true, don't update
      if(set->verticesDense[i]){
        markers[i] = true;
      }
    }
  }
  else{
    for(int i = 0; i < set->size; i++){
      markers[set->verticesSparse[i]] = true;
    }
  }
}

/**
 * Returns the union of sets u and v. Destroys u and v.
 * Always returns a dense set
 */
VertexSet* vertexUnion(VertexSet *u, VertexSet* v)
{
  bool* markers = (bool*) malloc(sizeof(bool) * u->numNodes);

  #pragma omp parallel for schedule(static)
  for(int i = 0; i < u->numNodes; i++){
    markers[i] = false;
  }

  //make u and v fill out the markers array to get elems in union
  unionHelper(u, markers);
  unionHelper(v, markers);

  //get size of union
  int total = 0;
  #pragma omp parallel for reduction(+:total) schedule(static)
  for(int i = 0; i < u->numNodes; i++){
    int count = 0;
    if(markers[i]){
      count = 1;
    }
    total += count;
  }
  VertexSet* set = newVertexSet(DENSE, total, u->numNodes);

  //fill out new set
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < u->numNodes; i++){
    set->verticesDense[i] = markers[i];
  }
  set->size = total;
  //printf("size %d, %d\n", u->size, v->size);
  //printf("total %d\n", total);
  free(markers);
  freeVertexSet(u);
  freeVertexSet(v);
  return set;
}

