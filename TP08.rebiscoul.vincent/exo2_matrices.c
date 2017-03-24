#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

typedef struct matrix {
  int n, m;
  int* mat;
}* matrix;

typedef struct args {
  matrix P, Q;
  int k;
  int steps;
} args;

void matrix_init(matrix M, const int n, const int m){ /* On initialise la matrice */
  M->n = n;
  M->m = m;
  M->mat = calloc(n*m, sizeof(int));
}

void matrix_free(matrix M){ /* On libère la matrice */
  free(M->mat);
  free(M);
}

void matrix_print2(const int* M, const int n, const int m) { /* On affiche la matrice */
  int i,j;
  for (i = 0; i < n; ++i){
    for(j = 0; j < m; ++j) {
      printf("%4d ", M[i * m + j]);
    }
    putchar('\n');
  }
}

void matrix_print(const matrix M){ /* On affiche aussi la matrice */
  int i,j;
  for (i = 0; i < M->n; ++i){
    for(j = 0; j < M->m; ++j) {
      printf("%4d ", M->mat[i * M->m + j]);
    }
    putchar('\n');
  }
}

void singlemul(int* r, const matrix m1, const matrix m2, const int i, const int j, const int off){ /* Calcul un coefficient d'une matrice */
  if (m1->m != m2->n) {
    perror("Taille des matrices incompatible\n");
    exit(1);
  }
  int k;
  r[off] = 0;
  for(k = 0; k < m1->m; ++k) {
    r[off] += m1->mat[i * m1->m + k]*m2->mat[k * m2->m + j];
  }
}


void matmul(matrix r, matrix m1, matrix m2){ /* Fais une multiplication de matrices */
  if (m1->m != m2->n || r->n != m1->n || r->m != m2->m) {
    perror("Taille des matrices incompatible\n");
    exit(1);
  }
  int i,j,k;

  for(i = 0; i < m1->n; ++i) {
    for(j = 0; j < m2->m; ++j) {
      r->mat[i * r->n + j] = 0;      
    }
  }
  
#pragma omp parallel for collapse(3)
  for(i = 0; i < m1->n; ++i) {
    for(j = 0; j < m2->m; ++j) {
      for(k = 0; k < m1->m; ++k) {
        r->mat[i * r->m + j] += m1->mat[i * m1->m + k]*m2->mat[k * m2->m + j];
      }
    }
  }
}

int main (int argc, char **argv){
  if (argc < 3) {
    fprintf(stderr, "Usage: %s nthreads fichier\n", *argv);
    return 1;
  }

  matrix *M, out;
  int *R;
  int nthreads;
  nthreads = atoi(argv[1]);

  FILE* fichier;
  if ((fichier = fopen(argv[2], "r")) == NULL) {
    perror("Le nom de fichier est incorrect");
    return 1;
  }

  int nmatrices;
  int i,j,c;
  fscanf(fichier, "%d", &nmatrices); 
  M = calloc(nmatrices, sizeof(struct matrix));
  int n, m;
  for(c = 0; c < nmatrices; ++c) {
    M[c] = calloc(1, sizeof(struct matrix));
    fscanf(fichier, "%d", &n); 
    fscanf(fichier, "%d", &m); 
    matrix_init(M[c], n, m);
    for(i = 0; i < n; ++i) {
      for(j = 0; j < m; ++j) {
        fscanf(fichier, "%d", M[c]->mat + i * m + j);
      }
    }
  }
  fclose(fichier);

  n = M[0]->n;
  m = M[1]->m;

  out = calloc(1, sizeof(struct matrix));
  matrix_init(out, n, m);

  matmul(out, M[0], M[1]);

  matrix_print(out);

  free(M);

  return 0;
}


