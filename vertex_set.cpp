#include "vertex_set.h"

#include <stdlib.h>
#include <string.h>
#include <cassert>
#include <stdio.h>
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
  vertexSet->numNodes = numNodes;
  vertexSet->vertices = (Vertex*)malloc(sizeof(Vertex) * capacity);
  // initialize to -1's?
  // HALP
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
  int index = set->size;
  set->vertices[index + 1] = v;
}

void removeVertex(VertexSet *set, Vertex v)
{
  // TODO: Implement
  int index = set->size;
  // what do
  for (int i = 0; i < index; i++) {
    if (set->vertices[i] == v) {
      set->vertices[i] = -1;
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

