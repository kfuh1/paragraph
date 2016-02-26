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
  /*VertexSet* frontier = newVertexSet(SPARSE, num_nodes(g), num_nodes(g));
  addVertex(frontier, maxId);
  int iter = 0;

  for(int i = 0; i < num_nodes(g); i++){
    decomp[i] = -1;
  }

  decomp[maxId] = maxId;

  int* parents = (int*) malloc(sizeof(int*) * num_nodes(g));
  int count = 0;
  while(frontier->size > 0){
    for(int i = 0; i < num_nodes(g); i++){
      parents[i] = -1;
    }
    //printf("%d\n", frontier->size);
    for(int i = 0; i < frontier->size; i++){
      Vertex src = frontier->verticesSparse[i];
      const Vertex* start = outgoing_begin(g,src);
      const Vertex* end = outgoing_end(g,src);
      for(const Vertex* v = start; v < end; v++){
        if(decomp[*v] == -1){
          if(parents[*v] == -1){
            parents[*v] = src;
          }
          else if(parents[*v] > src){
            parents[*v] = src;
            
          }
        }
      }
      removeVertex(frontier, src);
    }
    for(int i = 0; i < num_nodes(g); i++){
      if(parents[i] != -1){
        decomp[i] = parents[i];
      }
    }

    iter++;

    for(int i = 0; i < num_nodes(g); i++){
      if(decomp[i] == -1){
        if(iter > maxVal - dus[i]){
          addVertex(frontier, i);
          decomp[i] = i;
        }
      }
    }
    
  }
  free(parents);*/
}
