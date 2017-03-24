#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
  int tube[2], n;
  char output[20];

  pipe(tube);
  
  n = strlen(argv[1]);
  
  if (fork() == 0){
    write(tube[1], argv[1], n+1);

    return 0;
  }

  read(tube[0], output, 20);
  printf("%s\n", output);

  wait(NULL);

  return 0;
}
