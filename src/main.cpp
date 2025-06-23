#include <Arduino.h>
#include <BROSE9323.h>
#include <GameControllers.h>

#define WHITE 1
#define DEFAULT_INTERVAL 700

#define WIDTH 16   // screen width in pixels
#define HEIGHT 84  // screen height in pixels
#define PANEL_WIDTH 28

// shared pins between all controllers
#define LATCH_PIN 0
#define CLOCK_PIN 1
// individual data pin for each controller
#define DATA_PIN_0 2

BROSE9323 display(HEIGHT, WIDTH, PANEL_WIDTH);
GameControllers controllers;

// matrix for "S" shape
const char pieces_S_l[2][2][4] = {{{0, 0, 1, 1}, {0, 1, 1, 2}},
                                  {{0, 1, 1, 2}, {1, 1, 0, 0}}};
// matrix for "S" shape
const char pieces_S_r[2][2][4] = {{{1, 1, 0, 0}, {0, 1, 1, 2}},
                                  {{0, 1, 1, 2}, {0, 0, 1, 1}}};
// matrix for "L" shape
const char pieces_L_l[4][2][4] = {{{0, 0, 0, 1}, {0, 1, 2, 2}},
                                  {{0, 1, 2, 2}, {1, 1, 1, 0}},
                                  {{0, 1, 1, 1}, {0, 0, 1, 2}},
                                  {{0, 0, 1, 2}, {1, 0, 0, 0}}};
// matrix for "square" shape
const char pieces_Sq[1][2][4] = {{{0, 1, 0, 1}, {0, 0, 1, 1}}};
// matrix for "T" shape
const char pieces_T[4][2][4] = {{{0, 0, 1, 0}, {0, 1, 1, 2}},
                                {{0, 1, 1, 2}, {1, 0, 1, 1}},
                                {{1, 0, 1, 1}, {0, 1, 1, 2}},
                                {{0, 1, 1, 2}, {0, 0, 1, 0}}};
// matrix for "I" shape
const char pieces_l[2][2][4] = {{{0, 1, 2, 3}, {0, 0, 0, 0}},
                                {{0, 0, 0, 0}, {0, 1, 2, 3}}};

const short MARGIN_TOP = 0;
const short MARGIN_LEFT = 0;
const short SIZE = 2;  // 2x2 pixel grid
const short TYPES = 6;
const int MELODY_LENGTH = 10;
const int MELODY_NOTES[MELODY_LENGTH] = {262, 294, 330, 262};
const int MELODY_DURATIONS[MELODY_LENGTH] = {500, 500, 500, 500};

int click[] = {1047};
int click_duration[] = {100};
int erase[] = {2093};
int erase_duration[] = {100};
word currentType, rotation;
short pieceX, pieceY;
short prevPieceX, prevPieceY;  // Track previous piece position
short piece[2][4];
short prevPiece[2][4];  // Track previous piece data
int interval = DEFAULT_INTERVAL, score;
long timer, delayer;
boolean grid[WIDTH][HEIGHT / SIZE];  // Dynamic grid size based on display dimensions
boolean b1, b2, b3;
boolean gamePaused = false;  // Track pause state

void breakLine(short line) {
  for (short y = line; y >= 0; y--) {
    for (short x = 0; x < WIDTH; x++) {
      grid[x][y] = grid[x][y - 1];
    }
  }
  for (short x = 0; x < WIDTH; x++) {
    grid[x][0] = 0;
  }
  display.invertDisplay(true);
  delay(50);
  display.invertDisplay(false);
  score += 10;
}

void checkLines() {
  boolean full;
  for (short y = HEIGHT / SIZE - 1; y >= 0; y--) {
    full = true;
    for (short x = 0; x < WIDTH; x++) {
      full = full && grid[x][y];
    }
    if (full) {
      breakLine(y);
      y++;
    }
  }
}

void drawGrid() {
  for (short x = 0; x < WIDTH; x++)
    for (short y = 0; y < HEIGHT / SIZE; y++)
      if (grid[x][y])
        display.fillRect(MARGIN_LEFT + ((HEIGHT / SIZE - 1 - y) * 2), MARGIN_TOP + (x * 2), SIZE, SIZE, WHITE);
}

