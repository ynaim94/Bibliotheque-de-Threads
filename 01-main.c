#include <stdio.h>
#include <assert.h>
#include "thread.h"
#include<stdlib.h>
/* test du thread_self et yield du main seul.
 *
 * le programme doit retourner correctement.
 * valgrind doit être content.
 *
 * support nécessaire:
 * - thread_yield() depuis et vers le main
 * - thread_self() depuis le main
 */

int main()
{
  int err, i;
/*Queue init*/
head=malloc (sizeof (struct queue));
SIMPLEQ_INIT(head);
current=thread_create(1,(void*)main,0); /*initialize current_thread*/
/************/

  for(i=0; i<10; i++) {
    printf("le main yield tout seul\n");
    err = thread_yield();
    assert(!err);
  }

  printf("le main est %p\n", (void*) thread_self());

/*Free Queue*/
free(head);
free(current_thread->context.uc_stack.ss_sp);/*free the stack of current_thread*/
free(current_thread);
/************/
  return 0;
}
