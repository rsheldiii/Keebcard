#include "settings.h"
#include "Conway.h"

static uint8_t Conway::board[BOARD_ROW_SIZE] = {
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
  rand(), rand(), rand(), rand(), rand(), rand(), rand(), rand(),
};

static uint8_t Conway::new_board[BOARD_ROW_SIZE] = { 0 };


Conway::Conway(SSD1306Device* _oled) {
  oled = _oled;
}

void Conway::run() {
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
  const uint8_t NUM_DELTAS = 8;
  const UPosition positionDeltas[NUM_DELTAS] = {
    {-1, -1}, {0,-1},  {1,-1},
    {-1,  0}, /*cell*/ {1, 0},
    {-1,  1}, {0, 1},  {1, 1},
  };

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

uint8_t Conway::positionToRowIndex(UPosition position) {
  return (position.x + (position.y / 8) * BOARD_X);
}

uint8_t Conway::getCell(UPosition position) {
  // uint's don't have negative values, so you don't have to check < 0
  if (position.x > BOARD_X || position.y > BOARD_Y) {
    return 0;
  }

  uint16_t boundaryPosition = positionToRowIndex(position);
  uint8_t yValueInsideRow = position.y % 8;

  return (board[boundaryPosition] & (1 << yValueInsideRow)) >> yValueInsideRow;
}

void Conway::setCell(UPosition position, uint8_t value) {
  uint16_t boundaryPosition = positionToRowIndex(position);
  uint8_t yValueInsideRow = position.y % 8;

  uint8_t row = new_board[boundaryPosition];
  row = (row & ~(1 << yValueInsideRow)) | (value << yValueInsideRow);

  new_board[boundaryPosition] = row;
}

void Conway::updateScreen() {
  // oled->setCursor(0,0);
  oled->startData();
  for (uint8_t y = 0; y < (BOARD_Y >> 2); y++) {
    for(uint8_t x = 0; x < BOARD_X; x++){
      uint8_t row = board[x + (y >> 1) * BOARD_X];

      bool bottomOfData = y % 2 == 1;

      if (bottomOfData) {
        row = row >> 4;
      }

      row = (row & 8) << 4 | (row & 8) << 3 | (row & 4) << 3 | (row & 4) << 2 | (row & 2) << 2 | (row & 2) << 1 | (row & 1) << 1 | (row & 1);

      oled->sendData(row);
      oled->sendData(row);
    }
  }
  oled->endData();

  oled->switchFrame();
}
