#include "settings.h"
#include "Snake.h"


#define BOARD_WIDTH 64
#define BOARD_HEIGHT 16

// if you're reading this and it's still broken out it's for testing purposes
// each byte represents a 16x2 swath, which is then of course 8 pixels 2x2.
// swaths start in the top left and proceed rightwards in rows
// pixels start at the top as well and go downwards
static uint8_t Snake::board[128] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static Position Snake::headPosition = {2, 7};
static Position Snake::tailPosition = {1, 7};
static Position Snake::foodPosition = {32, 7};

static Direction Snake::direction = RIGHT;
static Direction Snake::lastDirection = RIGHT;

static uint16_t Snake::score = 0;
static bool Snake::gameOver = false;
static bool Snake::scoredThisTurn = false;
static uint8_t Snake::time;
static uint8_t Snake::debounce = 0;
static uint8_t Snake::linksToAdd = 4;

// snakePath defines the association between snake links. Snake links can only be the cardinal neighbor of their next-of-kin, so we only need 4 bits to represent this
// I'll need to make these actually 2 bits eventually
static uint8_t Snake::snakePath[128] = { RIGHT, RIGHT, RIGHT, RIGHT, 0 }; // TODO NEEDS TO BE 256(ish) but memory instability

Snake::Snake(SSD1306Device* _oled) {
  oled = _oled;
  oled->fill(0x0);
}

void Snake::run() {
  oled->switchRenderFrame();
  Position position = tailPosition;
  sendToGrid(tailPosition, true);
  for (uint8_t i = 0; i < score; i++) {
    Direction d = snakePath[i];
    addDeltaToPosition(position, d);
    sendToGrid(position, true);
  }
  sendToGrid(headPosition, true);
  sendToGrid(foodPosition, true);

  while(!gameOver) {
    time = millis();
    // cache last direction to use it when moving the head later
    // for a brief moment direction and lastDirection are both the same direction,
    // but we correct immediately.
    // this isn't done in checkInputs as we may call that multiple times


    checkGameOver();
    checkForScore();
    checkInputs();

    moveSnake();
    lastDirection = direction;

    // better than a no op
    while ((uint8_t)(millis() - time) < 120) { // 160
      checkInputs();
    }

    // delay(max(120 - (millis() - time), 0));
    // this is for debugging
    // renderScreen();
  }

  oled->fill(0);
  oled->setCursor(0,0);
  oled->print(F("Game Over!"));
  oled->setCursor(0,2);
  oled->print(F("Score: "));
  oled->print(score-4);
}

void Snake::checkInputs() {
  // TODO if you're really lucky millis() will wrap to 0 and this doesn't work. 1/256 of the time
  // lol I could just check when I set it I guess
  // but it works for now
  if (debounce !=0) {
    if (uint8_t(millis() - debounce) < 150) {
      return;
    } else {
      debounce = 0;
    }
  }

  if (digitalRead(LEFT_ARROW) == LOW) {
    // not milliseconds, invocations of checkInputs
    debounce = millis();
    if (lastDirection == LEFT) {
      direction = DOWN;
    } else {
      direction = (uint8_t)lastDirection-1;
    }
  }

  if (digitalRead(RIGHT_ARROW) == LOW) {
    debounce = millis();
    // direction is in order - left up right down - so add 1 and % 4 and poof it works
    direction = (lastDirection+1) & 3;
  }
}

void Snake::moveSnake() {
  sendToGrid(tailPosition, false);
  // placed first so we can use the old head position
  updateTailPosition();

  // we don't have to sendToGrid(tailPosition, true) since there should always already be a pixel at the new tail position. it's great for testing though
  sendToGrid(tailPosition, true);

  updateHeadPosition();

  sendToGrid(headPosition, true);
}


// must be called _before_ head moves
void Snake::updateTailPosition() {
  // tail doesn't move the turn we score
  if (linksToAdd > 0) {
    return;
  }
  // no links if score is 0
  if (score == 0) {
    tailPosition = {headPosition.x, headPosition.y}; // TODO should be able to just do headPosition. shallow copy but no pointers so meh
  } else {
    // get last link in the chain
    uint8_t linkBeforeTail = snakePath[0];
    // move tailPosition to last link in the chain
    // we will eventually break apart the uint8_t to two uint4_t and cast to a Direction
    // these numbers might not match up then
    addDeltaToPosition(tailPosition, (Direction)linkBeforeTail);
  }
}

