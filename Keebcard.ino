 #include <TinyWireM.h>
#include <Tiny4kOLED.h>
// #include <Entropy.h>
#include "settings.h"


// PROGRAM DEFINES =============================================================

// uncomment whichever one you want
// #define BUSINESS_CARD
#define TETRIS
// #define CONWAY
// #define PONG
// #define SNAKE

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
  // it doesn't vary all that much, but it helps
  srand(analogRead(MIDDLE_BUTTON));
  // here's the good stuff, Entropy based off clock jitter
  // Entropy.initialize();

  pinMode(LEFT_BUTTON, INPUT);
  pinMode(RIGHT_BUTTON, INPUT);
  pinMode(MIDDLE_BUTTON, INPUT);

  #ifdef SNAKE
    oled.setMemoryAddressingMode(0); // TODO prolly don't need this
    // no double buffering for now. very little to update so it runs fast anyways
    oled.switchRenderFrame();
  #endif
}


void setup() {
  universal_setup();
  setup_game();
}

void setup_game(){
  #ifdef TETRIS
    GIMSK = 0b00100000;    // turns on pin change interrupts
    PCMSK = 0b00011010;    // turn on interrupts on pins PB0, PB1, &amp;amp; PB4
    sei();                 // enables interrupts

    Tetris tetris(&oled);
    gameOver(tetris.run());
    //oled.switchFrame();
  #elif defined(CONWAY)
    oled.setMemoryAddressingMode(0); // TODO prolly don't need this
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
    oled.setCursor(8, 1);
    oled.print(F("NOKIA OS"));
    // oled.switchFrame();
    delay(800);
    // oled.switchFrame();
    oled.clear();
    Snake snake(&oled);
    gameOver(snake.run());
  #endif
}

void loop() {
  if (digitalRead(MIDDLE_BUTTON) == LOW) {
    // oled.clear();
    // oled.switchFrame();
    // oled.clear();
    // setup_game();
  }
}

void gameOver(uint32_t score) {
    oled.clear();
    oled.setCursor(0,0);
    oled.print(F("Game Over!"));
    oled.setCursor(0,2);
    oled.print(F("Score: "));
    oled.print(score);
}
