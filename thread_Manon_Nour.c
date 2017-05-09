#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "queue.h"
#include "thread.h"

#define ERROR -1
#define MAIN_ID 0


static SIMPLEQ_HEAD(queue, thread) runq = SIMPLEQ_HEAD_INITIALIZER(runq);
static struct queue overq =  SIMPLEQ_HEAD_INITIALIZER(overq);
static struct queue sleepq =  SIMPLEQ_HEAD_INITIALIZER(sleepq);

typedef void* ret;

struct thread {
  thread_t id;
  ucontext_t* context;
  ret retval;
  struct queue waiting_thread;
  SIMPLEQ_ENTRY(thread) next;
} *current_thread;

thread_t current;


static int firstThread = 1;

void func2(void *(*func)(void *), void *funcarg){
  struct thread* loop;
  void *retval;
  retval = func(funcarg);
  /*	SIMPLEQ_FOREACH(loop, &overq, next){
	if (loop->id== thread_self()){
	thread_exit(retval );
	break;
	}
	}*/

  thread_exit(retval );
}

int create_main_thread(){

  /* initialisation du thread courant */
  ucontext_t ctx;
  thread_create( MAIN_ID,main,NULL); /*error: To be resolvedd*/
		 /* firstThread = 0;
  current_thread = malloc (sizeof(struct thread));
  current_thread->id = MAIN_ID;
  SIMPLEQ_INIT(&current_thread->waiting_thread);
  getcontext(&ctx);
  current_thread->context=&ctx;
  current_thread->retval = NULL;
		 */ 
}

