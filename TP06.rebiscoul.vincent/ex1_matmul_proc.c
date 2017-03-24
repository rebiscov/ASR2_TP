#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct matrix matrix;
struct matrix{
  int n;
  int m;
  int *matrix;
};

int mult(matrix matrices[], int a, int b, int c){
  int i, j, k, p = matrices[a].n*matrices[b].m, coeff;
  matrix m;
  char buff[15];
  int **pipes = malloc(sizeof(int*)*p);

  for (i = 0; i < p; i++){
    pipes[i] = malloc(sizeof(int)*2);
    pipe(pipes[i]);
  }
  

  matrices[c] = m;
  matrices[c].matrix = malloc(sizeof(int)*matrices[a].n*matrices[b].m);
  matrices[c].n = matrices[a].n; matrices[c].m = matrices[b].m;
  
  for (i = 0; i < matrices[a].n; i++){
    for (j = 0; j < matrices[b].m; j++){
      matrices[c].matrix[i*matrices[c].n+j] = 0; k = 0;
      coeff = 0;
      if (fork() == 0){ /* Si on est un fils on calcul */
	for (k = 0; k < matrices[a].m; k++){
	  coeff += matrices[a].matrix[i*matrices[a].n+k]*matrices[b].matrix[k*matrices[b].n+j];
	}
	sprintf(buff, "%d", coeff);
	write(pipes[matrices[a].n*i+j][1], buff, 15);

	for (i = 0; i < p; i++)
	  free(pipes[i]);
	free(pipes);
	
	exit(0);
      }
    }
  }

  for (i = 0; i < matrices[a].n; i++){
    for (j = 0; j < matrices[b].m; j++){
      read(pipes[matrices[a].n*i+j][0], buff, 15);
      matrices[c].matrix[i*matrices[c].n+j] = atoi(buff);
    }
  }
  
  for (i = 0; i < p; i++)
    free(pipes[i]);
  free(pipes);
  
  return 0;
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
