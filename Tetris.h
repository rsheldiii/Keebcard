#include <stdint.h>
//#include <Arduino.h>
#include <Tiny4kOLED_common.h>

#ifndef TETRIS
#define TETRIS

#define PIXELS_IN_SHAPE 4

//TODO union type relativePosition with 4 bits for signed x and 4 for signed y


typedef struct {
  int8_t x;
  int8_t y;
} Position;

typedef struct {
  Position pixels[PIXELS_IN_SHAPE];
} Shape;

// ----------------------------------------------------------------------------

class Tetris {
  SSD1306Device* oled;
  Position position;
  uint8_t score = 0;
  uint8_t buttonFlags = 0;
  static uint8_t board[32];
  static const Shape shapes[];
  static const uint8_t numShapes;
  // so we don't fetch from progmem every frame
  static Shape shape;
  // required to keep track of rotations
  static uint8_t shapeIndex;

	public:
    Tetris(SSD1306Device* _oled);
		uint8_t run(void);
  private:
    void main(void);
    void end(void);
    void checkForFullRows(void);
    void movePiece(void);
    void goLeft(void);
    void goRight(void);
    void advancePiece(void);
    void rotatePiece(void);
    void spawnNewPiece(void);
    void assignRandomShape(void);
    void assignShape(uint8_t index);
    bool checkCollision(Position delta);
    void addOrRemovePiece(bool add);
    void renderBoard(bool wholeScreen = false, bool removePiece = true);
    void checkInputs(void);
};

#endif
