#ifndef H_WALLET
#define H_WALLET

class Wallet {
  private:
    int balance;
    void deposit(int);
    bool deduct(int);

  public:
    Wallet();

    void setBalance(int);
    int getBalance();

    bool payTo(Wallet*, int);
};

#endif
