#include "../src/thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>


int num_thrd;   // number of threads
double A[SIZE][SIZE], B[SIZE][SIZE], C[SIZE][SIZE];
 
// initialize a matrix
void init_matrix(double m[SIZE][SIZE])
{
  int i, j = 0;
  double val = 0;
  for (i = 0; i < SIZE; i++)
    for (j = 0; j < SIZE; j++)
      m[i][j] = val++;
}
 
void print_matrix(int m[SIZE][SIZE])
{
  int i, j;
  for (i = 0; i < SIZE; i++) {
    printf("\n\t| ");
    for (j = 0; j < SIZE; j++)
      printf("%2d ", m[i][j]);
    printf("|");
  }
}
 
void* multiply(void* slice)
{
  int s = (int)slice;  
  int from = (s * SIZE)/num_thrd; 
  int to = ((s+1) * SIZE)/num_thrd;
  int i,j,k;
 
  printf("computing slice %d (from row %d to %d)\n", s, from, to-1);
  for (i = from; i < to; i++)
  {  
    for (j = 0; j < SIZE; j++)
    {
      C[i][j] = 0;
      for ( k = 0; k < SIZE; k++){
	C[i][j] += A[i][k]*B[k][j];
      }
    }
  }
  printf("finished slice %d\n", s);
  return 0;
}
 
int main(int argc, char* argv[])
{

  struct timeval tv1;
  struct timeval tv2;
  gettimeofday(&tv1,NULL);
  if (argc!=2)
  {
    printf("Usage: %s number_of_threads\n",argv[0]);
    return -1;
  }
  num_thrd = atoi(argv[1]);


  thread_t thread[num_thrd];
  int i;
  int err;
  init_matrix(A);
  init_matrix(B);
 
  for (i = 0; i < num_thrd; i++)
    {
      err = thread_create(&thread[i], &multiply, (void*)i);
      assert(!err);
    }
 
  for (i = 0; i < num_thrd; i++){
    err=thread_join(thread[i], NULL);
    assert(!err);
  }
  gettimeofday(&tv2,NULL);
  /* printf("\n\n"); */
  /* print_matrix(A); */
  /* printf("\n\n\t       * \n"); */
  /* print_matrix(B); */
  /* printf("\n\n\t       = \n"); */
  /* print_matrix(C); */
  /* printf("\n\n"); */
  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time : %f\n",time);
  return 0;
 
}
