#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

#include "../src/thread.h"

int a[SIZE], swapped = 0;


void initialise_array() {
	int i;
	srand(time(NULL));
	for(i = 0; i < SIZE; i++)
		a[i] = rand() % SIZE;

	printf("Tableau d'origine :\n");
	for(i = 0; i < SIZE; i++)
		printf("%d ", a[i]);
	printf("\n");
}

void fusion(int i) {
	int tmp, mod;
	
	mod = i % 2;

	if (i != SIZE-1) {
		if(a[i] > a[i+1]) {	
			tmp = a[i];
			a[i] = a[i+1];
			a[i+1] = tmp;
			swapped = 1;
		}
	}
}

int main(int argc, char *argv[]) {

	struct timeval tv1;
	struct timeval tv2;
	int i;
	void *res = NULL;



	

	gettimeofday(&tv1,NULL);	

	if (argc < 2) {
		printf("argument manquant: nombre de threads\n");
		gettimeofday(&tv2,NULL);
	return -1;
  }

	int len = atoi(argv[1]);

	pthread_t thread[len];

	initialise_array();

	do {

		swapped = 0;
		
		for(i = 0; i < len; i+=2)
			thread_create(&thread[i], &fusion, i);
		for(i = 0; i < len; i+=2)
			thread_join(thread[i], &res);

		swapped = 0;

		for(i = 1; i < len; i+=2)
			thread_create(&thread[i], &fusion, i);
		for(i = 1; i < len; i+=2)
			thread_join(thread[i], &res);

	} while(swapped == 1);

	gettimeofday(&tv2,NULL);
	
	printf("Tableau trié :\n");
	for(i = 0; i < SIZE; i++)
		printf("%d ", a[i]);
	printf("\n");

	double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
	printf("time : %f\n",time);

	exit(0);
}
