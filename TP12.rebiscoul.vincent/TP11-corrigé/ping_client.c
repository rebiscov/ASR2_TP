#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define USERNAME_SIZE 8
#define MAX_MSG_SIZE 1024

int create_socket(const char *hostname, const char *port) {
  const int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

  struct addrinfo hints = {
    .ai_flags=0,
    .ai_family=AF_INET,
    .ai_socktype=SOCK_STREAM,
    .ai_protocol=0,
    .ai_addrlen=0,
    .ai_addr=NULL,
    .ai_canonname=NULL,
    .ai_next=NULL,
  };
  struct addrinfo *addrinfo, *addrinfo_it;
  if (getaddrinfo(hostname, port, &hints, &addrinfo) != 0) {
    perror("getaddrinfo()");
    exit(EXIT_FAILURE);
  }

  for (addrinfo_it = addrinfo; addrinfo_it != NULL; addrinfo_it = addrinfo_it->ai_next) {
    if (connect(sock, addrinfo_it->ai_addr, addrinfo_it->ai_addrlen) != -1) {
      break;
    }
  }
  freeaddrinfo(addrinfo);


  if (addrinfo_it == NULL) {
    perror("Could not connect()");
    exit(EXIT_FAILURE);
  }

  return sock;
}

bool check_send(int conn, const void *buf, size_t buf_size) {
  const char *char_buf = buf;
  while (buf_size) {
    ssize_t ret = send(conn, char_buf, buf_size, MSG_NOSIGNAL);
    if (ret == -1) {
      return false;
    }
    assert(ret >= 0);
    assert((size_t)ret >= buf_size);
    buf_size = (size_t)(buf_size - (size_t)ret);
    char_buf += ret;
  }
  return true;
}

bool send_clienthello(const int conn, const char *username) {
  uint32_t version_buf = htonl(1);
  if (!check_send(conn, (char*) &version_buf, 4)) {
    return false;
  }

  char *username_buf = calloc(1, USERNAME_SIZE+1);
  strcpy(username_buf, username);
  if (!check_send(conn, username_buf, USERNAME_SIZE)) {
    free(username_buf);
    return false;
  }
  free(username_buf);
  return true;
}

bool read_serverhello(const int conn) {
  char error_code_buf[4];
  recv(conn, error_code_buf, 4, 0);
  uint32_t error_code = ntohl(*((uint32_t*) error_code_buf));

  char msg_size_buf[4];
  if (recv(conn, msg_size_buf, 4, 0) == -1) {
    perror("recv()");
    return false;
  }
  uint32_t msg_size = ntohl(*((uint32_t*) msg_size_buf));


  if (msg_size >= MAX_MSG_SIZE) {
    printf("Dropping SERVERHELLO bigger than %u bytes (%u).\n", MAX_MSG_SIZE, msg_size);
    return false;
  }

  char *msg = calloc(1, msg_size);
  if (recv(conn, msg, msg_size, 0) == -1) {
    perror("recv()");
    return false;
  }

  if (error_code == 0) {
    printf("Successful connection. Welcome message: “%s”\n", msg);
    free(msg);
  }
  else {
    printf("Error 0x%x: %s\n", error_code, msg);
    free(msg);
    return false;
  }

  return true;
}

bool send_clientmsg(const int conn, const char *msg) {
  uint32_t msg_size = strlen(msg);
  uint32_t msg_size_buf = htonl(msg_size);
  return check_send(conn, (char*) &msg_size_buf, 4) &&
    check_send(conn, msg, msg_size);
}

bool read_servermsg(const int conn) {
  char username[USERNAME_SIZE+1];
  username[USERNAME_SIZE] = '\0';
  if (recv(conn, &username, USERNAME_SIZE, 0) == -1) {
    perror("recv()");
    return false;
  }

  char msg_size_buf[4];
  if (recv(conn, msg_size_buf, 4, 0) == -1) {
    perror("recv()");
    return false;
  }
  uint32_t msg_size = ntohl(*((uint32_t*) msg_size_buf));

  if (msg_size >= MAX_MSG_SIZE) {
    printf("Dropping SERVERMSG bigger than %u bytes (%u).\n", MAX_MSG_SIZE, msg_size);
    return false;
  }

  char *msg = calloc(1, msg_size+1);
  ssize_t ret = recv(conn, msg, msg_size, 0);
  if (ret == -1) {
    perror("recv()");
    return false;
  }
  else if (ret == 0) {
    /* Connection closed. */
    return false;
  }
  msg[msg_size] = '\0';

  printf("<%s> %s\n", username, msg);

  free(msg);

  return true;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("Syntax: %s <hostname> <port> <username>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  if (strlen(argv[3]) > USERNAME_SIZE) {
    printf("Username cannot be longer than %d characters.\n", USERNAME_SIZE);
    exit(EXIT_FAILURE);
  }

  const int sock = create_socket(argv[1], argv[2]);

  if (!send_clienthello(sock, argv[3])) {
    perror("client hello");
    return EXIT_FAILURE;
  }

  if (!read_serverhello(sock)) {
    return EXIT_FAILURE;
  }

  if (!send_clientmsg(sock, "Hi everyone.")) {
    perror("client msg");
    return EXIT_FAILURE;
  }
  if (!send_clientmsg(sock, "I'm here.")) {
    perror("client msg");
    return EXIT_FAILURE;
  }

  while (1) {
    char buf[4096];
    recv(sock, buf, 4096, MSG_DONTWAIT);
    if (!send_clientmsg(sock, "ping.")) {
      perror("client msg");
      return EXIT_FAILURE;
    }
    sleep(10);
  }

  return EXIT_SUCCESS;
}

