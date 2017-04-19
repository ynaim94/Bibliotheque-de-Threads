#include "thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#define ERROR -1

static int firstThread=1;
thread_t thread_self(void){
  return current_thread->id;
}

int thread_create(thread_t newthread, void *(*func)(void *), void *funcarg) {
  ucontext_t ctx;
/* if (newthread == NULL){
   fprintf(stderr, "newthread not initialized\n");
    return ERROR;
  }
  */
  if ((t1 = malloc (sizeof(struct thread))) == NULL){
    fprintf(stderr, "Error Malloc\n");
    return ERROR;
  }
      ctx.uc_stack.ss_size = 64*1024;
  if ((ctx.uc_stack.ss_sp = malloc(ctx.uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error Malloc\n");
    return ERROR;
  }

 ctx.uc_link = NULL;
  makecontext(&ctx, (void (*)(void)) func ,1, funcarg);
  
  t1->id=newthread;
  t1->context=ctx;
  t1->retval=0;

if (firstThread){
current_thread=t1; 
firstThread=0;
}
else 
  SIMPLEQ_INSERT_TAIL(head, t1, next);
  return 0;
}

int thread_yield(void){
 // if ( SIMPLEQ_EMPTY(head) ) return 0;

  SIMPLEQ_INSERT_TAIL(head, current_thread, next);
  current_thread=SIMPLEQ_FIRST(head);
  SIMPLEQ_REMOVE_HEAD(head, next);
  return EXIT_SUCCESS;
}

/* Permet de placer la valeur de retour du thread spécifié à l'adresse retval */
/*
int thread_join(thread_t thread, void **retval){
  thread loop;
  thread_t tmp=NULL;
  thread current;
  SIMPLEQ_FOREACH(loop, head, id){
    if (loop.id==thread){ //on cherche si thread est bien dans la liste
      printf("il existe !%p\n", thread);
      tmp=thread;
    }
    if (loop.id==thread_self()){ //on veut récupérer les infos du thread courant
      current=loop;
    }
  }
  if(tmp==NULL){
    printf("%p n'appartient pas à la liste\n", thread);
    return -1;
  }

   trouver un moyen d'attendre que thread ait fini

  if(retval == NULL){// si retval est NULL, ne rien faire, sinon ..
    return 1;
  }
  else {
    *retval=tmp.retval; //..on place la valeur de retour du thread dans retval
  }
}
*/
/* simplement placer retval dans le retval du thread self */
/*
void thread_exit(void *retval){}
*/
