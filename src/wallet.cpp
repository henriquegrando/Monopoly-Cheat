#include "wallet.h"

Wallet::Wallet() {
  balance.ones = 0;
  balance.fives = 0;
  balance.tens = 0;
  balance.twenties = 0;
  balance.fifties = 0;
  balance.one_hundreds = 0;
  balance.five_hundreds = 0;
}

Wallet::Wallet(Bills bills) {
  this->balance = bills;
}

Wallet::Wallet(int value) {
  this->balance = convert(value);
}

void Wallet::setBalance(Bills bills) {
  this->balance = bills;
}

Bills Wallet::getBalance() {
  return this->balance;
}

Bills Wallet::convert(int value) {
  if(value < 0) {
    cerr << "Cannot convert negative value" << endl;
    return {0, 0, 0, 0, 0, 0, 0};
  }
  Bills newBills;

  newBills.five_hundreds = value / 500;
  value -= newBills.five_hundreds * 500;
  //cout << value << endl;

  newBills.one_hundreds = value / 100;
  value -= newBills.one_hundreds * 100;
  //cout << value << endl;

  newBills.fifties = value / 50;
  value -= newBills.fifties * 50;
  //cout << value << endl;

  newBills.twenties = value / 20;
  value -= newBills.twenties * 20;
  //cout << value << endl;

  newBills.tens = value / 10;
  value -= newBills.tens * 10;
  //cout << value << endl;

  newBills.fives = value / 5;
  value -= newBills.fives * 5;
  //cout << value << endl;

  newBills.ones = value;

  return newBills;
}

void Wallet::deposit(Bills bills) {
  this->balance += bills;
}

void Wallet::deposit(int value) {
  this->balance += convert(value);
}

bool Wallet::deduct(Bills bills) {
  // Check if there are enough bills to deduct
  if(bills > this->balance) {
    cerr << "Cannot deduct from wallet: not enough bills" << endl;
    return false;
  }

  this->balance -= bills;
  return true;
}

bool Wallet::deduct(int value) {
  deduct(convert(value));
}

bool Wallet::payTo(Wallet *entity, int value) {
  bool hasCredit = deduct(value);
  if(hasCredit)
    entity->deposit(value);

  return hasCredit;
}

void Wallet::receiveFrom(Wallet *entity, Bills bills) {
  if(entity->deduct(bills))
    deposit(bills);
}

void Wallet::receiveFrom(Wallet *entity, int value) {
  if(entity->deduct(value))
    deposit(value);
}

void Wallet::printBalance() {
  cout << balance;
}