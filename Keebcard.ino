#include <TinyWireM.h>
#include <Tiny4kOLED.h>


// PROGRAM DEFINES =============================================================

// uncomment whichever one you want
// #define BUSINESS_CARD
// #define TETRIS
// #define CONWAY
 // #define PONG
#define SNAKE

// cut down on features since digispark has a big bootloader
// #define DIGISPARK


#ifdef TETRIS
#include "Tetris.h"
#elif defined(CONWAY)
#include "Conway.h"
#elif defined(PONG)
#include "Pong.h"
#elif defined(BUSINESS_CARD)
#include "BusinessCard.h"
#elif defined(SNAKE)
#include "Snake.h"
#endif

// UNIVERSAL DEFINES ===========================================================

#define LEFT_BUTTON 3
#define RIGHT_BUTTON 4
#define MIDDLE_BUTTON 1

void universal_setup() {

  // flip screen horizontally
  // oled.setSegmentRemap(0xA0);
  // flip screen vertically
  // oled.setComOutputDirection(0xC0);

  oled.begin();

  oled.setFont(FONT8X16);
  // Switch the half of RAM that we are writing to, to be the half that is non currently displayed
  oled.switchRenderFrame();

  // clear both buffers of any dead squirrels
  oled.clear();
  oled.switchFrame();
  oled.clear();

  // Turn on the display
  oled.on();

  // seed random number with value from the analog pin
  srand(analogRead(MIDDLE_BUTTON));

  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);
  pinMode(MIDDLE_BUTTON, INPUT);
}


void setup() {
  universal_setup();

#ifdef TETRIS
  Tetris tetris(&oled);
  uint8_t score = tetris.run();
  gameOver(score);
#elif defined(CONWAY)
  Conway conway(&oled);
  conway.run();
#elif defined(BUSINESS_CARD)
  businessCard(&oled);
#elif defined(PONG)
  oled.setCursor(8, 1);
  oled.print(F("PLAYER 1 START"));
  oled.switchFrame();
  delay(800);
  oled.switchFrame();
  oled.clear();
  oled.setMemoryAddressingMode(1);
  Pong pong(&oled);
  pong.run();
#elif defined(SNAKE)
  oled.setMemoryAddressingMode(0);
  oled.setCursor(8, 1);
  oled.print(F("NOKIA OS"));
  oled.switchFrame();
  delay(800);
  oled.switchFrame();
  oled.clear();
  Snake snake(&oled);
  snake.run();
#endif
}

void loop() {
  // nothing right now
}

void gameOver(uint8_t score) {
  oled.clear();
  oled.switchFrame();
  oled.clear();

  const char gameOver[] = "Game Over";

  for (uint8_t i = 0; i < 9; i++) {
    oled.setCursor(i * 8, 4);
    oled.write(gameOver[i]);
  }

#ifndef DIGISPARK
  writeNum(score, 6);
#endif
}

void writeNum(uint8_t num, uint8_t offset) {
  uint8_t index = 64;
  do {
    uint8_t digit = num % 10;
    oled.setCursor(index, offset);

    oled.write(digit + 48);
    num = num / 10;
    index -= 8;
  } while (num);
}
