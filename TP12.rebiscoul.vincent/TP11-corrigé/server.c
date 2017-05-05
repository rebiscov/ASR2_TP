#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define USERNAME_SIZE 8
#define MAX_USERS 1000
#define MAX_MSG_SIZE 1024

int create_socket() {
  const int sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1) {
    perror("socket()");
    exit(EXIT_FAILURE);
  }

#ifdef SO_REUSEADDR
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
#endif
#ifdef SO_REUSEPORT
  setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, NULL, 0);
#endif

  const struct in_addr sin_addr = { .s_addr = 0x00000000 }; /* bind 0.0.0.0, ie. all interfaces */
  const struct sockaddr_in addr = { .sin_family=AF_INET, .sin_port=htons(1337), .sin_addr=sin_addr };
  if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
    perror("bind()");
    exit(EXIT_FAILURE);
  }

  if(listen(sock, 10) == -1) {
    perror("listen()");
    exit(EXIT_FAILURE);
  }

  return sock;
}

/* Sends the buffer entirely. Returns false if the socket is writable. */
bool check_send(int conn, const void *buf, size_t buf_size) {
  const char *char_buf = buf;
  while (buf_size) {
    ssize_t ret = send(conn, char_buf, buf_size, MSG_NOSIGNAL);
    if (ret == -1) {
      return false;
    }
    assert(ret >= 0);
    assert((size_t)ret <= buf_size);
    buf_size = (size_t)(buf_size - (size_t)ret);
    char_buf += ret;
  }
  return true;
}

/* Returns false iff the socket is writable. */
bool send_serverhello(int conn, uint32_t error_code, const char *msg) {
  uint32_t error_code_buf = htonl(error_code);
  size_t msg_size = strlen(msg)+1;
  uint32_t msg_size_buf = htonl(msg_size);
  return check_send(conn, &error_code_buf, 4) && 
    check_send(conn, &msg_size_buf, 4) &&
    check_send(conn, msg, msg_size);
}

size_t read_clientmsg(int conn, char **msg) {
  char msg_size_buf[4];
  if (recv(conn, msg_size_buf, 4, 0) == -1) {
    perror("recv()");
    return 0;
  }
  uint32_t msg_size = ntohl(*((uint32_t*) msg_size_buf));

  if (msg_size >= MAX_MSG_SIZE) {
    printf("Dropping CLIENTMSG bigger than %u bytes (%u).\n", MAX_MSG_SIZE, msg_size);
    return 0;
  }

  *msg = calloc(1, msg_size+1);
  if (recv(conn, *msg, msg_size, 0) == -1) {
    perror("recv()");
    return 0;
  }
  (*msg)[msg_size] = '\0';

  return msg_size;
}

void broadcast_servermsg(const char *username, uint32_t msg_size, const char *msg,
			 char **usernames, int *connections,
			 pthread_mutex_t *write_mutex, pthread_mutex_t *user_list_mutex) {
  uint32_t msg_size_buf = htonl(msg_size);
  pthread_mutex_lock(user_list_mutex);
  pthread_mutex_lock(write_mutex);
  unsigned int i;
  for (i=0; i<MAX_USERS; i++) {
    if (connections[i] != 0) {
      bool writable = check_send(connections[i], username, USERNAME_SIZE) &&
	check_send(connections[i], (char*) &msg_size_buf, 4) &&
	check_send(connections[i], msg, msg_size);
      if (!writable) {
	/* If we can't write anymore, remove the socket. */
	usernames[i] = NULL;
	connections[i] = 0;
      }
    }
  }
  pthread_mutex_unlock(write_mutex);
  pthread_mutex_unlock(user_list_mutex);
}

