#include <iostream>
using namespace std;

int main() {
  int sum = 0;
  for (int i = 1; i <= 10; i++) {
    cout << i << "\n";
    sum += i;
  }
  cout << sum << "\n";

  return 0;
}