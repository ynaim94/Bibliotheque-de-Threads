#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <valgrind/valgrind.h>
#include "queue.h"
#include "thread.h"

#define ERROR -1
#define MAIN_ID 0

/*liste pour les threads prêtes*/
static struct queue runq = SIMPLEQ_HEAD_INITIALIZER(runq);
/*liste pour les threads terminés*/
static struct queue overq =  SIMPLEQ_HEAD_INITIALIZER(overq);

struct timeval tv[2];
typedef void* ret;
thread_t current;
static int firstThread = 1;

/*structure de thread*/
struct thread {
  thread_t id;
  ucontext_t* context;
  ret retval;
  struct queue waiting_thread;
  SIMPLEQ_ENTRY(thread) next;
  int valgrind_stackid;
} *current_thread;


/*prendre en considération le retval si un thread sort avec un return sans thread_exit*/
void func2(void *(*func)(void *), void *funcarg){
  void *retval;
  /*fonction entrée en paramétre dans thread_create*/
  retval = func(funcarg);
  /*appel de thread_exit pour récupérer le retval*/
  thread_exit(retval );
}

/*fonction appelée dans les fonctions thread_self et thread_yield pour assurer une préemption pseudo-copérative*/
void preemption(){
  unsigned long us;
  /*temps au moment de l'appel à une fonction qui peut faire la préemption*/
  gettimeofday(&tv[1], NULL);
  /*temps d'execution écoulé du thread courant*/
  us = (tv[1].tv_sec-tv[0].tv_sec)*1000000+(tv[1].tv_usec-tv[0].tv_usec);
  /*Si le thread s'execute pour plus de 100 us, on fait une préemption*/
  if(us>100){
     thread_yield();
   }
}

