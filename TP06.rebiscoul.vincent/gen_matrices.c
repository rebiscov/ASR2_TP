#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void){
  int k, n, i, j, l;
  srand(time(NULL));

  scanf("%d %d", &n, &k);

  for (l = 0; l < 2; l++){
    printf("%d %d\n", n, n);
    for (i = 0; i < n; i++){
      for (j = 0; j < n; j++){
	printf("%d ", (rand()%(2*k+1)-k));
      }
      printf("\n");
    }
    printf("\n");
  }
  
  return 0;
}
