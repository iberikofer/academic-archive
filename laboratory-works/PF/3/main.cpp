#include <iostream>
using namespace std;

void arraySizeInputer(int &arraySizeLength) {
  for (int i = 0;; i++) {
    cin >> arraySizeLength;
    if (arraySizeLength <= 1) {
      cout << "The Size must be minimun 2 integers!" << "\n"
           << "Please, enter the number of array entities: ";
    } else {
      cout << "Nice, next step!" << "\n";
      break;
    }
  }
}

void arrayInputer(int arrayToInput[], int arrayInputSize) {
  for (int i = 0; i < arrayInputSize; i++) {
    cout << "Enter your integer for an array: ";
    int arrayInputNumber;
    cin >> arrayInputNumber;

    arrayToInput[i] = arrayInputNumber;
  }
}

void arrayLogger(int arrayToLog[], int arrayLogSize) {
  cout << "Your array is: [";

  for (int i = 0; i < arrayLogSize; i++) {
    if (i > 0)
      cout << ", ";
    cout << arrayToLog[i];
  }

  cout << "]" << "\n";
}

void arrayIsIncreasing(int array[], int arrayCheckSize) {
  bool isIncreasing = true;
  for (int i = 0; i < arrayCheckSize - 1; i++) {
    if (array[i] > array[i + 1]) {
      isIncreasing = false;
      break;
    }
  }
  if (isIncreasing) {
    cout << "Correct! Your array is increasing (or not decreasing)!\n";
  } else {
    cout << "Incorrect! Your array is decreasing somewhere!\n";
  }
}

int main() {
  cout << "Enter integers for an array. The app will check, if the array is "
          "increasing/decreasing."
       << "\n"
       << "Firstly, enter the number of array entities (minimum 2)." << "\n";

  int arraySize = 0;
  arraySizeInputer(arraySize);

  int *array = new int[arraySize];

  arrayInputer(array, arraySize);

  arrayLogger(array, arraySize);

  arrayIsIncreasing(array, arraySize);

  getchar();
  getchar();

  delete[] array;

  return 0;
}