#include <cctype>
#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
// Створити програму, що у текстовому файлі шукає адреси електронної пошти
// і друкує на екран у вигляді списку у форматі.

string wordToFormat(string wordToCheck) {
  for (int i = 0; i < wordToCheck.length(); i++) {
    wordToCheck[i] = tolower(wordToCheck[i]);
  }

  return wordToCheck;
}

//
void userLog(string word, string forattedWord, vector<string> domains,
             int &count) {
  for (size_t i = 0; i < domains.size(); i++) {
    if (forattedWord.find(domains[i]) != string::npos) {
      int userNameIndex = word.find("@");

      cout << "User " << count << ") " << word.substr(0, userNameIndex) << ": ["
           << forattedWord << "]" << endl;
      count++;
      break;
    }
  }
}

//
int main() {
  ifstream myLabFile("emails.txt");
  string currentWord;
  vector<string> emailDomains = {"@email.com",  "@gmail.com",   "@ukr.net",
                                 "@icloud.com", "@outlook.com", "@yahoo.com",
                                 "@mail.com"};
  int userCount = 1;

  if (myLabFile.is_open()) {
    while (myLabFile >> currentWord) {
      if (currentWord.find("@") != string::npos) {
        string currentFormattedWord = wordToFormat(currentWord);
        userLog(currentWord, currentFormattedWord, emailDomains, userCount);
      }
    }

    myLabFile.close();
  }

  return 0;
}