void handle_connection(int conn, struct sockaddr *addr, socklen_t *addrlen,
		       char **usernames, int *connections,
		       pthread_mutex_t *write_mutex, pthread_mutex_t *user_list_mutex) {
  (void) addr;
  (void) addrlen;
  char version_buf[4];
  if (recv(conn, version_buf, 4, 0) == -1) {
    perror("recv()");
    return;
  }
  uint32_t version = ntohl(*((uint32_t*) version_buf));
  printf("Client version: %d\n", version);

  if (version != 1) {
    send_serverhello(conn, 1, "Sorry, this server only supports protocol version 1.\n");
    return;
  }

  char *username = calloc(1, USERNAME_SIZE+1); /* \0 at the end. */
  if (recv(conn, username, USERNAME_SIZE, 0) == -1) {
    perror("recv()");
    return;
  }
  printf("New connection from: %s\n", username);

  unsigned int i;
  pthread_mutex_lock(user_list_mutex);
  for (i=0; i<MAX_USERS; i++) {
    if (usernames[i] != NULL && memcmp(usernames[i], username, 8) == 0) {
      send_serverhello(conn, 3, "This username is already in use.\n");
      pthread_mutex_unlock(user_list_mutex);
      return;
    }
  }
  for (i=0; i<MAX_USERS; i++) {
    if (usernames[i] == NULL) {
      assert(connections[i] == 0);
      if (!send_serverhello(conn, 0, "Welcome on this server!\n")) {
	pthread_mutex_unlock(user_list_mutex);
	return; /* Socket closed */
      }
      usernames[i] = username;
      connections[i] = conn;
      break;
    }
  }
  pthread_mutex_unlock(user_list_mutex);
  if (i == MAX_USERS) {
    printf("Cannot accept connection from %s, too many users.\n", username);
    send_serverhello(conn, 2, "Sorry, too many users are already connected.\n");
    return;
  }

  while (1) {
    char *msg;
    size_t msg_size = read_clientmsg(conn, &msg);
    if (msg_size == 0) {
      pthread_mutex_lock(user_list_mutex);
      for (i=0; i<MAX_USERS; i++) {
	if (usernames[i] != NULL && memcmp(usernames[i], username, 8) == 0) {
	  assert(connections[i] != 0);
	  usernames[i] = NULL;
	  connections[i] = 0;
	}
      }
      pthread_mutex_unlock(user_list_mutex);
      return;
    }
    printf("<%s> %s\n", username, msg);
    broadcast_servermsg(username, msg_size, msg, usernames, connections, write_mutex, user_list_mutex);
    free(msg);
  }
}

struct thread_args {
  int conn;
  struct sockaddr *addr;
  socklen_t *addrlen;
  char **usernames;
  int *connections;
  pthread_mutex_t *write_mutex;
  pthread_mutex_t *user_list_mutex;
};

void* handle_connection_thread(void *args_) {
  struct thread_args *args = args_;
  handle_connection(args->conn, args->addr, args->addrlen, args->usernames, args->connections, args->write_mutex, args->user_list_mutex);
  free(args_);
  return NULL;
}

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  const int sock = create_socket();

  char *usernames[MAX_USERS];
  int connections[MAX_USERS];
  memset(usernames, 0, MAX_USERS*sizeof(char*));
  memset(connections, 0, MAX_USERS*sizeof(int));

  pthread_mutex_t write_mutex, user_list_mutex;
  pthread_mutex_init(&write_mutex, 0);
  pthread_mutex_init(&user_list_mutex, 0);

  struct thread_args default_args = {
    .conn=0, .addr=NULL, .addrlen=0,
    .usernames=usernames, .connections=connections,
    .write_mutex=&write_mutex, .user_list_mutex=&user_list_mutex,
  };

  while (1) {
    struct sockaddr *conn_addr = calloc(1, sizeof(struct sockaddr));
    socklen_t *addrlen = calloc(1, sizeof(socklen_t));
    const int conn = accept(sock, conn_addr, addrlen);
    if (conn == -1) {
      perror("accept()");
      exit(EXIT_FAILURE);
    }
    struct thread_args *args = calloc(1, sizeof(struct thread_args));
    assert(args);
    memcpy(args, &default_args, sizeof(struct thread_args));
    args->conn = conn; args->addr=conn_addr; args->addrlen=addrlen;
    pthread_t thread;
    pthread_create(&thread, NULL, handle_connection_thread, args);
  }


  return EXIT_SUCCESS;
}
