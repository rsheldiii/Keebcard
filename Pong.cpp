#include "settings.h"
#include "Pong.h"

const uint8_t FONT_WIDTH = 8;
const uint8_t TOTAL_WIDTH = 128;

const uint8_t PLAYER_X = 30;
const uint8_t ENEMY_X = 98;

const uint8_t PADDLE_LENGTH = 8;
const uint8_t SCREEN_Y = 32;
const uint8_t MAX_BALL_VECTORS = 9;

Pong::Pong(SSD1306Device* oleda) {
  oled = oleda;
  srand(analogRead(MIDDLE_BUTTON));
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

  newBallVector(rand() + analogRead(MIDDLE_BUTTON), rand() & 1);
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

// new info: in memory addressing mode 1, automatic cursor incrememntation doesn't
// work as expected. in mode 0 the cursor moves horizontally first, and at the end of
// the screen will begin to fill the memory buffer. in mode 0, writing >4 lines
// without setting the cursor again will transgress into the memory buffer.
// Double buffering causes this to be hard to catch
void Pong::writeScoreToScreen(bool player) {
  // do it twice for double bufferino
  // you ever think about how adding "-erino" to the end of words is the millenial
  // equivalent of 'okeydokey' or 'yessireebob'
  for (uint8_t i = 0; i < 2; i++) {
    // clear score zones
    if (player) {
      oled->setMemoryAddressingMode(0);
      oled->setCursor(0 + ((playerScore >= 10) ? 0 : FONT_WIDTH) + ((playerScore >= 100) ? 0 : FONT_WIDTH), 0);

      oled->print(playerScore);
      oled->setMemoryAddressingMode(1);
    } else {

      oled->setMemoryAddressingMode(0);
      oled->setCursor(TOTAL_WIDTH-FONT_WIDTH*3, 0);

      oled->print(enemyScore);
      oled->setMemoryAddressingMode(1);
    }

    // switchFrame works in memory addressing mode 0. We are in mode 1 when we switch but it's just a flag, so that doesn't matter
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
    playerPos += digitalRead(LEFT_BUTTON) == LOW ? 1 : 0;
  }

  if (playerPos > 0){
    playerPos -= digitalRead(RIGHT_BUTTON) == LOW ? 1 : 0;
  }
}

void Pong::checkForPause() {
  if (digitalRead(MIDDLE_BUTTON) == LOW) {
    oled->clear();
    oled->setMemoryAddressingMode(0);
    oled->setCursor(40, 1);
    oled->print("PAUSED");
    oled->switchFrame();
    oled->setMemoryAddressingMode(1);

    while(!digitalRead(MIDDLE_BUTTON) == LOW){
      continue;
    }

    oled->clear();
    // lol
    writeScoreToScreen(true);
    writeScoreToScreen(false);
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
      // ballVector = { x, -2};
      // break;
    case 3:
      ballVector = { x, -1};
      break;
    case 4:
      ballVector = { x*2, 0 };
      break;
    case 5:
      // ballVector = { x, 1 };
      // break;
    case 6:
    case 7:
    case 8:
      ballVector = { x, 1 };
      break;
      // ballVector = { x, 2 };
      // break;
  }
}

void Pong::checkForCollision() {
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

  // remember that memory addressing mode bug? it happens here too. it ends up
  // looking more like a real pong ball though so I'm leaving it
  updateLines(ballPos.x, ballLine, 2);
  updateLines(PLAYER_X, playerLine, 1);
  updateLines(ENEMY_X, enemyLine, 1);

  oled->switchFrame();
}
