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

void* launch_thread(void *arg){ /* Lance la multiplication en lançant un thread*/
  args arguments = *(args*) arg;
  matrix M1 = arguments.P;
  matrix M2 = arguments.Q;
  int *R, k, off;
  R = calloc(arguments.steps, sizeof(int));

  off = 0;

  for  (k = arguments.k * arguments.steps; k < (arguments.k + 1)*arguments.steps; ++k) {
    singlemul(R, M1, M2, k / M2->m, k % M2->m, off);
    ++off;
  }
  return R;
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

  matrix *M;
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
  pthread_t* st;
  st = calloc(nthreads, sizeof(pthread_t));
  args* arguments;
  arguments = calloc(nthreads, sizeof(struct args));
  int **retour;
  retour = calloc(nthreads + 1, sizeof(int*));
  for(i = 0; i < nthreads; ++i) {
    arguments[i].P = M[0];
    arguments[i].Q = M[1];
    arguments[i].k = i;
    arguments[i].steps = m*n/nthreads;
  }
  int remain = (m*n)%nthreads;
  int off;
  retour[nthreads] = calloc(remain, sizeof(retour));


  off = 0;
#pragma omp parallel for num_threads(4)
  for (i = arguments[0].steps * nthreads; i < m*n; ++i){
    singlemul(retour[nthreads], M[0], M[1], i / m, i % m, off);
    ++off;
  }

  for(i = 0; i < nthreads; ++i) {
    pthread_create(st+i, NULL, &launch_thread, (void**) &arguments[i]);
  }
  for(i = 0; i < nthreads; ++i)
    pthread_join(st[i], (void**) &retour[i]);
  R = calloc(n*m, sizeof(int));
  for(i = 0; i < nthreads; ++i) {
    for(j = 0; j < arguments[0].steps; ++j) {
      R[i*arguments[0].steps + j] = ((int*) retour[i])[j];
    }
  }
  for(j = 0; j < remain; ++j) {
    R[nthreads*arguments[0].steps + j] = ((int*) retour[nthreads])[j];
  }
  matrix_print2(R, n, m);
  for(i = 0; i < nthreads; ++i) {
    free(retour[i]);
  }
  free(retour[nthreads]);
  free(retour);
  for(i = 0; i < nmatrices; ++i)
    matrix_free(M[i]);
  free(M);
  free(R);
  free(st);
  free(arguments);

  return 0;
}


