#include <stdint.h>
//#include <Arduino.h>
#include <Tiny4kOLED_common.h>

#define PIXELS_IN_SHAPE 4

//TODO union type relativePosition with 4 bits for signed x and 4 for signed y


typedef struct {
  int8_t x;
  int8_t y;
} Position;

typedef struct {
  Position pixels[PIXELS_IN_SHAPE];
} Shape;

class Tetris {
  Position position;
  uint8_t score = 0;
  uint8_t frame = 0;
  uint8_t framesPerTick = 10;

  uint8_t buttonFlags = 0;
  uint32_t leftButton = 0;
  uint32_t rightButton = 0;
  uint32_t upButton = 0;


  SSD1306Device* oled;
  static uint8_t board[32];
  static const Shape shapes[];
  static const uint8_t numShapes;
  // so we don't fetch from progmem every frame
  static Shape shape;
  // required to keep track of rotations
  static uint8_t shapeIndex;
  // time of the current frame render
  static uint32_t frameTime;

	public:
    Tetris(SSD1306Device* _oled);
		uint8_t run(void);
  private:
    bool checkCollision(Position delta);
    void addOrRemovePiece(bool add);
    void advancePiece(void);
    void assignRandomShape(void);
    void assignShape(uint8_t index);
    void checkForFullRows(void);
    void checkInputs(bool unsetFlags = false);
    void end(void);
    void goLeft(void);
    void goRight(void);
    void main(void);
    void movePiece(bool moveDown);
    void renderBoard(bool wholeScreen = false, bool addPiece = true);
    void rotatePiece(void);
    void spawnNewPiece(void);
};
