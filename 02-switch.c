#include <stdio.h>
#include <assert.h>
#include "thread.h"
#include<stdlib.h>
/* test de switchs.
 *
 * les affichages doivent être dans le bon ordre (fifo)
 * le programme doit retourner correctement.
 * valgrind doit être content.
 *
 * support nécessaire:
 * - thread_create()
 * - thread_yield() depuis ou vers le main
 * - thread_exit()
 * - thread_join() avec récupération de la valeur de retour, ou sans
 */

static void * thfunc(void *id)
{
  int err, i;
  for(i=0; i<10; i++) {
    printf("%s yield vers un autre thread %d ;i= %d \n", (char*) id, thread_self(),i);
    err = thread_yield();
    assert(!err);
  }

  printf("%s terminé\n", (char*) id);
  thread_exit(NULL);
}

int main()
{
  thread_t th1=2,th2=3,th3=4;
  void *res;
  int err, i;

/*Queue init*/
head=malloc (sizeof (struct queue));
SIMPLEQ_INIT(head);
current=thread_create(1,(void*)main,0); /*initialize current_thread*/
/************/

  err = thread_create(th1, thfunc, "fils1");
  assert(!err);
  err = thread_create(th2, thfunc, "fils2");
  assert(!err);
  err = thread_create(th3, thfunc, "fils3");
  assert(!err);
  /* des switchs avec l'autre thread */
  for(i=0; i<20; i++) {
    printf("le main yield vers un fils\n");
    err = thread_yield();
    assert(!err);
  }

  err = thread_join(th3, &res);
  assert(!err);
  assert(res == NULL);
  err = thread_join(th2, NULL); /* on ignore ce code de retour */
  assert(!err);
  err = thread_join(th1, &res);
  assert(!err);
  assert(res == NULL);

  printf("main terminé\n");
/*Free Queue*/
free(head);
free(current_thread->context.uc_stack.ss_sp);/*free the stack of current_thread*/
free(current_thread);
/************/
  return 0;
}
