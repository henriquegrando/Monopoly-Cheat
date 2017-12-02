#include "player.h"
#include "gamecontroller.h"

Player::Player(int id, string name) {
  this->id = id;
  this->name = name;
  this->position = GO;

  Bills initialBalance = {5, 1, 2, 1, 1, 4, 2};
  this->wallet.receiveFrom(&Bank::Balance, initialBalance);

  this->inJail = false;
  this->hasJailCard = false;
  this->roundsInJail = 0;
  this->ownedRailroads = 0;
  this->ownedUtilities = 0;

  this->buyingChance = 60;
  this->buildingChance = 80;
  this->payingJailChance = 80;
  this->mortgageChance = 30;
  this->minimumBalance = 50;
}

int Player::getId() {
  return id;
}

string Player::getName() {
  return this->name;
}

int Player::getPosition() {
  return this->position;
}

int Player::getOwnedUtilities() {
  return ownedUtilities;
}

int Player::getOwnedRailroads() {
  return ownedRailroads;
}

void Player::goTo(int position) {
  if(position < this->position) {
    cout << "\tPAYING 200 TO PLAYER!!" << endl;
    if(Bank::Balance.payTo(&this->wallet, 200)) {
      cout << "\t" << name << " passed GO and received 200 from Bank" << endl;
    }

    else throw PAY_FAILED;
  }

  this->position = position;
}

void Player::goToJail() {
  this->inJail = true;
  this->position = JAIL;
}

void Player::stepOnTile(Board::Tile *tile) {
  switch (tile->getType()){
    case PropertyTile: {
      Card *card = tile->getCard();

      // Player already owns card
      if(id == card->owner)
        cout << "\t" << name << " already owns " << card->name << endl;
      // Card is owned by another player
      else if(card->owner != -1) {
        cout << "\t" << "Player " << (card->owner+1) << " owns " << card->name << endl;
        // Don't pay rent if card is mortgaged
        if(card->isMortgaged)
          break;

        Player *otherPlayer = GameController::getPlayer(card->owner);
        int rent = 0;

        // Calculate rent
        switch(card->getType()) {
          case PropertyCard: {
            TitleDeed *property = (TitleDeed*) card;

            if(property->hasHotel)
              rent = property->rent[5];
            else rent = property->rent[property->n_houses];
            break;
          }
          case UtilityCard: {
            if(otherPlayer->getOwnedUtilities() == 1)
              rent = 4 * (Board::getDie(0) + Board::getDie(1));

            else if(otherPlayer->getOwnedUtilities() == 2)
              rent = 10 * (Board::getDie(0) + Board::getDie(1));

            else throw ANY_ERROR;
            break;
          }
          case RailroadCard: {
            Railroad *railroad = (Railroad*) card;

            rent = railroad->rent[otherPlayer->getOwnedRailroads() - 1];
            break;
          }
        }

        // Pay rent - OBLIGATORY
        if(wallet.payTo(&otherPlayer->wallet, rent))
          cout << "\t" << name << " paid " << rent << " to " << otherPlayer->getName() << endl;
        else {
          throw PAY_FAILED;
          //mortgage(rent);
        }
      }
      // Nobody owns card. Player may buy it
      else {
        if((this->wallet.getBalanceValue() - card->price) < this->minimumBalance) {
          cout << "\t" << name << " is at minimum balance. Property not bought." << endl;
        }
        else {
          cout << "\t" << name << " has " << buyingChance << "\% chance of buying " << card->name << endl;
          int chance = rand() % 100;
          if(chance <= buyingChance)
            buy(card);
        }
      }
      break;
    }
    case ChestTile:
      processEventCard(Board::getEventCard(tile->getType()));
      break;
    case ChanceTile:
      processEventCard(Board::getEventCard(tile->getType()));
      break;
    case JailTile:
      // nothing to do here
      cout << "\t" << name << " landed on Jail (no worries!)" << endl;
      break;
    case GoToJailTile:
      cout << "\t" << name << " landed on Go To Jail (oh no!)" << endl;
      this->goToJail();
      break;
    case GoTile:
      // nothing to do here
      cout << "\t" << name << " landed on GO" << endl;
      break;
    case FreeParkingTile:
      // nothing to do here
      cout << "\t" << name << " landed on Free Parking" << endl;
      break;
    case IncomeTaxTile:
      if(this->wallet.payTo(&Bank::Balance, 200))
        cout << "\t" << name << " paid 200 in Income Tax" << endl;
      else throw PAY_FAILED;

      break;
    case LuxuryTaxTile:
      if(this->wallet.payTo(&Bank::Balance, 100))
        cout << "\t" << name << " paid 100 in Luxury Tax" << endl;
      else throw PAY_FAILED;
      break;
  }
}

