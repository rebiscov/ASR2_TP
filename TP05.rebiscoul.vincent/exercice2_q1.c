#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>


int main(int argc, char *argv[]){
  int i, **tubes, n = atoi(argv[1]);
  pid_t pid, candidat, *pids = malloc(n*sizeof(n));
  char string[50];

  tubes = malloc((n+1)*sizeof(int*));
  for (i = 0; i <= n; i++){ /* On créé les tubes */
    tubes[i] = malloc(2*sizeof(int));
    pipe(tubes[i]);
  }
  
  for (i = 0; i < n; i++){
    pid = fork();
    if (pid == 0)
      break;
    else
      pids[i] = pid; /* Le père se rappelle du pid des fils, c'est pour attendre leurs morts plus tard */
  }

  if (pid == 0){ /* si on est un fils... */
    read(tubes[i][0], string, 50);
    candidat = atoi(string);

    pid = getpid();
    if (pid > candidat)
      candidat = pid;
    sprintf(string, "%d", candidat);
    write(tubes[i+1][1], string, 50);

    return 0;
  }
      
  else{ /* si on est le père */
    sprintf(string, "%d", getpid());
    write(tubes[0][1], string, 50);

    sleep(2);
    read(tubes[n][0], string, 50);
    printf("Gagnant :%s\n", string);
  }

  for(i = 0; i < n; i++) /* On attend la mort des fils */
    wait(&pids[i]);

  for(i = 0; i <= n; i++)
    free(tubes[i]);
  free(pids);

  return 0;
}
