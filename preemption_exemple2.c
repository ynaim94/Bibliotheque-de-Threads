#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "thread.h"

#define MAX 100 000

static void * thfunc(void* tv)
{
  struct timeval tv2;
  struct timeval* tv1 = (struct timeval*) tv;
  int i = 0;
  for (i = 0; i < 2 ;  i++){
    //printf("thread 1\n");
    usleep(100);
    thread_self();
  }
  thread_yield();
  gettimeofday(&tv2,NULL);
  
  double time = (tv2.tv_sec - tv1->tv_sec) + (tv2.tv_usec - tv1->tv_usec)/1000000.0;
  printf("time th1  %f\n",time);
  
  thread_exit(NULL);
}

static void * thfunc2(void* tv)
{
  struct timeval tv2;
  struct timeval* tv1 = (struct timeval*) tv;
  int i = 0;

  for (i = 0; i < 2 ;  i++){
    //printf("thread 2\n");
    usleep(100);
    thread_self();
  }

  gettimeofday(&tv2,NULL);
  
  double time = (tv2.tv_sec - tv1->tv_sec) + (tv2.tv_usec - tv1->tv_usec)/1000000.0;
  printf("time th2: %f\n",time);
  
  thread_exit(NULL);
}

int main(int argc, char *argv[]){
  
  struct timeval tv1;
  struct timeval tv2;
  struct timeval tv3;
  
  thread_t th1,th2;
  void *res;
  int err;
  
  gettimeofday(&tv1,NULL);
  err = thread_create(&th2, thfunc2, &tv1);
  assert(!err);
  gettimeofday(&tv2,NULL);
  err = thread_create(&th1, thfunc, &tv2);
  assert(!err);
  thread_join(th2, &res);
  
  gettimeofday(&tv3,NULL);
  
  double time = (tv3.tv_sec - tv1.tv_sec) + (tv3.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time total : %f\n",time);
  printf("main terminé\n");
  return 0;
}
