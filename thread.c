#include "thread.h"
#include "queue.h"
#include <ucontext.h>

typedef void* ret;
SIMPLEQ_HEAD(queue, thread);
struct queue head;
struct thread {
  SIMPLEQ_ENTRY(thread_t) id;
  SIMPLEQ_ENTRY(ucontext_t) context;
  SIMPLEQ_ENTRY(ret) retval;
} *t1, *t2;


thread_t thread_self(void){
  
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
  
}

int thread_yield(void){}

/* Permet de placer la valeur de retour du thread spécifié à l'adresse retval */

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

  /* trouver un moyen d'attendre que thread ait fini */
  
  if(retval == NULL){// si retval est NULL, ne rien faire, sinon ..
    return 1;
  }
  else {
    *retval=tmp.retval; //..on place la valeur de retour du thread dans retval
  }
}

/* simplement placer retval dans le retval du thread self */

void thread_exit(void *retval){}
