#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdbool.h>

SIMPLEQ_HEAD(queue, thread);
struct queue *head;
struct thread {
  thread_t id;
  ucontext_t context;
  void* retval;
  SIMPLEQ_ENTRY(thread) next;
} *t1, *t2;


struct thread current_thread ;

thread_t thread_self(void){
  return current_thread.id;
}

/* int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){ */
/*   ucontext_t ctx; */
/*   struct thread th;	 */
/*   getcontext(&ctx); */
/*   ctx.uc_stack.ss_size = 64*1024; */
/*   ctx.uc_stack.ss_sp = malloc(ctx.uc_stack.ss_size); */
/*   ctx.uc_link = NULL; */
/*   makecontext(&ctx, (void (*)(void)) func ,1,  *funcarg); */
  
/*   th.id=*newthread; */
/*   th.context=ctx; */
/*   th.retval=0; */
/*   SIMPLEQ_INSERT_TAIL(head, th, field???); */
/* } */

/* int thread_yield(void){ */
  
/*   if ( SIMPLEQ_EMPTY(head) == NULL ) return; */
/*   SIMPLEQ_INSERT_TAIL(head, current_thread, field???); */
/*   /\* Free the current_thread's stack *\/ */
/*   free( current_thread.context.uc_stack.ss_sp); */
/*   current_thread=SIMPLEQ_FIRST(head); */
/*   SIMPLEQ_REMOVE_HEAD(head, field??); */
    
/* } */

/* Permet de placer la valeur de retour du thread spécifié à l'adresse retval */

int thread_join(thread_t thread, void **retval){
  struct thread* loop;
  thread_t tmp=NULL;
  void* tmp_ret;
  bool is_present=false;
  int compteur = 0;
  /* on parcourt la file à la recherche du thread. 
S'il est présent, on sauvegarde les données qui nous intéresse
Sinon, on sort.
Tant que le thread est présent dans la file, on recommence la boucle jusqu'à ce qu'il n'y soit plus. A ce moment, on sort du do while. */
  do {
    SIMPLEQ_FOREACH(loop, head, next){
      if (loop->id==thread){ //on cherche si thread est bien dans la liste
	printf("il existe !%p\n", thread);
	tmp=thread;
	tmp_ret=loop->retval;
	is_present=true;
      }
    }
    if(tmp==NULL && compteur==0){
      printf("%p n'appartient pas à la liste\n", thread);
      return -1;
    }
    if(tmp==NULL && compteur>=0){
      printf("%p n'appartient plus à la liste\n", thread);
      is_present=false;
    }
     
    tmp=NULL;
    sched_yield();
    compteur++;
  } while (is_present);
  
  if(retval == NULL){// si retval est NULL, ne rien faire, sinon ..
    return 1;
  }
  else {
    *retval=tmp_ret; //..on place la valeur de retour du thread dans retval
  }
}

/* simplement placer retval dans le retval du thread self */

void thread_exit(void *retval){}
