#include "scan.h"

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
