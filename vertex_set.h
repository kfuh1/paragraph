#ifndef __VERTEX_SET__
#define __VERTEX_SET__

#include "graph.h"

typedef enum {
  DENSE,
  SPARSE,
} VertexSetType;

typedef struct {
  int size;     // Number of nodes in the set
  int numNodes; // Number of nodes in the graph
  int capacity;
  VertexSetType type; 
  Vertex* verticesSparse;
  bool* verticesDense;
} VertexSet;

VertexSet *newVertexSet(VertexSetType type, int capacity, int numNodes);
void freeVertexSet(VertexSet *set);

void addVertex(VertexSet *set, Vertex v);
void removeVertex(VertexSet *set, Vertex v);

void convertToDense(VertexSet *set);
void convertToSparse(VertexSet *set);

VertexSet*  vertexUnion(VertexSet *u, VertexSet* v);

#endif // __VERTEX_SET__
