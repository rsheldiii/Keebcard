#include "settings.h"
#include "Pong.h"

//--- CONSTANTS

// letters are guaranteed to be this big
// even if they don't look like it - they do not mask, they are 8x16 blocks
const uint8_t FONT_WIDTH = 8;

// total width of the screen
const uint8_t TOTAL_WIDTH = 128;
const uint8_t TOTAL_HEIGHT = 32;

// positions of the paddles
const uint8_t PLAYER_X = 30;
const uint8_t ENEMY_X = 98;

const uint8_t PADDLE_LENGTH = 8;
// mostly to pull out the magic number and give it a name
// this is used for random generation and paddle bouncing
const uint8_t MAX_BALL_VECTORS = 10;

Pong::Pong(SSD1306Device* _oled) {
  oled = _oled;
  // seed random number with value from the analog pin
  srand(analogRead(MIDDLE_BUTTON));
  // everything is done on columns so if we set vertical memory address mode
  // we get a sizeable speed boost
  oled->setMemoryAddressingMode(1);

  randomBallVector();
}

void Pong::run() {
  setupPlayArea();

  while (true) {
    updateGame();
    updateScreen();
  }
}

void Pong::setupPlayArea() {
  writeScoreToScreen(true);
  writeScoreToScreen(false);
}

void Pong::updateGame() {
  clearGame();

  if (!checkForScore()) {
    checkForCollision();
    checkForPause();

    moveBall();
    movePlayer();
    moveEnemy();
    // delay(1000);
  } else {
    reset(false);
  }
}

// clear the only lines we care about: the ones with the paddle and ball
void Pong::clearGame() {
  updateLines(PLAYER_X, 0, 1);
  updateLines(ENEMY_X, 0, 1);
  // In memory addressing mode 1, automatic cursor incrementation doesn't
  // respect the frame buffer boundary (since the SSD1306 has no idea we're using
  // a 32x128 pixel display). This means writeToScreen(x, y, z) where z is greater
  // than 1 actually writes to the memory buffer. this was very hard to catch due
  // to double buffering. It actually gives a nice effect to the ball though,
  // so I'm keeping it
  updateLines(prevBallPos.x, 0, 2);
}

bool Pong::checkForScore() {
  if (ballPos.x < PLAYER_X) {
    enemyScore = min(999, enemyScore+1);
    writeScoreToScreen(false);
    return true;
  }

  if (ballPos.x > ENEMY_X) {
    playerScore = min(999, playerScore+1);
    writeScoreToScreen(true);
    return true;
  }

  return false;
}

void Pong::writeScoreToScreen(bool player) {
  // the score region on the screen doesn't update in the normal update flow
  // it only updates when the score changes or if the screen is cleared

  // do it twice for double bufferino
  // you ever think about how adding "-erino" to the end of words is the millenial
  // equivalent of 'okeydokey' or 'yessireebob'
  for (uint8_t i = 0; i < 2; i++) {
    // oled::print expects memory mode 0
    oled->setMemoryAddressingMode(0);
    if (player) {
      // hacks to get the player's score to be left aligned
      oled->setCursor(((playerScore >= 10) ? 0 : FONT_WIDTH) + ((playerScore >= 100) ? 0 : FONT_WIDTH), 0);

      oled->print(playerScore);
    } else {
      oled->setCursor(TOTAL_WIDTH-FONT_WIDTH*3, 0);

      oled->print(enemyScore);
    }
    oled->setMemoryAddressingMode(1);

    // switching frames only works in memory addressing mode 0. We are in mode 1
    // here but in tink4koled internally it's just a flag, so that doesn't matter
    oled->switchFrame();
  }
}

void Pong::moveBall() {
  // used later for clearing double buffer
  prevBallPos = { ballPos.x, ballPos.y };
  ballPos.x += ballVector.x;
  ballPos.y += ballVector.y;
}

void Pong::movePlayer() {
  // check the bounds
  if (playerPos < (TOTAL_HEIGHT - PADDLE_LENGTH)) {
    playerPos += digitalRead(LEFT_BUTTON) == LOW ? 1 : 0;
  }

  if (playerPos > 0){
    playerPos -= digitalRead(RIGHT_BUTTON) == LOW ? 1 : 0;
  }
}

void Pong::checkForPause() {
  if (digitalRead(MIDDLE_BUTTON) == LOW) {
    oled->setMemoryAddressingMode(0);
    oled->clear();
    oled->setCursor(40, 1);
    oled->print("PAUSED");
    oled->switchFrame();

    while(!digitalRead(MIDDLE_BUTTON) == LOW){
      delay(1);
    }

    oled->clear();
    oled->switchFrame();
    oled->clear();

    oled->setMemoryAddressingMode(1);
    setupPlayArea();
  }
}

