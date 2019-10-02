#include <stdint.h>
#include <Tiny4kOLED_common.h>

typedef struct {
  uint8_t x;
  uint8_t y;
} UPosition;

typedef struct {
  int8_t x;
  int8_t y;
} Position;


class Pong {
  SSD1306Device* oled;
  uint8_t enemyPos = 24;
  uint8_t playerPos = 24;

  uint16_t enemyScore = 0;
  uint16_t playerScore = 0;

  UPosition prevBallPos = { 64, 16 };
  UPosition ballPos = { 64, 16 };
  Position ballVector = { 1, 1 };

  public:
    Pong(SSD1306Device* _oled);
    void run(void);
    void updateGame(void);
  private:
    bool checkForScore(void);
    void checkForCollision(void);
    void checkForPause(void);
    void clearScreen(void);
    void moveBall(void);
    void moveEnemy(void);
    void movePlayer(void);
    void newBallVector(uint8_t index, bool reverseX);
    void reset(bool hard);
    void setupPlayArea(void);
    void updateLines(uint8_t x, uint32_t line, uint8_t numLines);
    void updateScreen(void);
    void writeScoreToScreen(bool player);
};
