#include <iostream>
using namespace std;

void numberInputer(int numbersArray[]) {
  for (int i = 0; i <= 2; i++) {
    cout << "Enter your Natural number: ";
    int arrayInputNumber;
    cin >> arrayInputNumber;

    numbersArray[i] = arrayInputNumber;
  }
}

int findLargestNumber(int numbersArray[]) {
  cout << "Calculating..." << "\n";

  int largestNumber = numbersArray[0];
  for (int i = 1; i <= 2; i++) {
    if (numbersArray[i] > largestNumber) {
      largestNumber = numbersArray[i];
    }
  }

  return largestNumber;
}

int main() {
  cout << "Enter 3 Natural numbers for an array. The app will return THE "
          "LARGEST one."
       << "\n";
  int array[3];

  numberInputer(array);

  int largestNumber = findLargestNumber(array);

  cout << "THE LARGEST Natural number you typed is: \"" << largestNumber << "\""
       << "\n";

  getchar();
  getchar();

  return 0;
}