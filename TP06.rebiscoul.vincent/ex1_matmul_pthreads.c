#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

typedef struct matrix{
  int n;
  int m;
  int *matrix;
} matrix;

typedef struct element{
  matrix* a;
  matrix* b;
  int i; int j;
} element;

void* compute(void* in){
  int res = 0, l, n1 = ((element*)in)->a->n, n2 = ((element*)in)->b->n, *ret;
  int k = ((element*)in)->a->m;

  for (l = 0; l < k; l++){
    res += ((element*)in)->a->matrix[n1*((element*)in)->i+l]*((element*)in)->b->matrix[n2*l+((element*)in)->j];
  }

  ret = malloc(sizeof(int));
  *ret = res;
  
  return (void*)ret;
}

void mult(matrix matrices[], int a, int b, int c){
  int i, j, **retvalue = NULL;
  matrix m;
  element in;
  pthread_t* ids = malloc(sizeof(pthread_t)*matrices[a].n*matrices[b].m);  

  in.a = &matrices[a]; in.b = &matrices[b];
  
  matrices[c] = m;
  matrices[c].matrix = malloc(sizeof(int)*matrices[a].n*matrices[b].m);
  matrices[c].n = matrices[a].n; matrices[c].m = matrices[b].m;
  
  for (i = 0; i < matrices[c].n; i++){
    for (j = 0; j < matrices[c].m; j++){
      matrices[c].matrix[i*matrices[c].n+j] = 0;
      in.i = i; in.j = j;
      pthread_create(&ids[i*matrices[c].n+j], NULL, compute, &in);
    }
  }
  
  for (i = 0; i < matrices[c].n; i++){
    for (j = 0; j < matrices[c].m; j++){
      pthread_join(ids[i*matrices[a].n+j], (void*)&retvalue);
      matrices[c].matrix[i*matrices[c].n+j] = *((int*)retvalue);
    }
  }
}

void affiche(matrix matrices[], int i){
  int j, k;

  for (j = 0; j < matrices[i].n; j++){
    for (k = 0; k < matrices[i].m; k++){
      printf("%d ", matrices[i].matrix[j*matrices[i].n+k]);
    }
    printf("\n");
  }
}

int main (void){
  int i, j, nb_m;
  matrix* matrices = NULL;

  scanf("%d", &nb_m);
  matrices = malloc(sizeof(matrix)*nb_m+1);

  for (i = 0; i < nb_m; i++){
    scanf("%d %d", &matrices[i].n, &matrices[i].m);
    matrices[i].matrix = malloc(sizeof(int)*matrices[i].n*matrices[i].m);
    for (j = 0; j < matrices[i].n*matrices[i].m; j++){
      scanf("%d", &matrices[i].matrix[j]);
    }
  }

  mult(matrices, 0, 1, 2);
  affiche(matrices, 2);
  
  
  return 0;
}
