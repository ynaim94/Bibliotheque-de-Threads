#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "thread.h"

#define MAX 100 000

static void * thfunc(void* id)
{
  struct timeval tv1;
  struct timeval tv2;
  gettimeofday(&tv1,NULL);
  int i = 0;
  for (i = 0; i < 2 ;  i++){
    printf("thread : %s\n", (char*) id);
    usleep(100);
    thread_self();
  }

  gettimeofday(&tv2,NULL);
  
  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time %s: %f\n",(char *)id,time);
  
  thread_exit(NULL);
}

static void * thfunc2(void *id)
{
  struct timeval tv1;
  struct timeval tv2;
  gettimeofday(&tv1,NULL);
  thread_yield();
  int i = 0;
  for (i = 0; i < 5 ;  i++){
    printf("thread : %s\n", (char*) id);
    usleep(400);
    thread_self();
  }

  gettimeofday(&tv2,NULL);
  
  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time th2: %f\n",time);
  
  thread_exit(NULL);
}

int main(int argc, char *argv[]){
  
  struct timeval tv1;
  struct timeval tv2;

  gettimeofday(&tv1,NULL);
  thread_t th1,th2;
  void *res;
  int err;
  err = thread_create(&th1, thfunc, "th1");
  assert(!err);
  err = thread_create(&th2, thfunc2, "th2");
  assert(!err);
  thread_join(th1, &res);
  
  gettimeofday(&tv2,NULL);
  
  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time total : %f\n",time);
  printf("main terminé\n");
    
}
