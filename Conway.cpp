#include "settings.h"
#include "Conway.h"

// for a random board
// static uint8_t Conway::board[BOARD_ROW_SIZE] = {
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
//   rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
// };

// test pattern
// static uint8_t Conway::board[BOARD_ROW_SIZE] = {
//   0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//
//   0b10000000, 0b01000000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000001,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
//   0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
// };

// glider
// NOTE: These arrays are BIG ENDIAN. The very first bit you see is the 0x0 pixel in the top left corner of the screen.
// rotate the bytes 90 degrees clockwise to visualize how they map to the screen
static uint8_t Conway::board[BOARD_ROW_SIZE] = {
  // first 8 rows
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000010, 0b00000000,
  0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b00000011, 0b00000100,
  0b00000000, 0b00001100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000011, 0b00000011, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  // second 8 rows
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b11000000, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b11100000, 0b00010000, 0b00001000, 0b00001000, 0b01000000,
  0b00010000, 0b11100000, 0b01000000, 0b00000000, 0b00000000, 0b10000000, 0b10000000, 0b01000000,
  0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
};

static uint8_t Conway::new_board[BOARD_ROW_SIZE] = { 0 };

Conway::Conway(SSD1306Device* _oled) {
  oled = _oled;
}

void Conway::run() {
  updateScreen();

  while (true) {
    updateGame();
    updateScreen();
  }
}

void Conway::updateGame() {
  for (uint8_t x = 0; x < BOARD_X; x++) {
    for (uint8_t y = 0; y < BOARD_Y; y++) {
      updateCell({x, y});
    }
  }

  updateBoard();
}

void Conway::updateBoard() {
  for (uint8_t row = 0; row < BOARD_ROW_SIZE; row++) {
    board[row] = new_board[row];
  }
}

void Conway::updateCell(UPosition position) {
  uint8_t livingNeighbors = getLivingNeighbors(position);

  if (livingNeighbors < 2 || livingNeighbors > 3) {
    setCell(position, 0);
  } else if (livingNeighbors == 3) {
    setCell(position, 1);
  } else {
    setCell(position, getCell(position));
  }
}

uint8_t Conway::getLivingNeighbors(UPosition position) {
  const Position positionDeltas[] = {
    {-1, -1}, {0,-1},  {1,-1},
    {-1,  0}, /*cell*/ {1, 0},
    {-1,  1}, {0, 1},  {1, 1},
  };

  const uint8_t NUM_DELTAS = 8;

  uint8_t livingNeighbors = 0;
  for (uint8_t i = 0; i < NUM_DELTAS; i++) {
    UPosition newPosition = { position.x + positionDeltas[i].x, position.y + positionDeltas[i].y };
    livingNeighbors += getCell(newPosition);

    // after 4 neighbors the outcome is always the same, so we can just abort
    if (livingNeighbors > 3) {
      break;
    }
  }

  return livingNeighbors;
}

uint8_t Conway::getCell(UPosition position) {
  // uint's don't have negative values, so you don't have to check < 0
  if (position.x >= BOARD_X || position.y >= BOARD_Y) {
    return 0;
  }

  uint16_t boundaryPosition = positionToRowIndex(position);
  uint8_t yValueInsideRow = position.y % 8;

  return (board[boundaryPosition] & (1 << (7 - yValueInsideRow))) >> (7 - yValueInsideRow);
}

uint16_t Conway::positionToRowIndex(UPosition position) {
  return (position.x + (position.y >> 3) * BOARD_X);
}

void Conway::setCell(UPosition position, uint8_t value) {
  uint16_t boundaryPosition = positionToRowIndex(position);
  uint8_t yValueInsideRow = position.y % 8;

  uint8_t row = new_board[boundaryPosition];
  row = (row & ~(1 << (7 - yValueInsideRow))) | (value << (7 - yValueInsideRow));

  new_board[boundaryPosition] = row;
}

void Conway::updateScreen() {
  // oled->setCursor(0,0);
  oled->startData();
  for (uint8_t y = 0; y < (BOARD_Y >> 2); y++) {
    for(uint8_t x = 0; x < BOARD_X; x++){
      uint8_t row = board[x + (y >> 1) * BOARD_X];

      bool bottomOfData = (y % 2) == 0;

      if (bottomOfData) {
        row = row >> 4;
      }

      row = (row & 1) << 7 | (row & 1) << 6 | (row & 2) << 4 | (row & 2) << 3 | (row & 4) << 1 | (row & 4) << 0 | (row & 8) >> 2 | (row & 8) >> 3;

      oled->sendData(row);
      oled->sendData(row);
    }
  }
  oled->endData();

  oled->switchFrame();
}