void Snake::updateHeadPosition() {
  if ((score > 0) || linksToAdd > 0) {
    // push previous direction of head onto snakePath
    snakePath[score] = lastDirection;

    // shrink path by 1 if not scored
    if (linksToAdd == 0) {
      for (uint8_t i = 0; i < score; i++) {
        snakePath[i] = snakePath[i+1];
      }
    } else {
      // do it now so snakePath[score] isn't wrong
      linksToAdd--;
      score++;
      if (scoredThisTurn){
        setNewFoodPosition();
      }
    }
  }
  // move head according to that direction
  addDeltaToPosition(headPosition, direction);
}

void Snake::sendToGrid(Position position, bool value) {
  // many AVR chips have no division operator. it works, but it's _very_ slow.
  // bit shifting is division with no remainder. so, `>> 3` = `/ 8`
  uint8_t* row = &board[position.x + (BOARD_WIDTH * (position.y >> 3))];
  // modulo 8
  uint8_t col = position.y & 7;

  if (value) {
    *row |= 1 << col;
  } else {
    *row &= ~(1 << col);
  }

  uint8_t displayRow = *row;

  if ((position.y >> 2) & 1 == 1) {
    displayRow = displayRow >> 4;
  }

  // * 2 should really be 128 / BOARD_WIDTH but also 128 should be a const
  oled->setCursor(position.x * 2, position.y >> 2);
  oled->startData();
  // renderRow(0xffffffff);
  renderRow(displayRow);
  oled->endData();
}

void Snake::addDeltaToPosition(Position &pos, Direction delta) {
  switch(delta) {
    case RIGHT:
      pos.x +=1;
      break;
    case DOWN:
      pos.y +=1;
      break;
    case LEFT:
      pos.x -= 1;
      break;
    case UP:
      pos.y -= 1;
      break;
  }
}

bool Snake::checkForCollision(Position collisionPosition, bool includeHead) {
  Position snakePosition = tailPosition;

  for (uint16_t i = 0; i < score; i++) {
    Direction d = snakePath[i];
    addDeltaToPosition(snakePosition, d);

    if (snakePosition.x == collisionPosition.x && snakePosition.y == collisionPosition.y) {
      return true;
    }

    if (includeHead && headPosition.x == collisionPosition.x && headPosition.y == collisionPosition.y) {
      return true;
    }

    return false;
  }
}

void Snake::checkGameOver() {
  // check head to tail collision
  if (tailPosition.x == headPosition.x && tailPosition.y == headPosition.y) {
    gameOver = true;
  }

  // check for body collision
  if (checkForCollision(headPosition, false)) {
    gameOver = true;
  }

  if (headPosition.x >= BOARD_WIDTH || headPosition.x < 0 || headPosition.y >= BOARD_HEIGHT || headPosition.y < 0) {
    gameOver = true;
  }
}

void Snake::checkForScore() {
  scoredThisTurn = (foodPosition.x == headPosition.x && foodPosition.y == headPosition.y);
  if (scoredThisTurn) {
    linksToAdd += 2;
  }
}

void Snake::setNewFoodPosition() {
  srand(millis());

  // the dreaded do while
  // TODO this is gonna suck with large snakes plz fix
  do {
    foodPosition = { rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };
  } while (checkForCollision(foodPosition, true));

  // in order to avoid high memory costs we'll need to scan for how many spots we have and choose one eventually
  // uint16_t openSpots = 0;
  // for (uint8_t x = 0; x < BOARD_WIDTH; x++) {
  //   for (uint8_t y)
  // }

  sendToGrid(foodPosition, true);
}

void Snake::renderScreen() {
  for (uint8_t y = 0; y < 4; y++) { // 4 pages in vertical addressing mode TODO REPLACE WITH 4
    oled->setCursor(0,y);

    oled->startData();
    for (uint8_t x = 0; x < BOARD_WIDTH; x++) {
      uint8_t row = board[x + (BOARD_WIDTH * (y / 2))];
      if (y % 2 == 1) {
        // leave the squirrels
        row = row >> 4;
      }

      // delay(1);
      // renderRow(row);
    }
    oled->endData();
  }

  // oled->switchFrame();
}

// really uint4_t
// do setCursor first
void Snake::renderRow(uint8_t row) {
  // "inflating" the row
  // 0101 = 00110011
  // 1110 = 11111100, etc
  row = (row & 8) << 4 | (row & 8) << 3 | (row & 4) << 3 | (row & 4) << 2 | (row & 2) << 2 | (row & 2) << 1 | (row & 1) << 1 | (row & 1);

  oled->sendData(row);
  oled->sendData(row);
  // board width is half the pixel size so
  // oled->sendData(0x00);
}
