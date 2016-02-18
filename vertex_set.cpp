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
  //

  std::cout << "begin new";

  VertexSet* vertexSet = (VertexSet*)malloc(sizeof(VertexSet));
  vertexSet->type = type;
  vertexSet->size = 0;
  vertexSet->numNodes = numNodes;
  vertexSet->capacity = capacity;
  vertexSet->vertices = (Vertex*)malloc(sizeof(Vertex) * capacity);
  
  // initializing
  for (int i = 0; i < capacity; i++) {
    vertexSet->vertices[i] = -1;
  }

  std::cout << "end new";
  
  return vertexSet;
}

void freeVertexSet(VertexSet *set)
{
  // TODO: Implement
  free(set->vertices);
  free(set);
}

void addVertex(VertexSet *set, Vertex v)
{
  // TODO: Implement
  
  std::cout << "begin add";
  
  int size = set->size;
  
  if (size >= set->capacity) {
    return;
  }

  int firstOpenIdx = 0;
  //indicates whether open index has been found
  bool found = false; 

  int currCount = 0;
  int idx = 0;
  while(currCount < size){
    //because we need to loop through whole array to check
    //for duplicates, we only mark the first open index
    if (!found && set->vertices[idx] == -1) {
      firstOpenIdx = idx;
      found = true;
    }
    //exit if there's a duplicate
    if (set->vertices[idx] == v) {
      return;
    }
    if(set->vertices[idx] != -1){
      currCount += 1;
    }
    idx += 1;
  }

  set->vertices[firstOpenIdx] = v;
  set->size += 1;

  std::cout << "end add";

}

void removeVertex(VertexSet *set, Vertex v)
{
  // TODO: Implement
  

  std::cout << "begin remove";
  
  for (int i = 0; i < set->capacity; i++) {
    //assumes that there are no duplicates in our set
    if (set->vertices[i] == v) {
      set->vertices[i] = -1;
      set->size -= 1;
      return;
    }  
  }

  std::cout << "end remove";
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

