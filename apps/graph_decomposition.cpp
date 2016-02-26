#include "paraGraph.h"
#include "vertex_set.h"
#include "graph.h"

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
  
  addVertex(frontier, maxId);

  int iter = 0;
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    decomp[i] = -1;
  }

  //circle center of first thing is itself
  decomp[maxId] = maxId;

  //parents is used to keep track of who found the vertex in order
  //to figure out the smallest circle center in an interation
  Vertex* parents = (Vertex*) malloc(sizeof(Vertex*) * numNodes);
  #pragma omp parallel for schedule(static)
  for(int i = 0; i < numNodes; i++){
    parents[i] = -1;
  }

  parents[maxId] = maxId;

  //keeps track of which vertices are in current frontier for quick lookup
  bool* inFrontier = (bool*) malloc(sizeof(bool) * numNodes);

  VertexSet* newFrontier;
  while(frontier->size > 0){
    newFrontier = newVertexSet(SPARSE, numNodes, numNodes);
    #pragma omp parallel for schedule(static)
    for(int i = 0; i < numNodes; i++){
      inFrontier[i] = false;
    }
    //how large should this new frontier be?
    for(int i = 0; i < frontier->size; i++){
      Vertex src = frontier->verticesSparse[i];
      inFrontier[src] = true;
      const Vertex* start = outgoing_begin(g,src);
      const Vertex* end = outgoing_end(g,src);
      for(const Vertex* v = start; v < end; v++){
        //if vertex has not been claimed yet
        if(decomp[*v] == -1){
          parents[*v] = src;
          decomp[*v] = decomp[src];
          addVertex(newFrontier, *v);
        }
        else{
          //change the circle center to the smaller center
          if(decomp[src] < decomp[parents[*v]] && inFrontier[parents[*v]]){
            parents[*v] = src;
            decomp[*v] = decomp[src];
          }
        }
      }
    }

    iter++;

    for(int i = 0; i < numNodes; i++){
      if(decomp[i] == -1 && iter > maxVal - dus[i]){
          addVertex(newFrontier, i);
          decomp[i] = i;
      }
    }
    freeVertexSet(frontier); 
    frontier = newFrontier;
  }
  if(newFrontier != NULL){
    freeVertexSet(newFrontier);
  }
  free(parents);
}
