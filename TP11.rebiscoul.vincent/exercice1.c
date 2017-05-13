#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>

void get_ip(const char *domain, const char *port, struct addrinfo **res){
  struct addrinfo hints;
  int err;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;
  hints.ai_protocol = 0;

  err = getaddrinfo(domain, port, &hints, res);
  
  if (err != 0){
    printf("getaddrinfo: %s\n", gai_strerror(err));
    exit(1);
  }
}

int main(int argc, char *argv[]){
  struct addrinfo *res = NULL, *rp;
  char hostname[NI_MAXHOST];

  if (argc < 3){
    printf("Use: exercice1 domain port\n");
    return 0;
  }

  get_ip(argv[1], argv[2], &res);
  
  if (res == NULL)
    printf("Nothing was sent back by the host\n");

  for (rp = res; rp != NULL; rp = rp->ai_next){
    int error = getnameinfo(rp->ai_addr, rp->ai_addrlen, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
    
    if (error != 0) {
      fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(error));
      continue;
    }
    if (hostname[0] != '\0')
      printf("hostname: %s\n", hostname);
  }
  
  printf("using hostname: %s\n", hostname);
  
  freeaddrinfo(res);

  return 0;
}
