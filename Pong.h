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
    void update(void);
  private:
    void newBallVector(uint8_t index, bool reverseX);
    void clearScreen(void);
    void updateLines(uint8_t x, uint32_t line, uint8_t numLines);
    void reset(bool hard);
    void updateScreen(void);
    bool checkForScore(void);
    void writeScoreToScreen(bool player);
    void moveBall(void);
    void movePlayer(void);
    void moveEnemy(void);
    void checkForCollision(void);
    void checkForPause(void);
    void setupPlayArea(void);
};
