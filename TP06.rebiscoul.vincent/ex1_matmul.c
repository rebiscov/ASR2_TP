#include <stdlib.h>
#include <stdio.h>

typedef struct matrix matrix;
struct matrix{
  int n;
  int m;
  int *matrix;
};

void mult(matrix matrices[], int a, int b, int c){
  int i, j, k;
  matrix m;

  matrices[c] = m;
  matrices[c].matrix = malloc(sizeof(int)*matrices[a].n*matrices[b].m);
  matrices[c].n = matrices[a].n; matrices[c].m = matrices[b].m;
  
  for (i = 0; i < matrices[a].n; i++){
    for (j = 0; j < matrices[b].m; j++){
      matrices[c].matrix[i*matrices[c].n+j] = 0;
      for (k = 0; k < matrices[a].m; k++){
	matrices[c].matrix[i*matrices[c].n+j] += matrices[a].matrix[i*matrices[a].n+k]*matrices[b].matrix[k*matrices[b].n+j];
      }
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
  int i, j, k, nb_m;
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
