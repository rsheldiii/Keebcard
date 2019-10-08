#include <stdint.h>
//#include <Arduino.h>
#include <Tiny4kOLED_common.h>

typedef struct Position {
  uint8_t x;
  uint8_t y;
} Position;

enum Direction {LEFT = 0, UP = 1, RIGHT = 2, DOWN = 3 };

typedef struct QuadrupleDirection  {
  Direction firstDirection : 2;
  Direction secondDirection : 2;
  Direction thirdDirection : 2;
  Direction fourthDirection : 2;
} QuadrupleDirection;

class SnakePath {
  static QuadrupleDirection snakePath[200]; // the path the snake travels, starting at the head. 2 bits per for cardinal neighbors, a max of 1024 pixels * 2 bits / 8

  public:
    Direction get(uint8_t index);
    void set(uint8_t index, Direction direction);
};

class Snake {
  static SnakePath snakePath;

  // STARTS AT THE TAIL
  static uint8_t board[128]; // 16 x 64 pixel board, / 8 for uint8
  static Position headPosition; // position of head of snake
  static Position tailPosition; // position of tail of snake
  static Position foodPosition;

  static Direction direction; // direction of the snake
  static Direction lastDirection;

  static uint16_t score;
  static bool gameOver;
  static bool scoredThisTurn;
  static uint8_t time;
  static uint8_t debounce;
  static uint8_t linksToAdd;

  SSD1306Device* oled;


  public:
    Snake(SSD1306Device* _oled);
    void run(void);
  private:
    void main(void);
    void moveSnake(void);
    void renderScreen(void);
    void sendToGrid(Position position, bool value);
    void checkInputs(void);
    void updateTailPosition(void);
    void updateHeadPosition(void);
    void pushNewLink(void);
    void addDeltaToPosition(Position &pos, Direction delta);
    void checkGameOver(void);
    void renderRow(uint8_t row);
    void checkForScore(void);
    void setNewFoodPosition(void);
    bool checkForCollision(Position collisionPosition, bool includeHead);
};
