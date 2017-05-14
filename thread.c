#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include "queue.h"
#include "thread.h"
#include <valgrind/valgrind.h>

#define ERROR -1
#define MAIN_ID 0


static struct queue runq = SIMPLEQ_HEAD_INITIALIZER(runq);
static struct queue overq =  SIMPLEQ_HEAD_INITIALIZER(overq);

typedef void* ret;
void free_memory();
 
struct thread {
  thread_t id;
  ucontext_t* context;
  ret retval;
  struct queue waiting_thread;
  SIMPLEQ_ENTRY(thread) next;
  int valgrind_stackid;
} *current_thread;


thread_t current;


static int firstThread = 1;

void func2(void *(*func)(void *), void *funcarg){
  struct thread* loop;
  void *retval;
  retval = func(funcarg);
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
  current_thread->valgrind_stackid = VALGRIND_STACK_REGISTER((current_thread->context)->uc_stack.ss_sp, (current_thread->context)->uc_stack.ss_sp + (current_thread->context)->uc_stack.ss_size);
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
  t1->valgrind_stackid = VALGRIND_STACK_REGISTER((t1->context)->uc_stack.ss_sp, (t1->context)->uc_stack.ss_sp + (t1->context)->uc_stack.ss_size);
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
    // printf("current_thread id : %d",current_thread->id);
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
  struct thread* loop;
  struct thread* loop_rest;
  struct thread *previous_thread;
  struct thread *tmp;

  /* On cherche le thread dans la file des threads non-utilisés */
  SIMPLEQ_FOREACH(loop_rest,&overq, next){
    if (loop_rest->id==thread){
      if (retval == NULL){
	/* Ne rien faire */
      }
      else {
	*retval=loop_rest->retval;
      }
      return EXIT_SUCCESS; }
  }

  
  /* On cherche dans un premier temps si le thread recherché est dans la file des threads actifs */
  SIMPLEQ_FOREACH(loop, &runq, next){
    if (loop->id==thread){
      SIMPLEQ_INSERT_HEAD(&(loop->waiting_thread), current_thread, next);
      previous_thread=current_thread;
      current_thread=SIMPLEQ_FIRST(&runq);
      SIMPLEQ_REMOVE_HEAD(&runq, next);
      swapcontext(previous_thread->context, current_thread->context);
      current_thread=previous_thread;
      if(retval!=NULL){
	*retval = current_thread->retval;
      }
    }
    
  }

  return 0;
}

void thread_exit(void *retval){
  struct thread * loop;
  struct thread *tmp;
  /* On initialise un thread qui va prendre la valeur du thread courant et aller dans la file des threads non utilisés */
  struct thread* thread_over;
  thread_over=current_thread;
  /* On stocke retval dans le champ du thread qui a fini son exécution */
  thread_over->retval=retval;
  SIMPLEQ_FOREACH(loop, &current_thread->waiting_thread, next){
    loop->retval = retval;
    SIMPLEQ_INSERT_TAIL(&runq,loop,next);
  }

  if(!(SIMPLEQ_EMPTY(&runq))){
     SIMPLEQ_INSERT_TAIL(&overq, thread_over, next);
    /* Le thread courant devient la tête de la file */
    current_thread=SIMPLEQ_FIRST(&runq);
    SIMPLEQ_REMOVE_HEAD(&runq, next);
    /* Enfin on reprend le contexte du nouveau thread courant */
    setcontext(current_thread->context);
  }
}

void free_memory(){
  struct thread * loop;
  VALGRIND_STACK_DEREGISTER(current_thread->valgrind_stackid);
  free(current_thread->context->uc_stack.ss_sp);
  free(current_thread->context);
  free(current_thread);
  int length = 0;
  SIMPLEQ_FOREACH(loop, &overq, next){
    VALGRIND_STACK_DEREGISTER(loop->valgrind_stackid);
    free(loop->context->uc_stack.ss_sp);
    free(loop->context);
    length++;
  }
  int i = 0;
  struct thread* tmp[length];
  SIMPLEQ_FOREACH(loop, &overq, next){
    tmp[i]=loop;
    i++;
  }
  for(i = 0; i<length; i++){
    free(tmp[i]);
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
    return EXIT_SUCCESS;
  }
  else if (mutex->locker==thread_self()){
    perror("Deux lock\n");
    return EXIT_FAILURE;
  }
  else {
    SIMPLEQ_INSERT_TAIL(&(mutex->mutexq), current_thread, next);
    do {
      previous_thread=current_thread;
      current_thread=SIMPLEQ_FIRST(&runq);
      SIMPLEQ_REMOVE_HEAD(&runq, next);
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
    if (!SIMPLEQ_EMPTY(&(mutex->mutexq))){
      tmp=SIMPLEQ_FIRST(&(mutex->mutexq));
      SIMPLEQ_REMOVE_HEAD(&(mutex->mutexq),next);
      SIMPLEQ_INSERT_TAIL(&runq, tmp, next);
      mutex->locker=tmp->id;
    }
    else {
      mutex->locker=-1;
    }
    return EXIT_SUCCESS;
  }
}


