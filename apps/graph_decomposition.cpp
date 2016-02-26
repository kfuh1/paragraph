#include <stdlib.h>
#include "paraGraph.h"
#include "vertex_set.h"
#include "graph.h"

#define NA -1

class CircleUpdate
{
  public:
    int* decomp;
    bool* updated;

    CircleUpdate(int* decomp, bool* updated, int numNodes) : decomp(decomp), updated(updated)
    {
      #pragma omp parallel for schedule(static)
      for(int i = 0; i < numNodes; i++){
        updated[i] = false;
      }
    };
    bool update(Vertex src, Vertex dst){
      bool result = false;
      #pragma omp critical
      if(decomp[dst] == NA){
        decomp[dst] = decomp[src];
        updated[dst] = true;
        result = true;
      }
      else{
        if(updated[dst] && decomp[src] < decomp[dst]){
          decomp[dst] = decomp[src];
          result = true;
        }
      }
      return result;
    }
    bool cond(Vertex v){
      return true;
    }

};

class Visited
{
  public:
    int* decomp;
    int* dus;
    int maxVal;
    int iter;
    Visited(int* decomp, int* dus, int maxVal, int iter) :
        decomp(decomp), dus(dus), maxVal(maxVal), iter(iter) {};

    bool operator()(Vertex v){
      bool result = false;
      if(decomp[v] == NA && iter > maxVal - dus[v]){
        decomp[v] = v;
        result = true;
      }
      return result;
    }
};

/**
	Given a graph, a deltamu per node, the max deltamu value, and the id
	of the node with the max deltamu, decompose the graph into clusters. 
        Returns for each vertex the cluster id that it belongs to inside decomp.
	NOTE: deltamus are given as integers, floating point differences
	are resolved by node id order

**/
void decompose(graph *g, int *decomp, int* dus, int maxVal, int maxId) {
  int numNodes = num_nodes(g);
  VertexSet* frontier = newVertexSet(SPARSE, 1, numNodes);
  
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    decomp[i] = NA;
  }
  decomp[maxId] = maxId;

  addVertex(frontier, maxId);

  int iter = 0;

  //need this to pass into vertexMap when we go through and look
  //for vertices that need to be visited - need to have all vertices
  VertexSet* graphSet = newVertexSet(DENSE, numNodes, numNodes);
  for(int i = 0; i < numNodes; i++){
    graphSet->verticesDense[i] = true;
  }
  graphSet->size = numNodes;


  VertexSet* newFrontier = NULL;
  bool* updated = (bool*) malloc(sizeof(bool) * numNodes);
  while(frontier->size > 0){
    CircleUpdate cu (decomp, updated, numNodes);
    VertexSet* partialSet1 = edgeMap(g, frontier, cu);
    iter++;
    
    Visited v(decomp, dus, maxVal, iter);

    VertexSet* partialSet2 = vertexMap(graphSet, v, true);

    newFrontier = vertexUnion(partialSet1, partialSet2);

    freeVertexSet(frontier); 
    frontier = newFrontier;
  }
  free(updated);
  if(newFrontier != NULL){
    freeVertexSet(newFrontier);
  }
}
