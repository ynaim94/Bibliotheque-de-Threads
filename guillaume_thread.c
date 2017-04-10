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
  return 0;
}
