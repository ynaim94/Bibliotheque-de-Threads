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


static struct queue runq = SIMPLEQ_HEAD_INITIALIZER(runq);
static struct queue overq =  SIMPLEQ_HEAD_INITIALIZER(overq);
static struct queue sleepq =  SIMPLEQ_HEAD_INITIALIZER(sleepq);

typedef void* ret;
void free_memory();
 
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
  ucontext_t* ctx;
  firstThread = 0;
  current_thread = malloc (sizeof(struct thread));
  current_thread->id = MAIN_ID;
  SIMPLEQ_INIT(&current_thread->waiting_thread);
  ctx=malloc(sizeof(ucontext_t));  
  getcontext(ctx);
  ctx->uc_stack.ss_size = 64*1024;
  if (((ctx->uc_stack.ss_sp) = malloc(ctx->uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error malloc\n");
    return ERROR;
  }
  current_thread->context=ctx;
  current_thread->retval = NULL;
  on_exit((void(*)(void)) free_memory, NULL);
  
}

thread_t thread_self(void){

  if (firstThread){
    create_main_thread();
  }

  return current_thread->id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
  // ucontext_t ctx;

  static int i = 1;
  struct thread* t1;

  if (firstThread){
    printf("on crée le main \n");
    create_main_thread();
  }

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
  
  SIMPLEQ_INIT(&t1->waiting_thread);
  SIMPLEQ_INSERT_TAIL(&runq, t1, next);
  i++;
  
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
    printf("current_thread id : %d\n",current_thread->id);
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
      printf("test\n");
      swapcontext(previous_thread->context, current_thread->context);
      current_thread=previous_thread;
      printf("Je suis revenu au contexte de %d\n", thread_self());
      if(retval!=NULL){
	*retval = current_thread->retval;
      }
      printf("wat\n");
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
 

  SIMPLEQ_FOREACH(loop, &current_thread->waiting_thread, next){
    loop->retval = retval;
    SIMPLEQ_REMOVE(&sleepq, loop, thread, next);
    SIMPLEQ_INSERT_TAIL(&runq,loop,next);
  }

  if(!(SIMPLEQ_EMPTY(&runq))){
     SIMPLEQ_INSERT_TAIL(&overq, thread_over, next);
    /* Le thread courant devient la tête de la file */
    current_thread=SIMPLEQ_FIRST(&runq);
    /* On stocke la tête dans un pointeur */
    //	  struct thread* tmp=SIMPLEQ_FIRST(&runq);
	  
    SIMPLEQ_REMOVE_HEAD(&runq, next);
    printf("le nouveau thread %d\n", thread_self());
    /* Enfin on reprend le contexte du nouveau thread courant */
    setcontext(current_thread->context);
  }
}

void free_memory(){
  // printf("entrée free_memory\n");
  struct thread * loop;
  free(current_thread->context->uc_stack.ss_sp);
  free(current_thread->context);
  free(current_thread);
  SIMPLEQ_FOREACH(loop, &overq, next){
    free(loop->context->uc_stack.ss_sp);
    free(loop->context);
    free(loop);
  }
}

int thread_mutex_init(thread_mutex_t *mutex){
  SIMPLEQ_INIT(&(mutex->mutexq));
  mutex->locker=-1;
  return EXIT_SUCCESS;
}

int thread_mutex_destroy(thread_mutex_t *mutex){
  struct thread* loop;
  SIMPLEQ_FOREACH(loop, &(mutex->mutexq), next){
    SIMPLEQ_REMOVE_HEAD(&(mutex->mutexq), next);
  }
  return EXIT_SUCCESS;
}

int thread_mutex_lock(thread_mutex_t *mutex){
  struct thread* previous_thread;
  struct thread* loop;
  if (mutex->locker==-1){
    mutex->locker=thread_self();
    //printf("Je suis %d j'ai pris le lock\n", thread_self());
    return EXIT_SUCCESS;
  }
  else {
    SIMPLEQ_INSERT_TAIL(&(mutex->mutexq), current_thread, next);
    do {
      //printf("Je suis %d, je suis en attente, voici la waiting\n", thread_self());
      /* SIMPLEQ_FOREACH(loop, &(mutex->mutexq),next){ */
      /* 	printf("id : %d \n", loop->id); */
      /* } */
      previous_thread=current_thread;
      current_thread=SIMPLEQ_FIRST(&runq);
      SIMPLEQ_REMOVE_HEAD(&runq, next);
      //printf("Je suis dans la boucle du lock de %d et je lance %d\n", previous_thread->id, thread_self());
      swapcontext(previous_thread->context, current_thread->context);
      current_thread=previous_thread;
    } while(mutex->locker!=thread_self() && mutex->locker!=-1);
    mutex->locker=thread_self();
  }
  return EXIT_SUCCESS;
}


int thread_mutex_unlock(thread_mutex_t *mutex){
  struct thread* tmp;
  if (mutex->locker!=thread_self()){
    return EXIT_FAILURE;
  }
  else {
    //printf("Je suis %d, je libère le mutex\n", thread_self());
    if (!SIMPLEQ_EMPTY(&(mutex->mutexq))){
      tmp=SIMPLEQ_FIRST(&(mutex->mutexq));
      SIMPLEQ_REMOVE_HEAD(&(mutex->mutexq),next);
      SIMPLEQ_INSERT_TAIL(&runq, tmp, next);
      mutex->locker=tmp->id;
    }
    return EXIT_SUCCESS;
  }
}
