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
// mostly to pull out the magic number 9 and give it a name
const uint8_t MAX_BALL_VECTORS = 9;

Pong::Pong(SSD1306Device* _oled) {
  oled = _oled;
  // seed random number with value from the analog pin
  srand(analogRead(MIDDLE_BUTTON));
  // everything is done on columns so if we set vertical memory address mode
  // we get a sizeable speed boost
  oled->setMemoryAddressingMode(1);
  // initialize random ball vector with rand() and analogRead for good measure
  newBallVector(rand() + analogRead(MIDDLE_BUTTON), rand() & 1);
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

// clear the only lines we care about: the ones with the paddle and ball
void Pong::clearScreen() {
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
  // the score region on the screen doesn't update in the normal update flow
  // it only updates when the score changes or if the screen is cleared

  // do it twice for double bufferino
  // you ever think about how adding "-erino" to the end of words is the millenial
  // equivalent of 'okeydokey' or 'yessireebob'
  for (uint8_t i = 0; i < 2; i++) {
    // clear score zones
    if (player) {
      // oled::print expects memory mode 0
      oled->setMemoryAddressingMode(0);
      // hacks to get the player's score to be left aligned
      oled->setCursor(((playerScore >= 10) ? 0 : FONT_WIDTH) + ((playerScore >= 100) ? 0 : FONT_WIDTH), 0);

      oled->print(playerScore);
      oled->setMemoryAddressingMode(1);
    } else {

      oled->setMemoryAddressingMode(0);
      oled->setCursor(TOTAL_WIDTH-FONT_WIDTH*3, 0);

      oled->print(enemyScore);
      oled->setMemoryAddressingMode(1);
    }

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
  if (playerPos < (TOTAL_HEIGHT - PADDLE_LENGTH)) {
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
      delay(1);
    }

    oled->clear();
    setupPlayArea();
  }
}

// +4 to get to the middle of the paddle
void Pong::moveEnemy() {
  // enemy doesn't move until the ball crosses into their half
  // otherwise it's incredibly difficult to score
  if (ballPos.x >= 64) {
    // basically just moving the paddle if the ball isn't directly in front of it
    // AVR controllers generally don't have hardware support for division, but
    // PADDLE_LENGTH is a const so the compiler should figure it out
    enemyPos += (ballPos.y > enemyPos+(PADDLE_LENGTH / 2)) && enemyPos < (TOTAL_HEIGHT-PADDLE_LENGTH) ? 1 : 0;
    enemyPos -= (ballPos.y < (int8_t)(enemyPos+(PADDLE_LENGTH / 2))) && enemyPos > 0 ? 1 : 0;
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

  if ((ballPos.y + ballVector.y) > (TOTAL_HEIGHT-2) || ballPos.y + ballVector.y < 0) {
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
