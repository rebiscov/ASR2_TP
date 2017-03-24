#include <stdlib.h>
#include <stdio.h>

struct block {
  struct block *prev_block;
  uint8_t prev_hash[HASH_SIZE];
  long int nonce;
  int * tx;
};

struct chain{
  struct block *start;
};

void sha512(char *buf, size_t buf_size, uint8_t *result[64]){
  
}

int main(void){
  
  
  return 0;
}
