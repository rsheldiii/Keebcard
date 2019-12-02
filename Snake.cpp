#include "settings.h"
#include "Snake.h"


const uint8_t BOARD_WIDTH = 48;
const uint8_t BOARD_HEIGHT = 16;
const uint8_t STARTING_SCORE = 4;

// the board is not a multiple of the screen height, so this is the difference
// from one edge of the screen to the board
const uint8_t HORIZONTAL_GAP = ((64 - BOARD_WIDTH) / 2);

// if you're reading this and it's still broken out it's for testing purposes
// each byte represents a 16x2 swath of pixels, which is then of course 8 positions 2x2.
// swaths start in the top left and proceed rightwards in rows
// pixels start at the top as well and go downwards
// 8 for uint8_t
const uint8_t BOARD_SIZE = BOARD_WIDTH * BOARD_HEIGHT / 8;


// divided by 4 because quadruple direction
static QuadrupleDirection SnakePath::snakePath[BOARD_WIDTH * BOARD_HEIGHT / 4] = { { RIGHT, RIGHT, RIGHT, RIGHT }, { RIGHT, RIGHT, RIGHT, RIGHT }, { RIGHT, RIGHT, RIGHT, RIGHT } }; // TODO NEEDS TO BE 256(ish) but memory instability

Direction SnakePath::get(uint16_t index) {
  QuadrupleDirection d = snakePath[index >> 2];
  switch((uint8_t)(index & 0b11)) {
    case 0:
      return d.firstDirection;
    case 1:
      return d.secondDirection;
    case 2:
      return d.thirdDirection;
    case 3:
      return d.fourthDirection;
  }
}

void SnakePath::set(uint16_t index, Direction direction) {
  switch((uint8_t)(index & 0b11)) {
    case 0:
      snakePath[index >> 2].firstDirection = direction;
      break;
    case 1:
      snakePath[index >> 2].secondDirection = direction;
      break;
    case 2:
      snakePath[index >> 2].thirdDirection = direction;
      break;
    case 3:
      snakePath[index >> 2].fourthDirection = direction;
      break;
  }
}

static uint8_t Snake::board[BOARD_SIZE] = { 0 };

static Position Snake::tailPosition = {1, 7};
static Position Snake::headPosition = {1 + STARTING_SCORE + 1, 7};
static Position Snake::foodPosition = {32, 7};

static Direction Snake::direction = RIGHT;
static Direction Snake::lastDirection = RIGHT;

static uint16_t Snake::score = STARTING_SCORE;
static bool Snake::gameOver = false;
static bool Snake::scoredThisTurn = false;
static uint8_t Snake::time;
static uint8_t Snake::debounce = 0;
static uint8_t Snake::linksToAdd = 0;

// snakePath defines the association between snake links. Snake links can only be the cardinal neighbor of their next-of-kin, so we only need 4 bits to represent this

Snake::Snake(SSD1306Device* _oled) {
  oled = _oled;
  oled->fill(0);
}

// set up the play area. used at start and for pause
void Snake::setup() {
  oled->setMemoryAddressingMode(1);
  // * 2 because we are rasterized - every pixel is a 2x2 squares
  oled->setCursor((HORIZONTAL_GAP - 1) * 2,0);
  oled->startData();
  for(uint8_t j = 0; j < 8; j++){
    oled->sendData(0xffff);
  }

  oled->endData();
  oled->setCursor((BOARD_WIDTH + HORIZONTAL_GAP) * 2, 0);
  for(uint8_t i = 0; i < 8; i++) {
    oled->sendData(0xffff);
  }
  oled->endData();
  oled->setMemoryAddressingMode(0);

  Position position = tailPosition;

  // "initialize" snake onto screen
  sendToGrid(position, true);
  for (uint8_t i = 0; i < score; i++) {
    Direction d = snakePath.get(i);
    addDeltaToPosition(position, d);
    sendToGrid(position, true);
  }
  sendToGrid(headPosition, true);
}

uint16_t Snake::run() {
  setup();

  setNewFoodPosition();

  while(!gameOver) {
    checkForPause();
    // we don't have to but, just in case...
    sendToGrid(foodPosition, true);

    time = millis();

    checkForScore();
    checkInputs();

    moveSnake();
    // cache last direction to use it when moving the head later
    // for a brief moment direction and lastDirection are both the same direction,
    // but we correct immediately.
    // this isn't done in checkInputs as we call that multiple times
    lastDirection = direction;

    // better than a no op
    while ((uint8_t)(millis() - time) < 120) { // 160
      checkInputs();
    }

    checkGameOver();

    // for debugging food placement
    // oled->setCursor(0,0);
    // oled->print(foodPosition.x);

    // delay(max(120 - (millis() - time), 0));
    // this is for debugging
    // renderScreen();
  }
  return score - 4;
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

  if (digitalRead(LEFT_BUTTON) == LOW) {
    // not milliseconds, invocations of checkInputs
    debounce = millis();
    if (lastDirection == LEFT) {
      direction = DOWN;
    } else {
      direction = (uint8_t)lastDirection-1;
    }
  }

  if (digitalRead(RIGHT_BUTTON) == LOW) {
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
    Direction linkBeforeTail = snakePath.get(0);
    // move tailPosition to last link in the chain
    // we will eventually break apart the uint8_t to two uint4_t and cast to a Direction
    // these numbers might not match up then
    addDeltaToPosition(tailPosition, linkBeforeTail);
  }
}

