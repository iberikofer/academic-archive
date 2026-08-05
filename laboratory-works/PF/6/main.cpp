#include <cctype>
#include <cstring>
#include <iostream>

using namespace std;

int extractName(char *str, int const pos, char *name) {
  int currentPos = pos;
  int len = strlen(str);
  int nameStartPos = -1;
  int nameEndPos = -1;

  if (currentPos < 0 || currentPos >= strlen(str)) {
    return -1;
  }

  while (currentPos < len) {
    while (currentPos < len &&
           (isspace(str[currentPos]) || str[currentPos] == '.')) {
      ++currentPos;
    }

    if (isupper(str[currentPos])) {
      nameStartPos = currentPos;
      ++currentPos;

      while (currentPos < len && !isspace(str[currentPos])) {
        if (str[currentPos] == '.') {
          nameStartPos = -1;
          ++currentPos;
          break;
        }
        ++currentPos;
      }
    }
    if (nameStartPos == -1) {
      while (currentPos < len && !isspace(str[currentPos])) {
        ++currentPos;
      }
      continue;
    }

    while (currentPos < len && isspace(str[currentPos])) {
      ++currentPos;
    }

    if (nameStartPos >= 0) {
      if (isupper(str[currentPos])) {
        while (currentPos < len && !isspace(str[currentPos])) {
          if (str[currentPos] == '.') {
            nameStartPos = -1;
            ++currentPos;
            break;
          }
          ++currentPos;
        }

        nameEndPos = currentPos;
        break;
      } else if (!isupper(str[currentPos])) {
        nameStartPos = -1;
        while (currentPos < len && !isspace(str[currentPos])) {
          ++currentPos;
        }
        continue;
      }
    }
  }

  if (nameStartPos == -1) {
    return -1;
  }

  int i = 0;
  for (int j = nameStartPos; j < nameEndPos; ++j) {
    name[i] = str[j];
    ++i;
  }
  name[i] = '\0';

  return nameStartPos;
}

int main() {
  cout << "Enter the string, the app will check if it contains a name:" << endl;
  char str[150];
  cin.getline(str, 150);

  cout << "Also, enter the pos, where we will start our check: ";
  int pos = 0;
  cin >> pos;

  char name[30];

  int nameIndex = extractName(str, pos, name);

  if (nameIndex < 0) {
    cout << -1;
    return -1;
  }

  int nameLength = strlen(name);

  cout << "\"";
  for (int i = 0; i < nameLength; i++) {
    cout << name[i];
  }
  cout << "\" - found at index " << nameIndex << endl;

  getchar();
  getchar();

  return 0;
}