boolean nextHorizontalCollision(short piece[2][4], int amount) {
  for (short i = 0; i < 4; i++) {
    short newX = pieceX + piece[0][i] + amount;
    if (newX >= (WIDTH / SIZE) || newX < 0 || grid[newX][pieceY + piece[1][i]])
      return true;
  }
  return false;
}

boolean nextCollision() {
  for (short i = 0; i < 4; i++) {
    short y = pieceY + piece[1][i] + 1;
    short x = pieceX + piece[0][i];
    if (y >= (HEIGHT / SIZE) || x >= (WIDTH / SIZE) || grid[x][y])
      return true;
  }
  return false;
}

void copyPiece(short piece[2][4], short type, short rotation) {
  switch (type) {
    case 0:  // L_l
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_L_l[rotation][0][i];
        piece[1][i] = pieces_L_l[rotation][1][i];
      }
      break;
    case 1:  // S_l
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_S_l[rotation][0][i];
        piece[1][i] = pieces_S_l[rotation][1][i];
      }
      break;
    case 2:  // S_r
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_S_r[rotation][0][i];
        piece[1][i] = pieces_S_r[rotation][1][i];
      }
      break;
    case 3:  // Sq
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_Sq[0][0][i];
        piece[1][i] = pieces_Sq[0][1][i];
      }
      break;
    case 4:  // T
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_T[rotation][0][i];
        piece[1][i] = pieces_T[rotation][1][i];
      }
      break;
    case 5:  // l
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_l[rotation][0][i];
        piece[1][i] = pieces_l[rotation][1][i];
      }
      break;
  }
}

long getNextType() {
  return random(0, TYPES);
}

short getMaxRotation(short type) {
  if (type == 1 || type == 2 || type == 5)
    return 2;
  else if (type == 0 || type == 4)
    return 4;
  else if (type == 3)
    return 1;
  else
    return 0;
}

void generate() {
  randomSeed(millis());
  currentType = getNextType();
  if (currentType != 5)
    pieceX = random(WIDTH) / SIZE;
  else
    pieceX = random(WIDTH - 2) / SIZE;

  pieceY = 0;
  rotation = random(0, getMaxRotation(currentType) - 1);
  copyPiece(piece, currentType, rotation);
}

void drawPiece(short x, short y) {
  for (short i = 0; i < 4; i++)
    display.fillRect(MARGIN_LEFT + ((HEIGHT / SIZE - 1 - (y + piece[1][i])) * 2), MARGIN_TOP + ((x + piece[0][i]) * 2), SIZE, SIZE, WHITE);
}

boolean canRotate(short rotation) {
  short piece[2][4];
  copyPiece(piece, currentType, rotation);
  return !nextHorizontalCollision(piece, 0);
}

short getNumberLength(int n) {
  short counter = 1;
  while (n >= 10) {
    n /= 10;
    counter++;
  }
  return counter;
}

void refresh() {
  // Clear previous piece position using previous piece data
  for (short i = 0; i < 4; i++) {
    short x = prevPieceX + prevPiece[0][i];
    short y = prevPieceY + prevPiece[1][i];
    // Only clear if this position is not part of the grid
    if (x >= 0 && x < WIDTH / SIZE && y >= 0 && y < HEIGHT / SIZE && !grid[x][y]) {
      display.fillRect(MARGIN_LEFT + ((HEIGHT / SIZE - 1 - y) * 2), MARGIN_TOP + (x * 2), SIZE, SIZE, 0);
    }
  }

  // Draw grid (only the parts that are set)
  drawGrid();

  // Draw current piece (only if not paused)
  if (!gamePaused) {
    drawPiece(pieceX, pieceY);
  }

  // Draw pause indicator if paused
  if (gamePaused) {
    // Draw "PAUSE" text or indicator
    for (int i = 0; i < 5; i++) {
      display.fillRect(MARGIN_LEFT + (i * 4), MARGIN_TOP + 40, 2, 2, WHITE);
    }
  }

  // Update previous position and piece data
  prevPieceX = pieceX;
  prevPieceY = pieceY;
  for (short i = 0; i < 4; i++) {
    prevPiece[0][i] = piece[0][i];
    prevPiece[1][i] = piece[1][i];
  }

  display.display();
}

