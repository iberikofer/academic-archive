#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

enum Suit { HEARTS, DIAMONDS, CLUBS, SPADES };
enum Value {
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN,
  JACK,
  QUEEN,
  KING,
  ACE
};
struct CARD {
  Suit cardSuit;
  Value cardValue;
};

CARD cardRequest() {
  CARD handCard;

  int randomSuit = rand() % 4;
  handCard.cardSuit = static_cast<Suit>(randomSuit);
  int randomValue = rand() % 13;
  handCard.cardValue = static_cast<Value>(randomValue);

  return handCard;
}

void cardOutput(CARD card) {
  switch (card.cardValue) {
  case 0:
    cout << "TWO";
    break;
  case 1:
    cout << "THREE";
    break;
  case 2:
    cout << "FOUR";
    break;
  case 3:
    cout << "FIVE";
    break;
  case 4:
    cout << "SIX";
    break;
  case 5:
    cout << "SEVEN";
    break;
  case 6:
    cout << "EIGHT";
    break;
  case 7:
    cout << "NINE";
    break;
  case 8:
    cout << "TEN";
    break;
  case 9:
    cout << "JACK";
    break;
  case 10:
    cout << "QUEEN";
    break;
  case 11:
    cout << "KING";
    break;
  case 12:
    cout << "ACE";
    break;
  default:
    break;
  }
  cout << " of ";
  switch (card.cardSuit) {
  case 0:
    cout << "HEARTS";
    break;
  case 1:
    cout << "DIAMONDS";
    break;
  case 2:
    cout << "CLUBS";
    break;
  case 3:
    cout << "SPADES";
    break;
  default:
    break;
  }
  cout << endl;
}

CARD hand[5];
int changedCardNumber = 1;
CARD change(CARD *hand) {
  CARD cardToChange;
  int cardOrder = 1;
  cardToChange.cardValue = static_cast<Value>(13);

  cout << "Here is your 5-card hand: " << endl;

  for (int j = 0; j < 5; j++) {
    bool isPairFound = false;

    for (int k = 0; k < 5; k++) {
      if (j == k)
        continue;

      if (hand[j].cardValue == hand[k].cardValue) {
        isPairFound = true;
        continue;
      }
    }

    if (hand[j].cardValue < cardToChange.cardValue && isPairFound == false) {
      cardToChange.cardValue = hand[j].cardValue;
      cardToChange.cardSuit = hand[j].cardSuit;
      changedCardNumber = j;
    }

    cout << cardOrder << ". ";
    cardOutput(hand[j]);
    ++cardOrder;
  }

  cout << "Recommendation - Discard the next card: " << changedCardNumber + 1
       << ". ";

  return cardToChange;
}

int main() {
  srand(time(0));

  for (int i = 0; i < 5; i++) {
    CARD myCard = cardRequest();
    hand[i] = myCard;
  }

  cardOutput(change(hand));

  return 0;
}