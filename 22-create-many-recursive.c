#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "thread.h"

/* test de plein de create-destroy récursif.
 *
 * valgrind doit etre content.
 * la durée du programme doit etre proportionnelle au nombre de threads donnés en argument.
 * jusqu'à combien de threads cela fonctionne-t-il ?
 *
 * support nécessaire:
 * - thread_create()
 * - retour sans thread_exit()
 * - thread_join() avec récupération de la valeur de retour
 */

static void * thfunc(void *_nb)
{
  unsigned long nb = (unsigned long) _nb;
  if ((unsigned long) nb > 0) {
    thread_t th;
    int err;
    void *res;
    err = thread_create(&th, thfunc, _nb-1);
    assert(!err);
    err = thread_join(th, &res);
    assert(!err);
    assert(res == _nb-1);
  }
  return _nb;
}

int main(int argc, char *argv[])
{
  unsigned long nb;
  struct timeval tv1;
  struct timeval tv2;

  gettimeofday(&tv1,NULL);

  if (argc < 2) {
    printf("argument manquant: nombre de threads\n");
	gettimeofday(&tv2,NULL);
    return -1;
  }

  nb = atoi(argv[1]);

  thfunc((void*) nb);
  
  gettimeofday(&tv2,NULL);

  printf("%ld threads créés et détruits récursivement\n", nb);

  double time = (tv2.tv_sec - tv1.tv_sec) + (tv2.tv_usec - tv1.tv_usec)/1000000.0;
  printf("time : %f\n",time);
  return 0;
}
