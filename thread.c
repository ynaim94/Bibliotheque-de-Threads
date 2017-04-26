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

struct thread {
  thread_t id;
  ucontext_t* context;
  ret retval;
  SIMPLEQ_ENTRY(thread) next;
}  *t2, *current_thread = NULL;

thread_t current;


static int firstThread = 1;

int create_main_thread(){

  firstThread = 0;
  current_thread =  malloc (sizeof(struct thread));
  current_thread->id = MAIN_ID;
  current_thread->context=malloc(sizeof(ucontext_t));
  getcontext((current_thread->context));
  current_thread->retval = 0; // ??
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
  printf("yield lancé par thread %d\n", thread_self());
  struct thread* loop;
  printf("liste des threads \n");
  SIMPLEQ_FOREACH(loop, &head, next){
    printf("thread n°%d\n", loop->id);
  }
  struct thread previous_thread;
  if (firstThread){
    create_main_thread();
  }
  
  if ( SIMPLEQ_EMPTY(&head) ){
    printf("FIFO vide ! \n");
    return 0;
  }
  else{
    if(current_thread->id != MAIN_ID) {
      SIMPLEQ_INSERT_TAIL(&head, current_thread, next);// ajouter la thread courante à la fin de la Queue 
      //      current_thread->context.uc_link=&main_thread->context; //Rend la main au thread main à la fin    ****Pas besoin****
    }
    //ucontext_t ctx=current_thread->context;
    previous_thread=*current_thread;
    SIMPLEQ_FIRST(&head)->context->uc_link=current_thread->context;//Rend la main à la thread courante à la fin
    current_thread=SIMPLEQ_FIRST(&head); // La thread à la tête de la Queue devient la thread courante
    SIMPLEQ_REMOVE_HEAD(&head, next); //On l'enlève de la Queue
    swapcontext(previous_thread.context, current_thread->context); //On la lance
    *current_thread=previous_thread;
    //*current_thread=previous_thread;
    return EXIT_SUCCESS;
  }
}



/* Permet de placer la valeur de retour du thread spécifié à l'adresse retval */
int thread_join(thread_t thread, void **retval){
  /*Queue head1 init*/
  head1=malloc (sizeof (struct queue));
  SIMPLEQ_INIT(head1);
  /************/
  
  struct thread* loop;
  /* on parcourt la file à la recherche du thread. 
     S'il est présent, on sauvegarde les données qui nous intéresse
     Sinon, on sort.
     Tant que le thread est présent dans la file, on recommence la boucle jusqu'à ce qu'il n'y soit plus. A ce moment, on sort du do while. */
  //do{
  SIMPLEQ_FOREACH(loop, &head, next){
    if (loop->id==thread){ //on cherche si thread est bien dans la liste
      printf("il existe !%d\n", thread);
      //is_present=true;
      loop->context->uc_link = current_thread->context; //retourne la main à la thread courante lorsqu'elle termine
      SIMPLEQ_INSERT_TAIL(head1, current_thread, next);//On sauvegarde la thread courante dans une file à part
      current_thread=loop; //loop devient la thread courante
      swapcontext(loop->context, current_thread->context);
      SIMPLEQ_REMOVE(&head, loop, thread, next);
      break; }
  }
  /*    if(tmp== -1 && compteur==0){
	printf("%d n'appartient pas à la liste\n", thread);
	return -1;
	}
	if(tmp== -1 && compteur>=0){
	printf("%d n'appartient plus à la liste\n", thread);
	is_present=false;
	}
	
	tmp= -1;
	sched_yield();
	compteur++;
	} while (is_present);
  
	if(retval != NULL){// si retval est NULL, ne rien faire, sinon ..
    
	*retval=tmp_ret; //..on place la valeur de retour du thread dans retval
	}
	Free Queue*/
  free(head1);
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
  current = thread_self();
  printf("le thread %d appelle thread_exit\n", current);
  
  struct thread * loop;
    SIMPLEQ_FOREACH(loop, &head, next){
    printf("thread n°%d\n", loop->id);
  }
  //struct thread current;
  SIMPLEQ_FOREACH(loop, &head, next){
    if (current == loop->id){
      loop->retval = retval;
      SIMPLEQ_REMOVE(&head, loop, thread, next);
    }
  }
  SIMPLEQ_FOREACH(loop, &head, next){
    if(loop->id != current){
      printf("le thread %d est appelé\n", loop->id);
      *current_thread=*loop;
      setcontext(current_thread->context);
    }
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
