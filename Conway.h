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

typedef struct BitRow  {
  bool a : 1;
  bool b : 1;
  bool c : 1;
  bool d : 1;
  bool e : 1;
  bool f : 1;
  bool g : 1;
  bool h : 1;
} BitRow;

//--- CONSTANTS
const uint8_t BOARD_X = 64;
const uint8_t BOARD_Y = 16;
const uint16_t BOARD_SIZE = BOARD_X * BOARD_Y;
const uint16_t BOARD_ROW_SIZE = BOARD_SIZE / 8;


class Conway {
  SSD1306Device* oled;
  static uint8_t board[BOARD_SIZE>>3];
  static uint8_t new_board[BOARD_SIZE>>3];

  public:
    Conway(SSD1306Device* _oled);
    void run(void);
    void updateGame(void);
    void updateBoard(void);
    void updateScreen(void);
    void updateCell(UPosition position);
    uint8_t getLivingNeighbors(UPosition position);
    uint8_t getCell(UPosition position);
    uint8_t positionToRowIndex(UPosition position);
    void setCell(UPosition position, uint8_t value);
};
