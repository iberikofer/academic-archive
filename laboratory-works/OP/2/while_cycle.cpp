#include <iostream>
#include <windows.h>
using namespace std;

int main() {
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);

  int number;

  cout << "Введіть число більше 0: ";
  cin >> number;

  while (number <= 0) {
    cout << "Невірне значення! Спробуйте ще раз: ";
    cin >> number;
  }

  cout << "Дякую! Ви ввели:" << number;

  return 0;
}