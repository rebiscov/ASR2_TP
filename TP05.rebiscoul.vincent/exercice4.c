#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

void derive(char w[], char output[]){
  int i = 1, count = 1;
  char c = w[i], temp[50];
  
  while (w[i] != '\0'){
    if (c != w[i]){
      sprintf(temp, "%d", count);
      strcat(output, temp);
      count = 0;
      c = w[i];
    }
    i++;
    count++;
  }
  
  sprintf(temp, "%d", count);
  strcat(output, temp);  
}

int main(int argc, char *argv[]){
  char output[50] = "";
  derive(argv[1], output);

  printf("%s\n", output);
  
  return 0;
}
