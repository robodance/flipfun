#include <Arduino.h>
#include <BROSE9323.h>

// #define DEBUG  // Comment this line to disable serial debug output

#define WHITE 1
#define DEFAULT_INTERVAL 300

#define WIDTH 84   // screen width in pixels
#define HEIGHT 16  // screen height in pixels
#define PANEL_WIDTH 28

BROSE9323 display(WIDTH, HEIGHT, PANEL_WIDTH);

// Global variable to control program execution
volatile bool stopProgram = false;

const char text[] = "20 YRS OF HOMEMADE";
const uint8_t textsize = 1;

void clearDisplay(int delayMs = 0) {
  display.fillScreen(1);
  display.display();
  delay(100);
  display.fillScreen(0);
  display.display();
  delay(100);
}

void setup() {
  display.begin();
  clearDisplay();

  display.setTextSize(textsize);
  display.setTextWrap(false);
  display.setTextColor(1, 0);

  randomSeed(analogRead(0));
}

void randomFlip() {
  // Reset stop flag for this program
  stopProgram = false;

  // Program loop - runs until stopProgram is set to true
  while (!stopProgram) {
    // Flip a random number of dots (between 5 and 30)
    int numDots = random(5, 100);  // random(5, 31) gives 5 to 30 inclusive

    for (int i = 0; i < numDots; i++) {
      // Find a random dot on the grid and flip it
      int x = random(0, WIDTH);
      int y = random(0, HEIGHT);
      display.drawPixel(x, y, random(2));
    }
    display.display();
    delay(5);
  }
}

void randomFlicker() {
  // Reset stop flag for this program
  stopProgram = false;

  // Program loop - runs until stopProgram is set to true
  while (!stopProgram) {
    // Set entire screen to random black or white using drawPixel
    int color = random(2);
    for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        display.drawPixel(x, y, color);
      }
    }
    display.display();

    // Random delay between 20-100ms (faster than before)
    delay(random(20, 101));
  }
}

void sweep() {
  // Reset stop flag for this program
  stopProgram = false;

  // Program loop - runs until stopProgram is set to true
  while (!stopProgram) {
    // Draw one bar with random width 0-50 columns
    int barWidth = 120;  // Random width 0-50 columns

    // Animate columns moving from left to right
    for (int startX = 0; startX <= WIDTH; startX++) {
      // Clear entire screen to black
      for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
          display.drawPixel(x, y, 0);
        }
      }

      // Draw the columns for this bar
      for (int col = 0; col < barWidth; col++) {
        int x = startX + col;
        if (x < WIDTH) {                  // Only draw if within screen bounds
          int color = (col % 2) ? 0 : 1;  // Alternate black and white
          for (int y = 0; y < HEIGHT; y++) {
            display.drawPixel(x, y, color);
          }
        }
      }

      display.display();
      delay(1);  // Animation speed
    }
  }
}

void lines() {
  // Reset stop flag for this program
  stopProgram = false;

  display.setDirect(true);
  // Program loop - runs until stopProgram is set to true
  while (!stopProgram) {
    // Randomly flip whole columns across the entire width
    for (int x = 0; x < WIDTH; x++) {
      // Randomly decide if this column should be flipped
      if (random(2)) {
        // Flip the entire column to random color
        int color = random(2);
        for (int y = 0; y < HEIGHT; y++) {
          display.drawPixel(x, y, color);
        }
      }
    }

    delay(random(1, 10));  // Delay between updates
  }
}

void drawText() {
  for (int16_t x = display.width(); x > -((int16_t)strlen(text) * textsize * 6); x--) {
    // Randomize first 3 rows (20 random pixels)
    for (int i = 0; i < 10; i++) {
      int row = random(0, 3);
      int col = random(0, display.width());
      display.drawPixel(col, row, random(2));
    }

    // Randomize last 3 rows (20 random pixels)
    for (int i = 0; i < 10; i++) {
      int row = random(display.height() - 3, display.height());
      int col = random(0, display.width());
      display.drawPixel(col, row, random(2));
    }

    // Draw scrolling text in the middle
    display.setCursor(x, 4);
    display.print(text);
    display.display();
  }
}

void loop() {
  // Main loop - can start different programs here
  drawText();

  // After program exits, you can start another program or wait
  // For now, just restart randomFlip
}
