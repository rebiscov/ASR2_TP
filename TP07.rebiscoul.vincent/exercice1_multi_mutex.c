#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct element element;
struct element{
  void* value;
  element *next;
};

typedef element* llist;

struct queue{
  llist in;
  llist out;
};

pthread_mutex_t push = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pop = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t blocking = PTHREAD_MUTEX_INITIALIZER;
pthread_t id1;

struct queue* queue_new(void){
  struct queue* new_queue = NULL;
  new_queue = malloc(sizeof(struct queue));

  new_queue->in = NULL;
  new_queue->out = NULL;
  
  return new_queue;
}


void queue_free(struct queue *queue){
  llist temp;

  while (queue->in != NULL){
    temp = queue->in->next;
    free(queue->in);
    queue->in = temp;
  }
  while (queue->out != NULL){
    temp = queue->out->next;
    free(queue->out);
    queue->out = temp;
  }

  free(queue);
}

void queue_push(struct queue *q, void *value){
  element *new_element = malloc(sizeof(element));

  pthread_mutex_lock(&push);
  
  new_element->value = value;
  new_element->next = q->in;
  q->in = new_element;

  pthread_mutex_unlock(&push);
  pthread_mutex_unlock(&blocking);
}

void* queue_pop_blocking(struct queue *q){
  void* ret;
  llist temp;

  pthread_mutex_lock(&pop);
  
  if (q->out == NULL){
    if (q->in == NULL){
      pthread_mutex_lock(&blocking);
    }

    pthread_mutex_lock(&push);
    while (q->in != NULL){
      temp = q->in;
      q->in = q->in->next;
      if (q->out == NULL){
	q->out = temp;
	q->out->next = NULL;
      }
      else{
	temp->next = q->out;
	q->out = temp;
      }
    }
    q->in = NULL;
    pthread_mutex_unlock(&push);
  }

  ret = q->out->value;
  temp = q->out->next;
  free(q->out);
  q->out = temp;

  if (q->out == NULL && q->in == NULL)
    pthread_mutex_lock(&blocking);

  pthread_mutex_unlock(&pop);
  return ret;
}

void* queue_pop(struct queue *q){
  void* ret;
  llist temp;

  pthread_mutex_lock(&pop);
  
  if (q->out == NULL){
    pthread_mutex_lock(&push);
    
    if (q->in == NULL)
      return NULL;
    
    while (q->in != NULL){
      temp = q->in;
      q->in = q->in->next;
      if (q->out == NULL){
	q->out = temp;
	q->out->next = NULL;
      }
      else{
	temp->next = q->out;
	q->out = temp;
      }
    }
    pthread_mutex_unlock(&push);
  }

  ret = q->out->value;
  temp = q->out->next;
  free(q->out);
  q->out = temp;

  pthread_mutex_unlock(&pop);
  return ret;
}


void test_q1(void){
  struct queue *q = NULL;
  int a = 2, b = 3, c = 1, d = 6;

  q = queue_new();
  
  queue_push(q, (void*)&a);  
  queue_push(q, (void*)&b);
  queue_push(q, (void*)&c);

  printf("%d\n", (*(int*)queue_pop(q)));
  printf("%d\n", (*(int*)queue_pop(q)));
  queue_push(q, (void*)&d);
  printf("%d\n", (*(int*)queue_pop(q)));
  printf("%d\n", (*(int*)queue_pop(q)));
  
  queue_free(q);
}

void* thread1(void *arg){
  int i;
  struct queue *q = (struct queue*)arg;

  for (i = 0; i < 10000; i++){
    queue_push(q, (void*)i);
  }

  return NULL;
}

void test_q2(void){
  struct queue *q = NULL;
  int i;

  pthread_mutex_lock(&blocking);
  q = queue_new();

  pthread_create(&id1, NULL, thread1, (void*)q);

  for (i = 0; i < 10000; i++){
    printf("%d\n", (int)queue_pop_blocking(q));
  }

  pthread_join(id1, NULL);
  pthread_mutex_unlock(&blocking);

  queue_free(q);
}

int main(void){
  test_q1();
  test_q2();
}