void Player::processEventCard(EventCard *card) {
  cout << "\t" << name << " drew " << card->description << endl;
  // Switch type of drawn card
  switch(card->effectType) {
    // Receive amount from Bank
    case Collect:
      //this->wallet.receiveFrom(&Bank::Balance, card->value);
      Bank::Balance.payTo(&this->wallet, card->value);
      cout << "\t" << name << " collected " << card->value << endl;
      break;

    // Pay amount to bank
    case Pay:
      if(this->wallet.payTo(&Bank::Balance, card->value))
        cout << "\t" << name << " paid " << card->value << endl;
      break;

    // Move to tile
    case GoToTile:
      goTo(card->tile);
      break;

    // Go to next utility
    case GoToUtility: {
      int destination = -1;
      switch(position) {
        case CHANCE_1:
          destination = ELECTRIC_CO;
          break;
        case CHANCE_2:
          destination = WATER_WORKS;
          break;
        case CHANCE_3:
        destination = ELECTRIC_CO;
          break;
        default:
          throw ANY_ERROR;
          break;
      }

      goTo(destination);

      Utility *utility = (Utility*) Board::getTile(destination)->getCard();
      // Unowned. Player may buy it
      if(utility->owner == -1) {
        if((this->wallet.getBalanceValue() - utility->price) < this->minimumBalance) {
          cout << "\t" << name << " is at minimum balance. Utility not bought." << endl;
        }
        else {
          cout << "\t" << name << " has " << buyingChance << "\% chance of buying " << utility->name << endl;
          int chance = rand() % 100;
          if(chance <= buyingChance)
            buy(utility);
        }
      }

      // Card owned by another player
      else if(utility->owner != id) {
        cout << "\t" << "Player " << (utility->owner+1) << " owns " << utility->name << endl;

        Player *otherPlayer = GameController::getPlayer(utility->owner);

        Board::rollDice();
        int rent = 10 * (Board::getDie(0) + Board::getDie(1));
        if(wallet.payTo(&otherPlayer->wallet, rent))
          cout << "\t" << name << " paid " << rent << " to " << otherPlayer->getName() << endl;
        else {
          throw PAY_FAILED;
          //mortgage(rent);
        }
      }
      break;
    }

    // Go to next railroad
    case GoToRailroad: {
      int destination = -1;
      switch(position) {
        case CHANCE_1:
          destination = PENNSYLVANIA_RR;
          break;
        case CHANCE_2:
          destination = BO_RR;
          break;
        case CHANCE_3:
          destination = READING_RR;
          break;
        default:
          throw ANY_ERROR;
          break;
      }

      goTo(destination);
      Railroad *railroad = (Railroad*) Board::getTile(destination)->getCard();

      // Unowned. Player may buy it
      if(railroad->owner == -1) {
        if((this->wallet.getBalanceValue() - railroad->price) < this->minimumBalance) {
          cout << "\t" << name << " is at minimum balance. Railroad not bought." << endl;
        }
        else {
          cout << "\t" << name << " has " << buyingChance << "\% chance of buying " << railroad->name << endl;
          int chance = rand() % 100;
          if(chance <= buyingChance)
            buy(railroad);
        }
      }

      // Card owned by another player
      else if(railroad->owner != id) {
        cout << "\t" << "Player " << (railroad->owner+1) << " owns " << railroad->name << endl;

        Player *otherPlayer = GameController::getPlayer(railroad->owner);

        int rent = 2 * railroad->rent[otherPlayer->getOwnedRailroads() - 1];

        if(wallet.payTo(&otherPlayer->wallet, rent))
          cout << "\t" << name << " paid " << rent << " to " << otherPlayer->getName() << endl;
        else {
          throw PAY_FAILED;
          //mortgage(rent);
        }
      }
      break;
    }

    // Go back 3 spaces
    case GoBack3: {
      int newPosition = this->position - 3;
      if(newPosition < 0)
        newPosition = 40 + newPosition;
      this->position = newPosition;
      //goTo(newPosition);
      break;
    }

    // Go to jail
    case GoToJail:
      goToJail();
      break;

    // Pay for each house and hotel
    case GeneralRepairs: {
      int i;
      for(i = 0; i < this->cards.size(); i++) {
        Card *card = this->cards.at(i);
        if(card->getType() == PropertyCard) {
          TitleDeed *property = (TitleDeed*) card;
          // Pay for hotel
          if(property->hasHotel) {
            if(!this->wallet.payTo(&Bank::Balance, 100))
              throw PAY_FAILED;
          }
          // Pay for houses
          else if(property->n_houses > 0) {
            if(!this->wallet.payTo(&Bank::Balance, 25*property->n_houses))
              throw PAY_FAILED;
          }
        }
      }
      break;
    }

    // Pay for each house and hotel
    case StreetRepairs: {
      int i;
      for(i = 0; i < this->cards.size(); i++) {
        Card *card = this->cards.at(i);
        if(card->getType() == PropertyCard) {
          TitleDeed *property = (TitleDeed*) card;
          // Pay for hotel
          if(property->hasHotel) {
            if(!this->wallet.payTo(&Bank::Balance, 115))
              throw PAY_FAILED;
          }

          else if(property->n_houses > 0) {
            if(!this->wallet.payTo(&Bank::Balance, 40*property->n_houses))
              throw PAY_FAILED;
          }
        }
      }
      break;
    }
    case PayToAll:
      GameController::payAll(this, card->value);
      break;
    case ReceiveFromAll:
      GameController::receiveFromAll(this, card->value);
      break;
  }
}

