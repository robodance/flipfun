#include <Arduino.h>
#include "Wire.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#define WIDTH 16   // screen width in pixels
#define HEIGHT 84  // screen height in pixels
#define BUZZER 12  // buzzer connection port
#define left 3     // button connection ports
#define right 4
#define change 5
#define down 6

Adafruit_SSD1306 display(HEIGHT, WIDTH, &Wire, -1);

// matrix for "S" shape
const char pieces_S_l[2][2][4] = {{ {0, 0, 1, 1}, {0, 1, 1, 2} },
                                  { {0, 1, 1, 2}, {1, 1, 0, 0} }};
// matrix for "S" shape
const char pieces_S_r[2][2][4] = {{ {1, 1, 0, 0}, {0, 1, 1, 2} },
                                  { {0, 1, 1, 2}, {0, 0, 1, 1} }};
// matrix for "L" shape
const char pieces_L_l[4][2][4] = {{ {0, 0, 0, 1}, {0, 1, 2, 2} },
                                  { {0, 1, 2, 2}, {1, 1, 1, 0} },
                                  { {0, 1, 1, 1}, {0, 0, 1, 2} },
                                  { {0, 0, 1, 2}, {1, 0, 0, 0} }};
// matrix for "square" shape
const char pieces_Sq[1][2][4] = {{ {0, 1, 0, 1}, {0, 0, 1, 1} }};
// matrix for "T" shape
const char pieces_T[4][2][4] = {{ {0, 0, 1, 0}, {0, 1, 1, 2} },
                                { {0, 1, 1, 2}, {1, 0, 1, 1} },
                                { {1, 0, 1, 1}, {0, 1, 1, 2} },
                                { {0, 1, 1, 2}, {0, 0, 1, 0} }};
// matrix for "I" shape
const char pieces_l[2][2][4] = {{ {0, 1, 2, 3}, {0, 0, 0, 0} },
                                { {0, 0, 0, 0}, {0, 1, 2, 3}}};

const short MARGIN_TOP = 0;
const short MARGIN_LEFT = 0;
const short SIZE = 2;  // 2x2 pixel grid
const short TYPES = 6;
const int MELODY_LENGTH = 10;
const int MELODY_NOTES[MELODY_LENGTH] = {262, 294, 330, 262};
const int MELODY_DURATIONS[MELODY_LENGTH] = {500, 500, 500, 500};

int click[] = { 1047 };
int click_duration[] = { 100 };
int erase[] = { 2093 };
int erase_duration[] = { 100 };
word currentType, nextType, rotation;
short pieceX, pieceY;
short piece[2][4];
int interval = 20, score;
long timer, delayer;
boolean grid[WIDTH][HEIGHT/SIZE];  // Dynamic grid size based on display dimensions
boolean b1, b2, b3;


void breakLine(short line) {
  tone(BUZZER, erase[0], 1000 / erase_duration[0]);
  delay(100);
  noTone(BUZZER);
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
  for (short y = HEIGHT/SIZE - 1; y >= 0; y--) {
    full = true;
    for (short x = 0; x < WIDTH; x++) {
      full = full && grid[x][y];
    }
    if (full) { breakLine(y); y++; }
  }
}

void drawGrid() {
  for (short x = 0; x < WIDTH; x++)
    for (short y = 0; y < HEIGHT/SIZE; y++)
      if (grid[x][y])
        display.fillRect(MARGIN_LEFT + (x * 2), MARGIN_TOP + (y * 2), SIZE, SIZE, WHITE);
}

boolean nextHorizontalCollision(short piece[2][4], int amount) {
  for (short i = 0; i < 4; i++) {
    short newX = pieceX + piece[0][i] + amount;
    if (newX > WIDTH - 1 || newX < 0 || grid[newX][pieceY + piece[1][i]])
      return true;
  }
  return false;
}

boolean nextCollision() {
  for (short i = 0; i < 4; i++) {
    short y = pieceY + piece[1][i] + 1;
    short x = pieceX + piece[0][i];
    if (y > (HEIGHT/SIZE - 1) || grid[x][y])  // Dynamic bottom check
      return true;
  }
  return false;
}

void copyPiece(short piece[2][4], short type, short rotation) {
  switch (type) {
    case 0: //L_l
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_L_l[rotation][0][i];
        piece[1][i] = pieces_L_l[rotation][1][i];
      }
      break;
    case 1: //S_l
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_S_l[rotation][0][i];
        piece[1][i] = pieces_S_l[rotation][1][i];
      }
      break;
    case 2: //S_r
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_S_r[rotation][0][i];
        piece[1][i] = pieces_S_r[rotation][1][i];
      }
      break;
    case 3: //Sq
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_Sq[0][0][i];
        piece[1][i] = pieces_Sq[0][1][i];
      }
      break;
    case 4: //T
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_T[rotation][0][i];
        piece[1][i] = pieces_T[rotation][1][i];
      }
      break;
    case 5: //l
      for (short i = 0; i < 4; i++) {
        piece[0][i] = pieces_l[rotation][0][i];
        piece[1][i] = pieces_l[rotation][1][i];
      }
      break;
  }
}

void generate() {
  currentType = nextType;
  nextType = random(TYPES);
  if (currentType != 5)
    pieceX = random(WIDTH);
  else
    pieceX = random(WIDTH - 2);
  pieceY = 0;
  rotation = 0;
  copyPiece(piece, currentType, rotation);
}

void drawPiece(short type, short rotation, short x, short y) {
  for (short i = 0; i < 4; i++)
    display.fillRect(MARGIN_LEFT + ((x + piece[0][i]) * 2), MARGIN_TOP + ((y + piece[1][i]) * 2), SIZE, SIZE, WHITE);
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
  display.clearDisplay();
  drawGrid();
  drawPiece(currentType, 0, pieceX, pieceY);
  display.display();
}

void setup() {
  pinMode(left, INPUT_PULLUP);
  pinMode(right, INPUT_PULLUP);
  pinMode(change, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setRotation(1);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.clearDisplay();
  display.fillRect(0, 0, WIDTH, HEIGHT, WHITE);  // Debug rectangle
  display.display();
  delay(5000);


  randomSeed(analogRead(0));
  nextType = random(TYPES);
  generate();
  timer = millis();
}

void loop() {
  if (millis() - timer > interval) {
    checkLines();
    refresh();
    if (nextCollision()) {
      for (short i = 0; i < 4; i++)
        grid[pieceX + piece[0][i]][pieceY + piece[1][i]] = 1;
      generate();
    } else
      pieceY++;
    timer = millis();
  }

  // when "left" button is pressed
  if (!digitalRead(left)) {
    tone(BUZZER, click[0], 1000 / click_duration[0]);
    delay(100);
    noTone(BUZZER);
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
  if (!digitalRead(right)) {
    tone(BUZZER, click[0], 1000 / click_duration[0]);
    delay(100);
    noTone(BUZZER);
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
  if (!digitalRead(down)) {
    interval = 20;
  } else {
    interval = 400;
  }

  // when "change" button is pressed
  if (!digitalRead(change)) {
    tone(BUZZER, click[0], 1000 / click_duration[0]);
    delay(100);
    noTone(BUZZER);
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
