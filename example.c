#include "thread.h"
#include "queue.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
static void * threadfunc(void * arg)
{
  char *name = arg;
  printf("je suis le thread %d, lancé avec l'argument %s\n",
	 (thread_t) thread_self(), name);
  thread_yield();
  printf("je suis encore le thread %d, lancé avec l'argument %s\n",
	 (thread_t) thread_self(), name);
    thread_exit(arg);
  return NULL;
}

int main(int argc, char *argv[])
  {
  thread_t thread1=1, thread2=2;
  void *retval1, *retval2;
  int err;

/*Queue init*/
head=malloc (sizeof (struct queue));
SIMPLEQ_INIT(head);
current=thread_create((thread_t)1,(void*)main,0); /*Create current thread*/
/************/

  printf("le main lance 2 threads...\n");
  err = thread_create(thread1, threadfunc, "thread1");
  assert(!err);
  err = thread_create(thread2, threadfunc, "thread2");
  assert(!err);
  printf("le main a lancé les threads %d et %d\n",
	 (thread_t) thread1, (thread_t) thread2);

  printf("le main attend les threads\n");
   err = thread_join(thread2, &retval2);
  assert(!err);
  err = thread_join(thread1, &retval1);
  assert(!err);
  printf("les threads ont terminé en renvoyant '%s' and '%s'\n",
	 (char *) retval1, (char *) retval2);
/*Free Queue*/
free(head);
free(current_thread->context.uc_stack.ss_sp);/*free the stack of current_thread*/
free(current_thread);
/************/
  return 0;
}
