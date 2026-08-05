#include <iostream>
using namespace std;

int main() {

  bool number_is_found = false;

  for (int a = 1; a <= 9; a++) {
    for (int b = 1; b <= 9; b++) {
      for (int c = 1; c <= 9; c++) {
        if (number_is_found) {
          return 0;
        }
        if ((a * 100 + b * 10 + b) + (c * 100 + a * 10 + b) ==
            b * 100 + a + 10 + c) {
          number_is_found = true;
          cout << "A: " << a << ", B: " << b << ", C: " << c << "\n"
               << "Equality 'abb + cab = bac': " << a << b << b << " + " << c
               << a << b << " = " << b << a << c << "\n"
               << "Equality, but more detailed: " << a * 100 << " + " << b * 10
               << " + " << b << " + " << c * 100 << " + " << a * 10 << " + "
               << b << " == " << b * 100 << " + " << a * 10 << " + " << c
               << "\n";
        }
      }
    }
  }

  return 0;
}