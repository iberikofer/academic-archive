#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

void matrixCreator(int n, int m, int minValue, int maxValue,
                   vector<vector<int>> &myMatrix) {
  cout << "Your matrix is:" << "\n";

  for (int i = 0; i < n; ++i) {
    cout << "(";
    for (int j = 0; j < m; ++j) {
      int random_number = (rand() % (maxValue - minValue + 1)) + minValue;
      myMatrix[i][j] = random_number;
      j == m - 1 ? cout << myMatrix[i][j] << ")"
                 : cout << myMatrix[i][j] << "\t";
    }
    cout << "\n";
  }

  cout << "\n";
}

void columnsAddition(int n, int m, vector<vector<int>> &myMatrix,
                     vector<int> &myMatrixSums) {
  for (int j = 0; j < m; ++j) {
    for (int i = 0; i < n; i++) {
      if (myMatrix[i][j] < 0) {
        myMatrixSums[j] += myMatrix[i][j];
        break;
      } else {
        myMatrixSums[j] += myMatrix[i][j];
      }
    }
  }

  cout << "Your rows Sums are:" << endl << "(";
  for (int i = 0; i < m; ++i) {
    i == m - 1 ? cout << myMatrixSums[i] << ")"
               : cout << myMatrixSums[i] << "\t";
  }
  cout << "\n";
}

int main() {

  int n = 0, m = 0;
  cout << "Enter the N value(number of rows): ";
  cin >> n;
  cout << "Enter the M value(number of columns): ";
  cin >> m;

  int minValue = 0;
  int maxValue = 0;
  cout << "Enter the MIN value for matrix numbers: ";
  cin >> minValue;
  cout << "Enter the MAX value for matrix numbers: ";
  cin >> maxValue;

  srand(time(0));
  vector<vector<int>> myMatrix(n, vector<int>(m));
  matrixCreator(n, m, minValue, maxValue, myMatrix);

  vector<int> myMatrixSums(m);
  columnsAddition(n, m, myMatrix, myMatrixSums);

  getchar();
  getchar();

  return 0;
}