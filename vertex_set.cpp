#include "vertex_set.h"

#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <stdio.h>
#include "mic.h"

#include <iostream>

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
  vertexSet->numNodes = numNodes;
  vertexSet->capacity = capacity;

  if(type == SPARSE){
    vertexSet->vertices = (Vertex*)malloc(sizeof(Vertex) * capacity);
    #pragma omp parallel for 
    for (int i = 0; i < capacity; i++) {
      vertexSet->vertices[i] = -1;
    }
    vertexSet->verticesDense = NULL;
  }
  else{
    vertexSet->verticesDense = (bool*)malloc(sizeof(bool) * numNodes);
    #pragma omp parallel for
    for(int i = 0; i < numNodes; i++){
      vertexSet->verticesDense[i] = false;
    }
    vertexSet->vertices = NULL;
  }
  
  return vertexSet;
}

void freeVertexSet(VertexSet *set)
{
  // TODO: Implement
  if(set->type == SPARSE){
    free(set->vertices);
  }
  else{
    free(set->verticesDense);
  }
  free(set);
}

void addVertex(VertexSet *set, Vertex v)
{
  // TODO: Implement
  
  int size = set->size;
  int capacity = set->capacity;
  if(size >= capacity) {
    return;
  }

  if(set->type == SPARSE){
    set->vertices[size] = v;
  }
  else{
    set->verticesDense[v] = true;
  }

  set->size = size + 1;
}

void removeVertex(VertexSet *set, Vertex v)
{
  // TODO: Implement
  int size = set->size;
  if(set->type == SPARSE){
    int removeIdx;
    bool found = false;

    for(int i = 0; i < size; i++){
      if(set->vertices[i] == v){
        removeIdx = i;
        found = true;
        break;
      }
    } 
    //v not in the set - nothing to remove
    if(!found){
      return;
    }
  
    //move last vertex to empty spot so don't need to shift
    set->vertices[removeIdx] = set->vertices[size];
    //set the last element back to -1 to indicate removal
    set->vertices[size-1] = -1; 
  }
  else{
    if(set->vertices[v]){
      return;
    }
    set->verticesDense[v] = false;
  }
  set->size = size - 1;
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

