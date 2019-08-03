#include "settings.h"
#include "Pong.h"


#define GOAL_X 10
#define PLAYER_X 20 + GOAL_X
#define ENEMY_X 108 - GOAL_X

#define PADDLE_LENGTH 8
#define SCREEN_Y 32
#define MAX_BALL_VECTORS 9

Pong::Pong(SSD1306Device* oleda) {
  oled = oleda;
  srand(millis());
  // everything is done on columns so if we set vertical memory address mode
  // we get a sizeable speed boost
  oled->setMemoryAddressingMode(1);
  // oled->setPageAddress(0b0, 0b111);

  // oled->fill(0);
  // oled->switchFrame();
  // oled->fill(0);
  // oled->setCursor(1,1);
  // oled->print(F("Pong:"));
  // oled->switchFrame();

  newBallVector(rand(), false);
}

void Pong::run() {
  setupPlayArea();

  // oled->switchFrame();

  while (true) {
    update();
    updateScreen();
  }
}

void Pong::setupPlayArea() {
  // oled->fill(0xff);
  // oled->fill(0);
  // oled->switchFrame();
  // updateLines(0,0xffffffff,255);
  writeScoreToScreen(true);
  writeScoreToScreen(false);
  // oled->print("Pong");
  // oled->print(playerScore);
  //
  // oled->setCursor(107, 2);
  // oled->print(enemyScore);
  // updateLines(0, 0xffffffff, 16);
  // updateLines(112, 0xffffffff, 16);
  //
  // oled->switchFrame();
  //
  // updateLines(0, 0xffffffff, 16);
  // updateLines(112, 0xffffffff, 16);
}

void Pong::update() {
  clearScreen();

  if (!checkForScore()) {
    checkforCollision();

    moveBall();
    movePlayer();
    moveEnemy();
  } else {
    reset(false);
  }
}

bool Pong::checkForScore() {
  if (ballPos.x < PLAYER_X) {
    enemyScore++;
    writeScoreToScreen(false);
    return true;
  }

  if (ballPos.x > ENEMY_X) {
    playerScore++;
    writeScoreToScreen(true);
    return true;
  }

  return false;
}

void Pong::writeScoreToScreen(bool player) {
  // do it twice for double bufferino
  for (uint8_t i = 0; i < 2; i++) {
    // clear score zones
    if (player) {
      // updateLines is meant to be run in memory mode 1 which is why this whole function is funky
      updateLines(0,0,16); // TODO don't have to clear all 16

      oled->setMemoryAddressingMode(0);
      oled->setCursor(8, 0);
      oled->print(playerScore);
      oled->setMemoryAddressingMode(1);

    } else {
      updateLines(104,0,16);

      oled->setMemoryAddressingMode(0);
      oled->setCursor(112, 0);
      oled->print(enemyScore);
      oled->setMemoryAddressingMode(1);
    }

    // I think we want to not switch once so we're back on the same frame for double buffering purposes
    oled->switchFrame();
  }
}

void Pong::moveBall() {
  // saved later for clearing double buffer
  prevBallPos = { ballPos.x, ballPos.y };
  ballPos.x += ballVector.x;
  ballPos.y += ballVector.y;
}

void Pong::movePlayer() {
  if (playerPos < (SCREEN_Y - PADDLE_LENGTH)) {
    playerPos += digitalRead(LEFT_ARROW) == LOW ? 1 : 0;
  }

  if (playerPos > 0){
    playerPos -= digitalRead(UP_ARROW) == LOW ? 1 : 0;
  }
}

// +4 to get to the middle of the paddle
void Pong::moveEnemy() {
  if (ballPos.x >= 64) {
    enemyPos += (ballPos.y > enemyPos+4) && enemyPos < (SCREEN_Y-PADDLE_LENGTH) ? 1 : 0;
    enemyPos -= (ballPos.y < (int8_t)(enemyPos+4)) && enemyPos > 0 ? 1 : 0;
  }
}

void Pong::newBallVector(uint8_t index, bool reverseX) {
  index = index % MAX_BALL_VECTORS;
  uint8_t x = reverseX ? -1 : 1;

  switch(index) {
    case 0:
    case 1:
    case 2:
      ballVector = { x, -2};
      break;
    case 3:
      ballVector = { x, -1};
      break;
    case 4:
      ballVector = { x*2, 0 };
      break;
    case 5:
      ballVector = { x, 1 };
      break;
    case 6:
    case 7:
    case 8:
      ballVector = { x, 2 };
      break;
  }
}

void Pong::checkforCollision() {
  // checking that you are within the paddle
  // also allowing last-minute saves with the || there
  if ((ballPos.x == PLAYER_X+1 || ballPos.x == PLAYER_X) && (ballPos.y>=playerPos && ballPos.y<=playerPos+PADDLE_LENGTH)) {
    uint8_t speed = ballPos.y - playerPos + 1;
    // TODO one of these 4's doesn't happen
    newBallVector(speed, false);
  } else if ((ballPos.x+1 == ENEMY_X-1 || ballPos.x+1 == ENEMY_X) && (ballPos.y>=enemyPos && ballPos.y<=enemyPos+PADDLE_LENGTH)) {
    uint8_t speed = ballPos.y - enemyPos + 1;
    // TODO one of these 4's doesn't happen
    newBallVector(speed, true);
  }

  if ((ballPos.y + ballVector.y) > (SCREEN_Y-2) || ballPos.y + ballVector.y < 0) {
    ballVector.y = -ballVector.y;
  }
}

void Pong::reset(bool hard) {
  prevBallPos = { ballPos.x, ballPos.y };
  ballPos = { 64, 16 };
  ballVector = { 1, 1 };

  if(hard) {
    playerScore = 0;
    enemyScore = 0;
  }
}

// numLines is an optimization for ball
void Pong::updateLines(uint8_t x, uint32_t line, uint8_t numLines){
  oled->setCursor(x, 0);
  oled->startData();
  for (uint8_t j = 0; j < numLines; j++) {
    for(uint8_t i = 0; i < 4; i++) {
      uint8_t data = 0xff & (line >> (i * 8));
      oled->sendData( data);
    }
  }
  oled->endData();
}

void Pong::clearScreen() {
  updateLines(PLAYER_X, 0, 1);
  updateLines(ENEMY_X, 0, 1);
  updateLines(prevBallPos.x, 0, 2);
}

// edge case when ball is on paddle line - looks ok, could fix though
void Pong::updateScreen() {


  uint32_t playerLine = ((uint32_t)0xff) << playerPos;
  uint32_t enemyLine = ((uint32_t)0xff) << enemyPos;
  uint32_t ballLine = ((uint32_t)0x3) << ballPos.y;

  updateLines(ballPos.x, ballLine, 2);
  updateLines(PLAYER_X, playerLine, 1);
  updateLines(ENEMY_X, enemyLine, 1);

  oled->switchFrame();
}
