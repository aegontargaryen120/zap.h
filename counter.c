#include <stdio.h>

int main(void) {
  for (int i = 0; i < 11; ++i) {
    if (i != 0) printf("%d\n", i);
    continue;
  }
  
  return 0;
}
