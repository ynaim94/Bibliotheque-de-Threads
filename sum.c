#include <stdio.h>
#include "thread.h"
#include "mutex.h"

int a[SIZE];
int shared_index = 0;
int total_sum = 0;
pthread_mutex_t mutex1;

void *sum(void *ignored)
{
 int index, sum = 0;
 do {
    pthread_mutex_lock(&mutex1);
    index = shared_index;
    shared_index++;
    pthread_mutex_unlock(&mutex1);

    if (index < SIZE)
        sum += *(a + index);
}
while
 (index < SIZE);

pthread_mutex_lock(&mutex1);
total_sum += sum;
pthread_mutex_unlock(&mutex1);

 return 0;
}

main(int argc, char *argv[])
 {
	 
	 if (argc < 2) {
		printf("argument manquant: entier x pour la taille du tableau et  pour le nombre de threads\n");
		return -1;
  }
	 int len = SIZE;
	 int nb = atoi(argv[1]);

  int i;
  thread_t thread_x[nb];
  void *res = NULL;
  pthread_mutex_init(&mutex1, NULL);

  for (i = 0; i < len; i++)
    a[i] = i+1;
  
  for (i = 0; i < nb ; i++){
	  thread_create(&thread_x[i],sum,NULL);
  
/*
  for (i = 0; i < nb; i++){
	  printf("On tente le %dieme join\n",i);*/
	  thread_join(thread_x[i] , &res);
	  printf("%dieme join réussi\n",i);
  }

  printf("The sum of 1 to %d is %d\n", SIZE, total_sum);
 }
