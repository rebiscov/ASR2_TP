#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

long N, P;
int *a; 
long res = 0;
long *res_i = NULL;

void *prodvec(void *j) {
	long pos = (long) j;
	for (long i = (N/P)*pos; i < (N/P)*(pos+1); i++) {
		res_i[pos] += a[i];
	}
	pthread_exit(0);
}

int main(int argc, char *argv[]) {
	void *status;

	if (argc != 3)
		fprintf(stderr, "Usage %s/ N P \n", argv[0]);

	N = atoi(argv[1]);
	P = atoi(argv[2]);
	pthread_t threads[P];


	if (N%P != 0)
		fprintf(stderr, "N mod P != 0, not possible\n");

	a = (int*) malloc (N*sizeof(int));	  
	for (long i=0; i<N; i++)
	  a[i] = 1;
	
	res_i = malloc(P*sizeof(long));
	for (long i = 0; i < P; i++)
	  res_i[i] = 0;

	double t1 = (double)clock();

	for(long i=0; i<P; i++) {
	   pthread_create(&threads[i], NULL, prodvec, (void*) i); 
	}

	for(long i=0; i<P; i++)
	  pthread_join(threads[i], &status);

	for (long i = 0; i < P; i++)
	  res += res_i[i];
	

	double t2 = (double)clock();

	printf ("Sum=%li (in %f sec)\n",res, (t2-t1)/CLOCKS_PER_SEC);
	free (a);

	pthread_exit(NULL);
}