void clearDisplay() {
  display.fillScreen(1);
  display.display();
  delay(100);
  display.fillScreen(0);
  display.display();
}

void initializePieceData() {
  prevPieceX = pieceX;
  prevPieceY = pieceY;
  // Initialize previous piece data
  for (short i = 0; i < 4; i++) {
    prevPiece[0][i] = piece[0][i];
    prevPiece[1][i] = piece[1][i];
  }
}

void startNewGame() {
  // Generate new piece
  clearDisplay();
  generate();
  initializePieceData();
  timer = millis();
}

void resetGame() {
  // Clear the grid
  for (short x = 0; x < WIDTH; x++) {
    for (short y = 0; y < HEIGHT / SIZE; y++) {
      grid[x][y] = 0;
    }
  }

  // Reset game variables
  score = 0;
  interval = DEFAULT_INTERVAL;
  gamePaused = false;

  // Start new game
  startNewGame();
}

void setup() {
  // initialize shared pins
  controllers.init(LATCH_PIN, CLOCK_PIN);
  // activate first controller ans set the type to SNES
  controllers.setController(0, GameControllers::SNES, DATA_PIN_0);

  display.begin();
  delay(1000);

  randomSeed(analogRead(0));
  startNewGame();
}

void loop() {
  controllers.poll();

  // Handle pause mode
  if (controllers.down(0, GameControllers::START)) {
    gamePaused = !gamePaused;  // Toggle pause state
    delay(200);                // Debounce
  }

  // If paused, only handle SELECT for reset
  if (gamePaused) {
    if (controllers.down(0, GameControllers::SELECT)) {
      resetGame();
      delay(200);  // Debounce
    }
    return;  // Exit early, don't run game logic
  }

  if (millis() - timer > interval) {
    checkLines();
    refresh();
    if (nextCollision()) {
      for (short i = 0; i < 4; i++)
        grid[pieceX + piece[0][i]][pieceY + piece[1][i]] = 1;
      // Clear the piece trail since it's now part of the grid
      for (short i = 0; i < 4; i++) {
        short x = prevPieceX + prevPiece[0][i];
        short y = prevPieceY + prevPiece[1][i];
        if (x >= 0 && x < WIDTH / SIZE && y >= 0 && y < HEIGHT / SIZE && !grid[x][y]) {
          display.fillRect(MARGIN_LEFT + ((HEIGHT / SIZE - 1 - y) * 2), MARGIN_TOP + (x * 2), SIZE, SIZE, 0);
        }
      }
      generate();
    } else
      pieceY++;
    timer = millis();
  }

  // when "left" button is pressed
  if (controllers.down(0, GameControllers::LEFT)) {
    if (b1) {
      if (!nextHorizontalCollision(piece, -1)) {
        pieceX--;
        refresh();
      }
      b1 = false;
    }
  } else {
    b1 = true;
  }

  // when "right" button is pressed
  if (controllers.down(0, GameControllers::RIGHT)) {
    if (b2) {
      if (!nextHorizontalCollision(piece, 1)) {
        pieceX++;
        refresh();
      }
      b2 = false;
    }
  } else {
    b2 = true;
  }

  // when "down" button is pressed
  if (controllers.down(0, GameControllers::DOWN)) {
    interval = 20;
  } else {
    interval = DEFAULT_INTERVAL;
  }

  // when "change" button is pressed
  if (controllers.down(0, GameControllers::A)) {
    if (b3) {
      if (rotation == getMaxRotation(currentType) - 1 && canRotate(0)) {
        rotation = 0;
      } else if (canRotate(rotation + 1)) {
        rotation++;
      }
      copyPiece(piece, currentType, rotation);
      refresh();
      b3 = false;
      delayer = millis();
    }
  } else if (millis() - delayer > 50) {
    b3 = true;
  }
}