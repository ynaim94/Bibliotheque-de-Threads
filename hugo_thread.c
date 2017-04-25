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

static SIMPLEQ_HEAD(queue, thread) head = SIMPLEQ_HEAD_INITIALIZER(head);

typedef void* ret;

struct queue  *head1;

struct queue *head_rest;
struct thread {
  thread_t id;
  ucontext_t* context;
  ret retval;
  SIMPLEQ_ENTRY(thread) next;
}  current_thread;

thread_t current;


static int firstThread = 1;

int create_main_thread(){

  /* initialisation du thread courant */
  ucontext_t ctx;
  firstThread = 0;
  current_thread.id = MAIN_ID;
  getcontext(&ctx);
  current_thread.context=&ctx;
  current_thread.retval = 0; // ??

  /* initialisation de la file de thread sans activité */
  
  head_rest=malloc(sizeof(struct queue));
  SIMPLEQ_INIT(head_rest);
}

thread_t thread_self(void){

  if (firstThread){
    create_main_thread();
  }

  return current_thread.id;
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
    fprintf(stderr, "Error malloc\n");
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
  makecontext((t1->context), (void (*)(void)) func ,1, funcarg);
  t1->id= i;
  *newthread = i;
  t1->retval=0;

  SIMPLEQ_INSERT_TAIL(&head, t1, next);
  i++;
  
  return 0;
}

/*
  int thread_yield(void){
  //TODO: traiter cas d'erreur
  if ( SIMPLEQ_EMPTY(&head) ){
  return 0;
  }
  ucontext_t ctx = current_thread->context;
  struct thread *thread = current_thread;
  current_thread = SIMPLEQ_FIRST(&head);
  current_thread->context.uc_link=&thread->context;//Rend la main à la thread courante à la fin
  SIMPLEQ_REMOVE_HEAD(&head, next);
  SIMPLEQ_INSERT_TAIL(&head, thread, next);
  setcontext(&(current_thread->context));
  return 0;
  }
*/



int thread_yield(void){
  struct thread previous_thread;
  if (firstThread){
    create_main_thread();
  }
  
  if ( SIMPLEQ_EMPTY(&head) ){
    printf("FIFO vide ! \n");
    return 0;
  }
  else{
    /* création d'un thread temporaire */
    struct thread *tmp=malloc(sizeof(struct thread));
    tmp->context=malloc(sizeof(ucontext_t));
    /* affectation du thread courant à ce thread */
    *tmp=current_thread;
    /* insertion du thread courant à la fin de la file */
    SIMPLEQ_INSERT_TAIL(&head, tmp, next);
    /* On sauvegarde le thread courant pour la suite */
    previous_thread=current_thread;
    /* Rend la main à la thread courante à la fin */
    SIMPLEQ_FIRST(&head)->context->uc_link=current_thread.context;
    /* On affecte à tmp la valeur précédemment allouée pour la tête */
    tmp=SIMPLEQ_FIRST(&head);
    /* La tête devient le thread courant */
    current_thread=*tmp;
    /* On supprime la tête de la file */
    SIMPLEQ_REMOVE_HEAD(&head, next);
    /* Et on désalloue l'espace */
    free(tmp);
    /* On effectue le swap */
    swapcontext(previous_thread.context, current_thread.context);
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
  struct thread destination_thread;
  struct thread previous_thread;
  /* On cherche dans un premier temps si le thread recherché est dans la file des threads actifs */
  SIMPLEQ_FOREACH(loop, &head, next){
    if (loop->id==thread){
      printf("il existe dans head !%d\n", thread);
      /* Si on le trouve, on stocke sa valeur dans un thread */
      destination_thread=*loop;
      /* On fait en sorte que une fois exécuté, il revienne */
      destination_thread.context->uc_link=current_thread.context;
      /* On sauvegarde le thread courant */
      previous_thread=current_thread;
      /* Le nouveau thread courant est le thread ciblé */
      current_thread=destination_thread;
      /* On le retire de la file */
      SIMPLEQ_REMOVE(&head, loop, thread, next);
      /* On supprime l'espace alloué */
      free(loop);
      swapcontext(previous_thread.context, current_thread.context);
      /* On revient à la normale */
      current_thread=previous_thread;
      break; }
  }
  /* Sécurité mais normalement avec la boucle précédente, cela ne devrait pas arriver */
  if(head_rest==NULL){
    printf("Error malloc head_rest\n");
    return ERROR;
  }
  /* On cherche le thread dans la file des threads non-utilisés */
  SIMPLEQ_FOREACH(loop_rest, head_rest, next){
    if (loop_rest->id==thread){
      printf("il existe dans head_rest\n");
      if (retval == NULL){
	/* Ne rien faire */
	printf("la valeur de retour est ignorée\n");
      }
      else {
	printf("on place dans retval la valeur %p\n", loop_rest->retval);
	*retval=loop_rest->retval;
      }
      break; }
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
  /* On initialise un thread qui va prendre la valeur du thread courant et aller dans la file des threads non utilisés */
  struct thread* thread_rest;
  if ((thread_rest = malloc (sizeof(struct thread))) == NULL){
    fprintf(stderr, "Error malloc\n");
    return;
  }
  thread_rest->context=malloc(sizeof(ucontext_t));
  *thread_rest=current_thread;
  /* On stocke retval dans le champ du thread qui a fini son exécution */
  printf("Le thread %d place %p dans son champ retval\n", thread_self(), retval);
  thread_rest->retval=retval;
  SIMPLEQ_INSERT_TAIL(head_rest, thread_rest, next);

  /* Le thread courant devient la tête de la file */
  current_thread=*(SIMPLEQ_FIRST(&head));
  /* On stocke la tête dans un pointeur */
  struct thread* tmp=SIMPLEQ_FIRST(&head);
  struct thread * loop;
  SIMPLEQ_FOREACH(loop, &head, next){ 
    printf("thread n°%d\n", loop->id); 
  }
  SIMPLEQ_FOREACH(loop, head_rest, next){ 
    printf("thread rest n°%d\n", loop->id); 
  }
  SIMPLEQ_REMOVE_HEAD(&head, next);
    SIMPLEQ_FOREACH(loop, &head, next){ 
    printf("thread n°%d\n", loop->id); 
  }
  SIMPLEQ_FOREACH(loop, head_rest, next){ 
    printf("thread rest n°%d\n", loop->id); 
  }
  /* Une fois la tête supprimée, on désalloue l'espace associé */
  free(tmp);
  printf("le nouveau thread %d\n", thread_self());
  /* Enfin on reprend le contexte du nouveau thread courant */
  setcontext(current_thread.context);
}


/* void thread_exit(void *retval){ */
/*   current = thread_self(); */
/*   printf("le thread %d appelle thread_exit\n", current); */
  
/*   struct thread * loop; */
/*   SIMPLEQ_FOREACH(loop, &head, next){ */
/*     printf("thread n°%d\n", loop->id); */
/*   } */
/*   //struct thread current; */
/*   SIMPLEQ_FOREACH(loop, &head, next){ */
/*     if (current == loop->id){ */
/*       SIMPLEQ_REMOVE(&head, loop, thread, next); */
/*     } */
/*   } */
/*   SIMPLEQ_FOREACH(loop, &head, next){ */
/*     if(loop->id != current){ */
/*       printf("le thread %d est appelé\n", loop->id); */
/*       *current_thread=*loop; */
/*       setcontext(current_thread->context); */
/*     } */
/*   } */

/* } */

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