void Snake::updateHeadPosition() {
  if ((score > 0) || linksToAdd > 0) {
    // push previous direction of head onto snakePath
    snakePath.set(score, lastDirection);

    // shrink path by 1 if not scored
    if (linksToAdd == 0) {
      for (uint16_t i = 0; i < score; i++) {
        snakePath.set(i, snakePath.get(i+1));
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
  // modulo
  uint8_t col = position.y & 7;

  // here we actually set the new pixel in the grid
  if (value) {
    *row |= 1 << col;
  } else {
    *row &= ~(1 << col);
  }

  // we now copy the row in order to avoid trampling over it with bit shifting
  uint8_t displayRow = *row;

  if ((position.y >> 2) & 1 == 1) {
    displayRow = displayRow >> 4;
  }

  // * 2 should really be 128 / BOARD_WIDTH but also 128 should be a const
  oled->setCursor((position.x + HORIZONTAL_GAP) * 2, position.y >> 2);
  oled->startData();
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

    // gotta check first, increment next, in order to check the tailPosition
    if (snakePosition.x == collisionPosition.x && snakePosition.y == collisionPosition.y) {
      return true;
    }

    Direction d = snakePath.get(i);
    addDeltaToPosition(snakePosition, d);
  }

  if (includeHead && headPosition.x == collisionPosition.x && headPosition.y == collisionPosition.y) {
    return true;
  }

  return false;
}

void Snake::checkForPause() {
  if (digitalRead(MIDDLE_BUTTON) == LOW) {
    oled->clear();
    oled->setCursor(40, 1);
    oled->print(F("PAUSED"));

    while(!digitalRead(MIDDLE_BUTTON) == LOW){
      delay(1);
    }

    oled->clear();

    setup();
  }
}

void Snake::checkGameOver() {
  // where are the < 0 checks you ask? well these are uints.
  // why are they uints you exclaim, we need to check if we go below zero!
  // this already does that. because it's unsigned the uint underflows
  if (headPosition.x >= BOARD_WIDTH || headPosition.y >= BOARD_HEIGHT) {
    gameOver = true;
    return;
  }

  // check head to tail collision
  if (tailPosition.x == headPosition.x && tailPosition.y == headPosition.y) {
    gameOver = true;
    return;
  }

  // check for body collision
  if (checkForCollision(headPosition, false)) {
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
  // this one's a doozy because it's actually kind of hard to find a random spot

  // do a random check real quick to see if we can't land something
  foodPosition = { rand() % BOARD_WIDTH, rand() % BOARD_HEIGHT };

  if (checkForCollision(foodPosition, true)) {
    // oh boy, you've done it now
    // when the going gets tough and all the food spots are taken, instead of
    // randomly searching for one, let's find how many there are and choose one
    // at random. We are incredibly low on memory so we pass through twice
    // instead of keeping a ledger of what spaces are open
    uint8_t openSegments = 0;
    for (uint8_t s = 0; s < BOARD_SIZE; s++) {
      if (board[s] != 0xffff) {
        openSegments++;
      }
    }

    uint8_t nextSpot = (rand() % openSegments) + 1;
    openSegments = 0;

    for (uint8_t segmentIndex = 0; segmentIndex < BOARD_SIZE; segmentIndex++) {
      if (board[segmentIndex] != 0xffff) {
        openSegments++;
        if (openSegments == nextSpot){
          //
          uint8_t y = segmentIndex / BOARD_WIDTH;
          uint8_t x = segmentIndex % BOARD_WIDTH;

          for (uint8_t i = 0; i < 8; i++) {
            if (!checkForCollision({x, y+i}, true)) {
              foodPosition = {x, y + i};
              sendToGrid(foodPosition, true);
              return;
            }
          }
        }
      }
    }

    // they won, what do we do?
  } else {
    sendToGrid(foodPosition, true);
  }

  // why not right
  srand(millis() + analogRead(MIDDLE_BUTTON));
}

// void Snake::renderScreen() {
//   for (uint8_t y = 0; y < 4; y++) { // 4 pages in vertical addressing mode TODO REPLACE WITH 4
//     oled->setCursor(0,y);
//
//     oled->startData();
//     for (uint8_t x = 0; x < BOARD_WIDTH; x++) {
//       uint8_t row = board[x + (BOARD_WIDTH * (y / 2))];
//       if (y % 2 == 1) {
//         // leave the squirrels
//         row = row >> 4;
//       }
//
//       // delay(1);
//       // renderRow(row);
//     }
//     oled->endData();
//   }
//
//   // oled->switchFrame();
// }
//
// // really uint4_t
// // do setCursor first
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
