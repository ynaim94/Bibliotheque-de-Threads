#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>

#include "thread.h"

/* test de plein de create, puis plein de join quand ils ont tous fini
 *
 * valgrind doit etre content.
 * la durée du programme doit etre proportionnelle au nombre de threads donnés en argument.
 * jusqu'à combien de threads cela fonctionne-t-il ?
 *
 * support nécessaire:
 * - thread_create()
 * - thread_exit()
 * - thread_join() sans récupération de la valeur de retour
 */

static void * thfunc(void *dummy __attribute__((unused)))
{
  thread_exit(NULL);
}

int main(int argc, char *argv[])
{

	struct timeval tv1;
	struct timeval tv2;

	gettimeofday(&tv1,NULL);
  thread_t *th;
  int err, i, nb;

  if (argc < 2) {
    printf("argument manquant: nombre de threads\n");
	gettimeofday(&tv2,NULL);
    return -1;
  }

  nb = atoi(argv[1]);

  th = malloc(nb*sizeof(*th));
  if (!th) {
    perror("malloc");
    return -1;
  }

  /* on cree tous les threads */
  for(i=0; i<nb; i++) {
    err = thread_create(&th[i], thfunc, NULL);
    assert(!err);
  }

  /* on leur passe la main, ils vont tous terminer */
  for(i=0; i<nb; i++) {
    thread_yield();
  }

  /* on les joine tous, maintenant qu'ils sont tous morts */
  for(i=0; i<nb; i++) {
    err = thread_join(th[i], NULL);
    assert(!err);
  }

  free(th);

  gettimeofday(&tv2,NULL);
  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time : %f\n",time);
  printf("%d threads créés et détruits\n", nb);
  return 0;
}