/*Fonction pour libérer toute la mémoire encore allouée à la sortie du programme*/
void free_memory(){
  struct thread * loop;
  VALGRIND_STACK_DEREGISTER(current_thread->valgrind_stackid);
  /*libérer les malloc du thread courant*/
  free(current_thread->context->uc_stack.ss_sp);
  free(current_thread->context);
  free(current_thread);
  int length = 0;
  /*libérer les malloc des threads terminés*/
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

/* initialisation de la première thread : la main */
int create_main_thread(){
  ucontext_t* ctx;
  firstThread = 0;
  /*allouer de l'espace mémoire pour le thread du main*/
  current_thread = malloc (sizeof(struct thread));
  /*allouer de l'espace mémoire pour le contexte du thread du main*/
  ctx=malloc(sizeof(ucontext_t));  
  getcontext(ctx);
  ctx->uc_stack.ss_size = 64*1024;
  /*allouer de l'espace mémoire pour la pile du thread du main*/
  if (((ctx->uc_stack.ss_sp) = malloc(ctx->uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error malloc\n");
    return ERROR;
  }
  current_thread->context=ctx;
  /*initialiser la queue des threads qui attendent la main suite à un join */
  SIMPLEQ_INIT(&current_thread->waiting_thread);
  /*retval du thread NULL par défaut*/
  current_thread->retval = NULL;
 /*assigner l'ID au thread main*/
  current_thread->id = MAIN_ID;
  /*aider valgrind à repérer les mallocs*/
  current_thread->valgrind_stackid = VALGRIND_STACK_REGISTER((current_thread->context)->uc_stack.ss_sp, (current_thread->context)->uc_stack.ss_sp + (current_thread->context)->uc_stack.ss_size);
  /*initialisation de tv[0]*/
  gettimeofday(&tv[0], NULL);
  /*libérer toute la mémoire allouée à la fin du programme*/
  on_exit((void(*)(void)) free_memory, NULL);
  
}


thread_t thread_self(void){
  /*préemption pseudo-copérative*/ 
 preemption();
 /*créer le thread main s'il n'est pas encore cré*/
   if (firstThread){
     create_main_thread();
   } 
   /*retourner l'ID du thread courant*/
   return current_thread->id;
}

int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg) {
  static int i = 1;
  struct thread* t1;
  /*préemption pseudo-copérative*/ 
  preemption();
  /*créer le thread main s'il n'est pas encore crée*/
  if (firstThread){
    create_main_thread();
  }
  /*contrôle de saisie :  ID non NULL*/
  if (newthread == NULL){
    fprintf(stderr, "newthread not initialized\n");
    return ERROR;
  }
  /*allouer de l'espace mémoire du thread*/
  if ((t1 = malloc (sizeof(struct thread))) == NULL){
      fprintf(stderr, "Error malloc\n");
    return ERROR;
  }
    /*allouer de l'espace mémoire pour le contexte du thread*/
  t1->context=malloc(sizeof(ucontext_t));
  getcontext((t1->context));
  /*allouer de l'espace mémoire pour la pile du thread*/
  (t1->context)->uc_stack.ss_size = 64*1024;
  if (((t1->context)->uc_stack.ss_sp = malloc((t1->context)->uc_stack.ss_size)) == NULL){
    fprintf(stderr, "Error malloc\n");
    return ERROR;
  }
  /*aider valgrind à repérer les mallocs*/
  t1->valgrind_stackid = VALGRIND_STACK_REGISTER((t1->context)->uc_stack.ss_sp, (t1->context)->uc_stack.ss_sp + (t1->context)->uc_stack.ss_size);
  (t1->context)->uc_link =  NULL;
  /* créer le context du thread pour qu'il execute sa fonction puis l'exit dans tout les cas*/
  makecontext((t1->context), (void (*)(void)) func2 ,2,func, funcarg);
  /*assigner un ID au thread*/
  t1->id= i;
  *newthread = i;
  t1->retval=NULL;
    /*initialiser la queue des threads qui attendent la main suite à un join */
  SIMPLEQ_INIT(&t1->waiting_thread);
  /*insérer le thread crée dans la liste des threads prêts*/
  SIMPLEQ_INSERT_TAIL(&runq, t1, next);
  i++;
  return 0;
}



int thread_yield(void){
  struct thread* previous_thread;
  /*créer le thread main s'il n'est pas encore crée*/
  if (firstThread){
    create_main_thread();
  }
  /*S'il n'y a pas de threads prêts, le thread continue à s'executer*/
  if ( SIMPLEQ_EMPTY(&runq) ){
    return 0;
  }
  /*S'il y a des threads prêts*/
  else{
    /*inserer le thread courant à la fin de la file des threads prêts */
    SIMPLEQ_INSERT_TAIL(&runq, current_thread, next);
    /*sauvegarder le thread courant pour la suite */
    previous_thread=current_thread;
    /* Rendre la main à la thread courante à la fin */
    SIMPLEQ_FIRST(&runq)->context->uc_link=current_thread->context;
    /* La tête de la file des threads prêts devient le thread courant */
    current_thread=SIMPLEQ_FIRST(&runq);
    /*supprimer la tête de la file des threads prêts */
    SIMPLEQ_REMOVE_HEAD(&runq, next);
    /*Remise à zero de tv[0] pour mesurer le temps d'execution du nouveau thread*/
    gettimeofday(&tv[0], NULL);
    /*changer le contexte*/
    swapcontext(previous_thread->context, current_thread->context);
    /*remettre en place le thread courant */
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

  /*chercher le thread dans la file des threads terminés*/
  SIMPLEQ_FOREACH(loop_rest,&overq, next){
    if (loop_rest->id==thread){
      if (retval == NULL){
	/* Ne rien faire */
      }
      else {
	/*récupérer le retval du thread cherché*/
	*retval=loop_rest->retval;
      }
      return EXIT_SUCCESS; }
  }

  /*chercher dans un premier temps si le thread recherché est dans la file des threads prêts */
  SIMPLEQ_FOREACH(loop, &runq, next){
    if (loop->id==thread){
      /*inserer le thread courant à la file locale des threads endormis */
      SIMPLEQ_INSERT_HEAD(&(loop->waiting_thread), current_thread, next);
      /*sauvegarder le thread courant pour la suite */
      previous_thread=current_thread;
      /* La tête de la file des threads prêts devient le thread courant */
      current_thread=SIMPLEQ_FIRST(&runq);
      /*supprimer la tête de la file des threads prêts */
      SIMPLEQ_REMOVE_HEAD(&runq, next);
       /*Remise à zero de tv[0] pour mesurer le temps d'execution du nouveau thread*/
      gettimeofday(&tv[0], NULL);
      /*cahnger le contexte*/
      swapcontext(previous_thread->context, current_thread->context);
      /*remettre en place le thread courant */
      current_thread=previous_thread;
      /*stocker la valeur de retour du thread courant dans l'adresse retval*/
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
  /*initialiser un thread qui va prendre la valeur du thread courant et aller dans la file des threads non utilisés */
  struct thread* thread_over;
  thread_over=current_thread;
  /*stocker retval dans le champ du thread qui a fini son exécution */
  thread_over->retval=retval;
  /*assigner la valeur de retval aux threads qui attendent via join ce thread*/
  SIMPLEQ_FOREACH(loop, &current_thread->waiting_thread, next){
    loop->retval = retval;
    SIMPLEQ_INSERT_TAIL(&runq,loop,next);
  }
  /*Si on a encore des threads prêts*/
  if(!(SIMPLEQ_EMPTY(&runq))){
     SIMPLEQ_INSERT_TAIL(&overq, thread_over, next);
    /* Le thread courant devient la tête de la file */
    current_thread=SIMPLEQ_FIRST(&runq);
    /*supprimer la tête de la file des threads prêts */
    SIMPLEQ_REMOVE_HEAD(&runq, next);
    /*Remise à zero de tv[0] pour mesurer le temps d'execution du nouveau thread*/
    gettimeofday(&tv[0], NULL);
    /* Enfin on reprend le contexte du nouveau thread courant */
    setcontext(current_thread->context);
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


