#include <stdio.h>

int check_sign(int num) {
  if (num < 0) {
    printf("Error. Number must be positive :(.\n");
    return -1;
  } else {
    return 0;
  }
}

int main() {
  int z1, z2, z3;

  printf("Enter the z1 please: ");
  if (scanf("%d", &z1) != 1) {
    printf("Error. Invalid input. Numbers are required...\n");
    return 1;
  }
  if (check_sign(z1) != 0) {
    return 1;
  }

  printf("Enter the z2 please: ");
  if (scanf("%d", &z2) != 1) {
    printf("Error. Invalid input. Numbers are required...\n");
    return 1;
  }
  if (check_sign(z2) != 0) {
    return 1;
  }

  printf("Enter the z3 please: ");
  if (scanf("%d", &z3) != 1) {
    printf("Error. Invalid input. Numbers are required...\n");
    return 1;
  }
  if (check_sign(z3) != 0) {
    return 1;
  }

  int max_value = z1 > z2 ? z1 : z2;
  max_value = max_value > z3 ? max_value : z3;

  int min_value = z1 < z2 ? z1 : z2;
  min_value = min_value < z3 ? min_value : z3;

  int mid_value = z1 + z2 + z3 - max_value - min_value;

  printf("Max: %d\n", max_value);
  printf("Mid: %d\n", mid_value);
  printf("Min: %d\n", min_value);

  getchar();
  getchar();

  return 0;
}