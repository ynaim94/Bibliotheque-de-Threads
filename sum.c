#include <stdio.h>
#include <sys/time.h>
#include "thread.h"

int a[SIZE];
int shared_index = 0;
int total_sum = 0;
thread_mutex_t mutex1;

void *sum(void *ignored)//changer le nom du paramètre
{
 int index, sum = 0;
 do {
    thread_mutex_lock(&mutex1);
    index = shared_index;
    shared_index++;
    thread_mutex_unlock(&mutex1);

    if (index < SIZE)
        sum += *(a + index);
}
while
 (index < SIZE);

thread_mutex_lock(&mutex1);
total_sum += sum;
thread_mutex_unlock(&mutex1);

 return 0;
}

main(int argc, char *argv[])
 {
	 struct timeval tv1;
	 struct timeval tv2;
	 gettimeofday(&tv1,NULL);
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
  gettimeofday(&tv2,NULL);

  printf("The sum of 1 to %d is %d\n", SIZE, total_sum);

  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time : %f\n",time);
 }
