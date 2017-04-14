#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define ERROR -1

thread_t thread_self(void){
  return current_thread->id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
  ucontext_t ctx;
  struct thread *th;
 

 if (newthread == NULL){
    fprintf(stderr, "newthread not initialized\n");
    return -1;
  }
  
  if ((th = malloc (sizeof(struct thread))) == NULL){
    fprintf(stderr, "Error Malloc\n");
    return -1;
  }
      ctx.uc_stack.ss_size = 64*1024;
  if ((ctx.uc_stack.ss_sp = malloc(ctx.uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error Malloc\n");
    return -1;
  }

 ctx.uc_link = NULL;
  makecontext(&ctx, (void (*)(void)) func ,1, funcarg);
  
  th->id=*newthread;
  th->context=ctx;
  th->retval=0;
  SIMPLEQ_INSERT_TAIL(head, th, next);
  return 0;
}

int thread_yield(void){
  if ( SIMPLEQ_EMPTY(head) ) return ERROR;
  SIMPLEQ_INSERT_TAIL(head, current_thread, next);
  current_thread=SIMPLEQ_FIRST(head);
  SIMPLEQ_REMOVE_HEAD(head, next);
  return EXIT_FAILURE;
}

int thread_join(thread_t thread, void **retval){
  struct thread* loop;
  thread_t tmp= -1;
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
    if(tmp== -1 && compteur==0){
      printf("%p n'appartient pas à la liste\n", thread);
      return -1;
    }
    if(tmp== -1 && compteur>=0){
      printf("%p n'appartient plus à la liste\n", thread);
      is_present=false;
    }
     
    tmp= -1;
    //scheld_yield();
    compteur++;
  } while (is_present);
  
  if(retval == NULL){// si retval est NULL, ne rien faire, sinon ..
    return 1;
  }
  else {
    *retval=tmp_ret; //..on place la valeur de retour du thread dans retval
  }
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


////////////////////////////////////////////////////////////////////
///////////////    Mutex    ////////////////////////////////////////
////////////////////////////////////////////////////////////////////


int thread_mutex_init(thread_mutex_t *mutex){
  mutex->locker = -1;
  return EXIT_SUCCESS;
}

int thread_mutex_destroy(thread_mutex_t *mutex){
  free(mutex);
  return EXIT_SUCCESS;
}

int thread_mutex_lock(thread_mutex_t *mutex){
  thread_t id = thread_self();
  if (mutex->locker != -1) {
    return EXIT_FAILURE;
  }
  mutex->locker = id;
  return EXIT_SUCCESS;
}

int thread_mutex_unlock(thread_mutex_t *mutex){
  thread_t id = thread_self();
  if (mutex->locker == -1) {
    return EXIT_FAILURE;
  }
  if (mutex->locker != id){
    return EXIT_FAILURE;
  }
  mutex->locker = -1;
  return EXIT_SUCCESS;
}

