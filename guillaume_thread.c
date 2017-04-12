#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>

struct thread{
  thread_t id;
  ucontext_t context;
  void * retval;
  SIMPLEQ_ENTRY(thread) next;
};

thread_t thread_self(void){
  
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
  
}

int thread_yield(void){

}

int thread_join(thread_t thread, void **retval){

}

void thread_exit(void *retval){
  thread_t current_id = thread_self();
  struct thread * loop;
  thread_t tmp=NULL;
  struct thread current;
  SIMPLEQ_FOREACH(loop, head, next){
    if (current_id == loop->id){
      loop->retval = retval;
      SIMPLEQ_REMOVE(head, loop, thread, next);
    }
  }
}


SIMPLEQ_HEAD(queue, thread);
struct queue* head;

int main(int argc, char * argv[]){
  SIMPLEQ_INIT(head);
  return EXIT_SUCCESS;
}




typedef struct thread_mutex { int lock; int locker} thread_mutex_t; // ajout locker
#define LOCKED 1
#define UNLOCKED 0

int thread_mutex_init(thread_mutex_t *mutex){
  mutex->lock = UNLOCKED;
  mutex->locker = 0;
  return EXIT_SUCCESS;
}

int thread_mutex_destroy(thread_mutex_t *mutex){
  free(mutex);
  return EXIT_SUCCESS;
}

int thread_mutex_lock(thread_mutex_t *mutex){
  thread_t id thread_self();
  if (mutex->lock == LOCKED) {
    return EXIT_FAILURE
  }
  mutex->lock = LOCKED;
  mutex->locker = id;
  return EXIT_SUCCESS;
}

int thread_mutex_unlock(thread_mutex_t *mutex){
  thread_t id thread_self();
  if (mutex->lock == UNLOCKED) {
    return EXIT_FAILURE;
  }
  if (mutex->locker != id){
    return EXIT_FAILURE;
  }
  mutex->lock = UNLOCKED;
  return EXIT_SUCCESS;
}