void Player::buy(Card *card) {
  if(wallet.payTo(&Bank::Balance, card->price)) {
    cards.push_back(card);
    card->owner = this->id;

    cout << "\t" << name << " bought " << card->name << endl;

    switch(card->getType()) {
      // Increase owned of color
      case PropertyCard: {
        TitleDeed *deed = (TitleDeed*) card;
        int i;
        ColorSet *set = NULL;
        for(i = 0; i < colorsets.size(); i++) {
          set = colorsets[i];
          if(set->getColor() == deed->color) {
            set->addCard(deed);
            break;
          }
        }
        // Color set not present
        if(i == colorsets.size()) {
          set = new ColorSet(deed->color);
          set->addCard(deed);
          colorsets.push_back(set);
        }
        //colorSets[deed->color]++;
        break;
      }
      // Increase number of owned utilities
      case UtilityCard:
        ownedUtilities++;
        break;
      // Increase number of owned railroads
      case RailroadCard:
        ownedRailroads++;
        break;
    }
  }

  else {
    cout << "\t" << name << " does not have enough credit to buy " << card->name << endl;
    //throw PAY_FAILED;
  }
}

void Player::build(TitleDeed *deed) {
  if(deed->hasHotel) {
    cout << "\t" << deed->name << " has hotel. Cannot build anymore." << endl;
    return;
  }

  if(deed->n_houses == 4) {
    if(wallet.getBalanceValue() - deed->hotel_cost < minimumBalance) {
      cout << "\t" << name << " could not build a hotel on " << deed->name << endl;
      return;
    }

    cout << "\tBuilding house on " << deed->name << " with cost " << deed->hotel_cost << endl;
    if(this->wallet.payTo(&Bank::Balance, deed->hotel_cost)) {
      deed->n_houses = 0;
      deed->hasHotel = true;
      cout << "\t" << name << " built a hotel on " << deed->name << endl;
      return;
    }
    else throw PAY_FAILED;
  }

  if(wallet.getBalanceValue() - deed->house_cost < minimumBalance) {
    cout << "\t" << name << " could not build a house on " << deed->name << endl;
    return;
  }

  cout << "\tBuilding house on " << deed->name << " with cost " << deed->house_cost << endl;
  if(this->wallet.payTo(&Bank::Balance, deed->house_cost)) {
    deed->n_houses++;
    cout << "\t" << name << " built a house on " << deed->name << endl;
    return;
  }
  else throw PAY_FAILED;
}