void Pong::moveEnemy() {
  // enemy doesn't move until the ball crosses into their half
  // otherwise it's incredibly difficult to score
  if (ballPos.x >= TOTAL_WIDTH/2) {
    // basically just moving the paddle if the ball isn't directly in front of it
    // AVR controllers generally don't have hardware support for division, but
    // PADDLE_LENGTH is a const so the compiler should figure it out
    // >= and <= makes the enemy hit less line drives - it hits too many
    enemyPos += (ballPos.y >= enemyPos+(PADDLE_LENGTH / 2)) && enemyPos < (TOTAL_HEIGHT-PADDLE_LENGTH) ? 1 : 0;
    enemyPos -= (ballPos.y <= (int8_t)(enemyPos+(PADDLE_LENGTH / 2))) && enemyPos > 0 ? 1 : 0;
  }
}

// used for serves
// we want soft serves so we cherry-pick our numbers
void Pong::randomBallVector() {
  // 3 or 6
  uint8_t vector = ((rand() & 1) + 1) * 3;
  newBallVector(vector, rand() & 1);
}

void Pong::newBallVector(uint8_t index, bool reverseX) {
  index = index % MAX_BALL_VECTORS;
  uint8_t x = reverseX ? -1 : 1;

  switch(index) {
    case 0:
      ballVector = { x, -2};
      break;
    case 1:
    case 2:
    case 3:
      ballVector = { x, -1};
      break;
    case 4:
    case 5:
      ballVector = { x*2, 0 };
      break;
    case 6:
    case 7:
    case 8:
      ballVector = { x, 1 };
      break;
    case 9:
      ballVector = { x, 2 };
      break;
  }
}

void Pong::checkForCollision() {
  // checking that you are within the paddle
  // also allowing last-minute saves with the || there
  if ((ballPos.x == PLAYER_X+1 || ballPos.x == PLAYER_X) && (ballPos.y>=playerPos && ballPos.y<=playerPos+PADDLE_LENGTH)) {
    uint8_t speed = ballPos.y - playerPos + 1;
    newBallVector(speed, false);
  } else if ((ballPos.x+1 == ENEMY_X-1 || ballPos.x+1 == ENEMY_X) && (ballPos.y>=enemyPos && ballPos.y<=enemyPos+PADDLE_LENGTH)) {
    uint8_t speed = ballPos.y - enemyPos + 1;
    newBallVector(speed, true);
  }

  // if the ball hits the top or bottom, reverse its y direction
  if ((ballPos.y + ballVector.y) > (TOTAL_HEIGHT-2) || ballPos.y + ballVector.y < 0) {
    ballVector.y = -ballVector.y;
  }
}

void Pong::reset(bool hard) {
  prevBallPos = { ballPos.x, ballPos.y };
  // once again, no division, but these are consts
  ballPos = { TOTAL_WIDTH / 2, TOTAL_HEIGHT / 2 };
  randomBallVector();

  // unused as of now
  if(hard) {
    playerScore = 0;
    enemyScore = 0;
  }
}

// SSD1306 needs to be talked to in chunks of 8 pixels by column.
// we forego the library to write direct to the screen - bitmap writing may have
// worked too but would be a little more awkward
void Pong::updateLines(uint8_t x, uint32_t line, uint8_t numLines){
  oled->setCursor(x, 0);
  oled->startData();
  for (uint8_t j = 0; j < numLines; j++) {
    for(uint8_t i = 0; i < 4; i++) {
      uint8_t data = 0xff & (line >> (i * 8));
      oled->sendData(data);
    }
  }
  oled->endData();
}

// edge case when ball is on paddle line - looks ok, could fix though
void Pong::updateScreen() {
  uint32_t playerLine = ((uint32_t)0xff) << playerPos;
  uint32_t enemyLine = ((uint32_t)0xff) << enemyPos;
  uint32_t ballLine = ((uint32_t)0x3) << ballPos.y;

  // fun one: in mem mode 0, you can be on the second page. meaning the "hack"
  // where writing two successive lines updates the memory buffer actually
  // updates the line to the right in the memory buffer. whoops
  // uint32_t scoreLine = 0xaaaaaaaa;

  // remember that memory addressing mode bug? it happens here too. it ends up
  // looking more like a real pong ball though so I'm leaving it
  updateLines(ballPos.x, ballLine, 2);
  updateLines(PLAYER_X, playerLine, 1);
  updateLines(ENEMY_X, enemyLine, 1);
  // updateLines(TOTAL_WIDTH / 2, scoreLine, 1);

  oled->switchFrame();
}
