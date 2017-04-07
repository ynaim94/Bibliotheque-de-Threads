#include "thread.h"
#include "queue.h"
#include <ucontext.h>

struct thread{
  thread_t id;
  ucontext_t context;
};

SIMPLEQ_HEAD(queue, thread);
struct queue head;
SIMPLEQ_INIT(head);


thread_t thread_self(void){
  
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
  
}

int thread_yield(void){}

int thread_join(thread_t thread, void **retval){}

void thread_exit(void *retval) __attribute__ ((__noreturn__)){}
