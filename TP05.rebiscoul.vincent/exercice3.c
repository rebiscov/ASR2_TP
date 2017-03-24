#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

int pid1, pid2;

void signal_handler(int sig){
  kill(pid1, SIGKILL);
  kill(pid2, SIGKILL);
  kill(getpid(), SIGKILL);
}

int main(int argc, char *argv[]){
  int n = atoi(argv[1]), pid_gagnant = 0, tube1i[2], tube2i[2], tube1o[2], tube2o[2], i, j1, j2;
  char buffer[50];
  struct sigaction sa;
  

  pipe(tube1i);
  pipe(tube1o);
  pid1 = fork();
  if (pid1 == 0){ /* Si on est le fils 1... */
    srand(time(NULL));
    for (i = 0; i < n; i++){
      sprintf(buffer, "%d", rand()%3);
      write(tube1o[1], buffer, 50);
      read(tube1i[0], buffer, 50);      
    }
    
    return 0;
  }

  pipe(tube2i);
  pipe(tube2o);
  pid2 = fork();
  if (pid2 == 0){ /* Si on est le fils 2... */
    srand(time(NULL) + 128028);
    for (i = 0; i < n; i++){
      sprintf(buffer, "%d", rand()%3); /* On joue */
      write(tube2o[1], buffer, 50); /* On envoie à l'arbitre */
      read(tube2i[0], buffer, 50); /* On recupere le pid, je sais pas à quoi ça sert mais bon... */      
    }
    
    return 0;
  }
  
  /* Si on est le père... */
  sa.sa_handler = signal_handler;
  sigaction(SIGINT, &sa, NULL);
  for (i = 0; i < n; i++){
    read(tube1o[0], buffer, 50);
    j1 = atoi(buffer);
    read(tube2o[0], buffer, 50);
    j2 = atoi(buffer);

    if (j1 == j2){
      pid_gagnant = 0;
      printf("Égalité\n");
    }
    else if (j2 == j1 + 1%3){
      printf("Joueur 2 a gagné\n");
      pid_gagnant = pid2;
    }
    else{
      printf("Joueur 1 a gagné\n");
      pid_gagnant = pid1;
    }
    
    sprintf(buffer, "%d", pid_gagnant);
    write(tube1i[1], buffer, 50);
    write(tube2i[1], buffer, 50);
  }
  
  wait(&pid1);
  wait(&pid2);
  
  return 0;
}