thread_t thread_self(void){

  if (firstThread){
    create_main_thread();
  }

  return current_thread->id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
  // ucontext_t ctx;
  void (*)(void)
  static int i = 1;
  struct thread* t1;

 

  if (newthread == NULL){
    fprintf(stderr, "newthread not initialized\n");
    return ERROR;
  }
   
  if ((t1 = malloc (sizeof(struct thread))) == NULL){
    //  fprintf(stderr, "Error malloc\n");
    return ERROR;
  }
  
  t1->context=malloc(sizeof(ucontext_t));
  getcontext((t1->context));
  
  (t1->context)->uc_stack.ss_size = 64*1024;
  if (((t1->context)->uc_stack.ss_sp = malloc((t1->context)->uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error malloc\n");
    return ERROR;
  }

  (t1->context)->uc_link =  NULL;
  makecontext((t1->context), (void (*)(void)) func2 ,2,func, funcarg);
  t1->id= i;
  *newthread = i;
  t1->retval=0;  
 if (firstThread){
   current_thread=t1;
firstThread = 0;
  }
else{
  SIMPLEQ_INIT(&t1->waiting_thread);
  SIMPLEQ_INSERT_TAIL(&runq, t1, next);
  i++;
  }
  return 0;
}



int thread_yield(void){
  struct thread* previous_thread;
  if (firstThread){
    create_main_thread();
  }
  
  if ( SIMPLEQ_EMPTY(&runq) ){
    printf("FIFO vide ! \n");
    return 0;
  }
  else{
    /* insertion du thread courant à la fin de la file */
    SIMPLEQ_INSERT_TAIL(&runq, current_thread, next);
    /* On sauvegarde le thread courant pour la suite */
    previous_thread=current_thread;
    /* Rend la main à la thread courante à la fin */
    SIMPLEQ_FIRST(&runq)->context->uc_link=current_thread->context;
    /* La tête devient le thread courant */
    current_thread=SIMPLEQ_FIRST(&runq);
    printf("current_thread id : %d",current_thread->id);
    /* On supprime la tête de la file */
    SIMPLEQ_REMOVE_HEAD(&runq, next);
    /* On effectue le swap */
    swapcontext(previous_thread->context, current_thread->context);
    /* On remet en place le thread courant */
    current_thread=previous_thread;
    return EXIT_SUCCESS;
  }
}



/* Permet de placer la valeur de retour du thread spécifié à l'adresse retval */
int thread_join(thread_t thread, void **retval){
  /*Queue head1 init*/

  /************/
  printf("%d attend %d avec join\n", thread_self(), thread);
  struct thread* loop;
  struct thread* loop_rest;
  struct thread *previous_thread;
  struct thread *tmp;

  /* On cherche le thread dans la file des threads non-utilisés */
  SIMPLEQ_FOREACH(loop_rest,&overq, next){
    if (loop_rest->id==thread){
      printf("il existe dans overq\n");
      if (retval == NULL){
	/* Ne rien faire */
	printf("la valeur de retour set ignorée\n");
      }
      else {
	printf("on place dans retval la valeur %p\n", loop_rest->retval);
	*retval=loop_rest->retval;
      }
      return EXIT_SUCCESS; }
  }

  
  /* On cherche dans un premier temps si le thread recherché est dans la file des threads actifs */
  SIMPLEQ_FOREACH(loop, &runq, next){
    if (loop->id==thread){
      printf("il existe dans head !%d\n", thread);
      printf("waiting thread de %d\n", loop->id);
      SIMPLEQ_INSERT_HEAD(&(loop->waiting_thread), current_thread, next);
      SIMPLEQ_INSERT_TAIL(&(sleepq), current_thread, next);
      SIMPLEQ_FOREACH(tmp, &(loop->waiting_thread), next){
	printf("thread n°%d\n", tmp->id);
      }
      previous_thread=current_thread;
      current_thread=SIMPLEQ_FIRST(&runq);
      SIMPLEQ_REMOVE_HEAD(&runq, next);
      printf("j'étais %d, je lance %d\n", previous_thread->id, thread_self());
      swapcontext(previous_thread->context, current_thread->context);
      current_thread=previous_thread;
      printf("Je suis revenu au contexte de %d\n", thread_self());
      *retval = current_thread->retval;
    }
    
  }

  return 0;
}

/* simplement placer retval dans le retval du thread self */
/* void thread_exit(void *retval){ */
/*   if ( SIMPLEQ_EMPTY(head) ) free(current_thread); */
/*   else{ */
/*     SIMPLEQ_FIRST(head)->retval=retval; */
/*     free(current_thread); */
/*     current_thread=SIMPLEQ_FIRST(head); */
/*     SIMPLEQ_REMOVE_HEAD(head, next); */
/*   } */
/* } */

void thread_exit(void *retval){
  struct thread * loop;
  struct thread *tmp;
  /* On initialise un thread qui va prendre la valeur du thread courant et aller dans la file des threads non utilisés */
  struct thread* thread_over;
  /* if ((thread_over = malloc (sizeof(struct thread))) == NULL){ */
  /*   //    fprintf(stderr, "Error malloc\n"); */
  /*   return; */
  /* } */
  printf("exit : waiting thread de %d\n", thread_self());
  SIMPLEQ_FOREACH(tmp, &(current_thread->waiting_thread), next){
	printf("thread n°%d\n", tmp->id);
  }
  // thread_over->context=malloc(sizeof(ucontext_t));
  thread_over=current_thread;

  /* On stocke retval dans le champ du thread qui a fini son exécution */
  printf("Le thread %d place %p dans son champ retval\n", thread_self(), retval);
  thread_over->retval=retval;
  SIMPLEQ_INSERT_TAIL(&overq, thread_over, next);

  SIMPLEQ_FOREACH(loop, &current_thread->waiting_thread, next){
    loop->retval = retval;
    SIMPLEQ_REMOVE(&sleepq, loop, thread, next);
    SIMPLEQ_INSERT_TAIL(&runq,loop,next);
  }

  if(!(SIMPLEQ_EMPTY(&runq))){
    /* Le thread courant devient la tête de la file */
    current_thread=SIMPLEQ_FIRST(&runq);
    /* On stocke la tête dans un pointeur */
    //	  struct thread* tmp=SIMPLEQ_FIRST(&runq);
	  
    SIMPLEQ_REMOVE_HEAD(&runq, next);
    printf("le nouveau thread %d\n", thread_self());
    /* Enfin on reprend le contexte du nouveau thread courant */
    setcontext(current_thread->context);
  }
  else {
    free_memory();

  }
}

int free_memory(){
 struct thread * loop;
    free(current_thread->context->uc_stack.ss_sp);
    free(current_thread->context);
    free(current_thread);
  SIMPLEQ_FOREACH(loop, &runq, next){
     free(loop->context->uc_stack.ss_sp);
    free(loop->context);
    free(loop);
  }
}
int thread_mutex_init(thread_mutex_t *mutex){
  mutex->dummy = -1;
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
  if (mutex->dummy == id)
    return EXIT_FAILURE;
  do{  
    while (mutex->dummy != -1){
      if(nanosleep(&time, &time2) < 0)
	perror("nanosleep\n");
    }
    mutex->dummy = id;
  } while (mutex->dummy != id);
  return EXIT_SUCCESS;
}


int thread_mutex_unlock(thread_mutex_t *mutex){
  thread_t id = thread_self();
  if (mutex->dummy == -1) {
    return EXIT_FAILURE;
  }
  if (mutex->dummy != id){
    return EXIT_FAILURE;
  }
  mutex->dummy = -1;
  return EXIT_SUCCESS;
}
