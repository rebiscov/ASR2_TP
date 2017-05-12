#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>

int sent[1073741824];
int cont = 1;

struct send_infos{
  int sock;
  int argc;
  char **argv;
};

void get_host(const char infos[], char host[], char port[]){
  unsigned int i;
  for (i = 0; infos[i] != ':'; i++);
  host[i] = '\0';

  for (unsigned int j = 0; j < i; j++)
    host[j] = infos[j];
  for (unsigned int j = i + 1; infos[j] != '\0'; j++){
    port[j-i-1] = infos[j];
    port[j-i] = '\0';
  }
}

void send_all(int argc, char **argv, char *message, int sock){
  char port[20], host[20];
  for (unsigned int i = 3; i < argc; i++){
    get_host(argv[i], host, port);
    
    struct in_addr sin_addr_rc;
    inet_pton(AF_INET, host, &(sin_addr_rc.s_addr));
    
    struct sockaddr_in addr_rc = {
      .sin_family = AF_INET,
      .sin_port = htons(atoi(port)),
      .sin_addr = sin_addr_rc
    };

    if (sendto(sock, message, 8, 0, (const struct sockaddr *) &addr_rc, sizeof(addr_rc)) == -1)
      printf("couldn't send message to %s\n", argv[i]);
  }
}

void* write_to(void *in){
  char message[250];
  int len;
  struct send_infos* s = (struct send_infos*)in;
  
  while(cont){
    getline(&message, 250, stdin);
    send_all(s->argc, s->argv, message, s->sock);
  }
  
  return NULL;
}

int main(int argc, char **argv){
  char message[250], src[20];
  memset(sent, 0, sizeof(int));
  
  if (argc < 3){
    printf("use: username port dest1 dest2...\n");
    exit(1);
  }
  srand(time(NULL));

  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock == -1){
    perror("socket");
    exit(1);
  }
    
  struct in_addr sin_addr = {
    .s_addr = INADDR_ANY
  };
  
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(atoi(argv[2])),
    .sin_addr = sin_addr
  };
  
  bind(sock, (const struct sockaddr *) &addr, sizeof(addr));

  sprintf(message, "%d Bonjour de la part de %s\n", rand()%1073741824, argv[1]);
  
  send_all(argc, argv, message, sock);
  send_all(argc, argv, "TEST\n", sock);

  struct sockaddr_in addr_sd;
  socklen_t len;

  pthread_t id;
  
  struct send_infos s = {
    .argc = argc,
    .argv = argv,
    .sock = sock
  };

  pthread_create(&id, NULL, write_to, &s);
  
  while(cont){
    recvfrom(sock, message, 250, 0,(struct sockaddr*) &addr_sd, &len);
    inet_ntop(AF_INET, &(addr_sd.sin_addr), src, INET_ADDRSTRLEN);
    printf("%s:%d %s", src, ntohs(addr_sd.sin_port), message);
  }
  
  return 0;
}
