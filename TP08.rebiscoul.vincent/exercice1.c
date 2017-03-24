#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct retrait retrait;
struct retrait{
  int thune;
  char* nom;
};


int solde, semaine = 1;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t salaire_id1, salaire_id2, carte_id1, carte_id2;

void *salaire(void *salaire){
  pthread_mutex_lock(&mutex);  
  solde += *(int*)salaire;
  pthread_mutex_unlock(&mutex);
}


void *carte(void *ret){
  int debit = ((retrait*)ret)->thune;
  char* debiteur = ((retrait*)ret)->nom;

  pthread_mutex_lock(&mutex);  
  if (solde - debit < 0)
    printf("Semaine %d - %s: Retrait de %d € n'a pas pu être effectué. Il reste %d €\n", semaine, debiteur, debit, solde);
  else
    solde -= debit;
  
  pthread_mutex_unlock(&mutex);
}

int main(int argc, char *argv[]){
  int s1 = 2000, s2 = 3000;
  retrait r1, r2;

  r1.nom = malloc(20*sizeof(char)); r2.nom = malloc(20*sizeof(char));
  r1.nom = "Ben"; r2.nom = "Cassiopée";

  
  srand(time(NULL));
  
  if (argc != 2)
    solde = 10000;
  else
    solde = atoi(argv[1]);

  while (1){
    r1.thune = rand()%20000;
    r2.thune = rand()%10000;
    pthread_create(&salaire_id1, NULL, salaire, &s1);
    pthread_create(&salaire_id2, NULL, salaire, &s2);
    pthread_create(&carte_id1, NULL, carte, &r1);
    pthread_create(&carte_id2, NULL, carte, &r2);
    printf("Semaine %d - Solde: %d\n", semaine, solde);
    
    semaine++;
    sleep(1);
  }

  return 0;
}