void Player::tryToBuild() {
  cout << "\t" << name << " is deciding to build" << endl;
  if(this->wallet.getBalanceValue() <= this->minimumBalance) {
    cout << "\t" << name << " is at minimum balance. Nothing was built." << endl;
    return;
  }

  cout << "\t" << name << " has " << buildingChance << "\% chance of building" << endl;
  int chance = rand() % 100;

  // Check if player has all cards of same color for building
  if(chance <= buildingChance) {
    cout << "\t" << name << " owns:" << endl;
    //string colors[8] = {"purple", "cyan", "pink", "orange", "red", "yellow", "green", "blue"};
    int i;
    for(i = 0; i < colorsets.size(); i++) {
      ColorSet *colorset = colorsets[i];
      cout << "\t\t" << colorset->getName() << "(" << colorset->getSize() << "):" << endl;

      int j;
      for(j = 0; j < colorset->getSize(); j++) {
        cout << "\t\t\t" << colorset->getCard(j)->name;
        cout << "(" << colorset->getCard(j)->n_houses << "), " << colorset->getCard(j)->hasHotel << endl;
      }

      // Can't build if a card is mortgaged
      if(colorset->hasMortgage())
        continue;

      if(colorset->hasAllCards()) {
        build(colorset->getCard(0));
      }
    }
  }
  else {
    cout << "\t" << name << " did not build anything this round" << endl;
  }
}

void Player::tryToMortgage(int value) {
  int i, min = 0, mortgageIndex = -1;
  // Find the minimum mortgage to cover for debt
  string colors[8] = {"purple", "cyan", "pink", "orange", "red", "yellow", "green", "blue"};
  for(i = 0; i < cards.size(); i++) {
    Card *card = cards[i];
    if(card->mortgage > min && card->mortgage >= value) {
      min = card->mortgage;
      mortgageIndex = i;
    }
  }

  if(mortgageIndex != -1) {
    Card *card = cards[mortgageIndex];
    card->isMortgaged = true;
  }
}

bool Player::paidToGetOutOfJail() {
  cout << "\t" << name << " is trying to pay to get out of jail" << endl;
  if((this->wallet.getBalanceValue() - 50) < this->minimumBalance) {
    cout << "\t" << name << " is at minimum balance. Will not pay to exit jail." << endl;
    return false;
  }

  cout << "\t" << name << " has " << payingJailChance << "\% chance of paying to leave jail" << endl;
  int chance = rand() % 100;

  if(chance <= payingJailChance) {
    if(!wallet.payTo(&Bank::Balance, 50))
      throw PAY_FAILED;
    else return true;
  }

  else {
    cout << "\t" << name << " did not pay to leave jail" << endl;
    return false;
  }
}
