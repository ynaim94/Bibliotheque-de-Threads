#include "thread.h"
#include "queue.h"
#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define ERROR -1
static int firstThread=1;
thread_t thread_self(void){
  return current_thread->id;
}


int thread_create(thread_t newthread, void *(*func)(void *), void *funcarg) {
 // ucontext_t ctx;
/* if (newthread == NULL){
   fprintf(stderr, "newthread not initialized\n");
    return ERROR;
  }
  */
  if ((t1 = malloc (sizeof(struct thread))) == NULL){
    fprintf(stderr, "Error Malloc\n");
    return ERROR;
  }
 getcontext(&(t1->context));
      (t1->context).uc_stack.ss_size = 64*1024;
  if (((t1->context).uc_stack.ss_sp = malloc((t1->context).uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error Malloc\n");
    return ERROR;
  }

 (t1->context).uc_link =  NULL;
  makecontext(&(t1->context), (void (*)(void)) func ,1, funcarg);
  
  t1->id=newthread;
 // t1->context=ctx;
  t1->retval=0;

if (firstThread){
current_thread=t1;
main_thread=t1;
//getcontext(&current_thread->context); 
firstThread=0;
}
else 
  SIMPLEQ_INSERT_TAIL(head, t1, next);
return 0;
}

int thread_yield(void){
 if ( SIMPLEQ_EMPTY(head) ){
return 0;
}
else{
if(current_thread!=main_thread) {
SIMPLEQ_INSERT_TAIL(head, current_thread, next);// ajouter la thread courante à la fin de la Queue 
current_thread->context.uc_link=&main_thread->context; //Rend la main à la thread main à la fin
}
ucontext_t ctx=current_thread->context;
SIMPLEQ_FIRST(head)->context.uc_link=&current_thread->context;//Rend la main à la thread courante à la fin
current_thread=SIMPLEQ_FIRST(head); // La thread à la tête de la Queue devient la thread courante
SIMPLEQ_REMOVE_HEAD(head, next); //On l'enlève de la Queue
swapcontext(&ctx, &current_thread->context); //On la lance

<<<<<<< HEAD

int thread_join(thread_t thread, void **retval){
=======
 return EXIT_SUCCESS;
}
}

 int thread_join(thread_t thread, void **retval){
>>>>>>> c2dcc40a9b0c2c5e210393abec84d6f244dcf120
  struct thread* loop;
  thread_t tmp=NULL;
  void* tmp_ret;
  bool is_present=false;
  int compteur = 0;
  struct timespec time, time2;
  time.tv_sec=0;
  time.tv_nsec=1000;
  /* on parcourt la file à la recherche du thread. 
S'il est présent, on sauvegarde les données qui nous intéresse
Sinon, on sort.
Tant que le thread est présent dans la file, on recommence la boucle jusqu'à ce qu'il n'y soit plus. A ce moment, on sort du do while. */
  do {
    SIMPLEQ_FOREACH(loop, head, next){
      if (loop->id==thread){ //on cherche si thread est bien dans la liste
	printf("il existe !%d\n", thread);
	tmp=thread;
	tmp_ret=loop->retval;
	is_present=true;
      }
    }
<<<<<<< HEAD
    if(tmp==NULL && compteur==0){
      printf("%p n'appartient pas à la liste\n", thread);
      return -1;
    }
    if(tmp==NULL && compteur>=0){
      printf("%p n'appartient plus à la liste\n", thread);
    if(tmp== -1 && compteur==0){
      printf("%d n'appartient pas à la liste\n", thread);
      return -1;
    }
    if(tmp== -1 && compteur>=0){
      printf("%d n'appartient plus à la liste\n", thread);
      is_present=false;
    }
     
    tmp=NULL;    
    if (nanosleep(&time,&time2) < 0){
      perror("nanosleep\n");
    }
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
  current = thread_self();
  struct thread * loop;
  thread_t tmp=(thread_t)NULL;
  //struct thread current;
  SIMPLEQ_FOREACH(loop, head, next){
    if (current == loop->id){
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
  struct timespec time, time2;
  time.tv_sec = 0;
  time.tv_nsec = 100;
  if (mutex->locker == id)
    return EXIT_FAILURE;
  do{  
    while (mutex->locker != -1){
      if(nanosleep(&time, &time2) < 0)
	perror("nanosleep\n");
    }
    mutex->locker = id;
  } while (mutex->locker != id);
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
