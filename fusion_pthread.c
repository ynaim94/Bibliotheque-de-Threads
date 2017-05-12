#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
 #define DIM 10

int a[DIM], swapped = 0;
pthread_t thread[DIM];

void v_initiate() {
	int i;
	srand(time(NULL));
	for(i = 0; i < DIM; i++)
		a[i] = rand() % DIM;

	printf("Vetor Original\n");
	for(i = 0; i < DIM; i++)
		printf("%d ", a[i]);
	printf("\n");
}

void bubble(int i) {
	int tmp, mod;
	
	mod = i % 2;

	if (i != DIM-1) {
		if(a[i] > a[i+1]) {	
			tmp = a[i];
			a[i] = a[i+1];
			a[i+1] = tmp;
			swapped = 1;
		}
	}
	//printf("swap par: %d\nswap impar: %d\n", swapped_par, swapped_impar);
}

int main() {
	int i, par_impar = 0;
	v_initiate();

	printf("Vetor Ordenado\n3 5 6 4 1 2 9 7 8 10\n");
		
	do {

		swapped = 0;
		
		for(i = 0; i < DIM; i+=2)
			pthread_create(&thread[i], NULL, &bubble, i);
		for(i = 0; i < DIM; i+=2)
			pthread_join(thread[i], NULL);

		swapped = 0;

		for(i = 1; i < DIM; i+=2)
			pthread_create(&thread[i], NULL, &bubble, i);
		for(i = 1; i < DIM; i+=2)
			pthread_join(thread[i], NULL);

	//printf("swap par: %d - swap impar: %d\n", swapped_par, swapped_impar);

	} while(swapped == 1);
	
	printf("Vetor Ordenado\n");
	for(i = 0; i < DIM; i++)
		printf("%d ", a[i]);
	printf("\n");

	exit(0);
}
