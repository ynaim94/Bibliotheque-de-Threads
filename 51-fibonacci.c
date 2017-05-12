#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include "thread.h"

/* fibonacci.
 *
 * la durée doit être proportionnel à la valeur du résultat.
 * valgrind doit être content.
 * jusqu'à quelle valeur cela fonctionne-t-il ?
 *
 * support nécessaire:
 * - thread_create()
 * - thread_join() avec récupération de la valeur de retour
 * - retour sans thread_exit()
 */

static void * fibo(void *_value)
{
  thread_t th, th2;
  int err;
  void *res = NULL, *res2 = NULL;
  unsigned long value = (unsigned long) _value;

  /* on passe un peu la main aux autres pour eviter de faire uniquement la partie gauche de l'arbre */
  thread_yield();

  if (value < 3)
    return (void*) 1;

  err = thread_create(&th, fibo, (void*)(value-1));
  assert(!err);
  err = thread_create(&th2, fibo, (void*)(value-2));
  assert(!err);

  err = thread_join(th, &res);
  assert(!err);
  err = thread_join(th2, &res2);
  assert(!err);

  return (void*)((unsigned long) res + (unsigned long) res2);
}

int main(int argc, char *argv[])
{
  struct timeval tv;
  struct timeval start_tv;
  
  gettimeofday(&start_tv, NULL);
  
  double elapsed = 0.0;
  
  unsigned long value, res;
  
  if (argc < 2) {
    printf("argument manquant: entier x pour lequel calculer fibonacci(x)\n");
    return -1;
  }

  value = atoi(argv[1]);
  res = (unsigned long) fibo((void *)value);
  gettimeofday(&tv, NULL);
  elapsed = (tv.tv_sec - start_tv.tv_sec) +
    (tv.tv_usec - start_tv.tv_usec) / 1000000.0;
  printf("fibo de %ld = %ld in %f \n", value, res, elapsed);
  
  return 0;
}
