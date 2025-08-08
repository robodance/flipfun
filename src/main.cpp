#include <Arduino.h>
#include <BROSE9323.h>

#define DEBUG 0  // Set to 0 to disable serial debug output

#define WHITE 1
#define DEFAULT_INTERVAL 300

#define WIDTH 84   // screen width in pixels
#define HEIGHT 16  // screen height in pixels
#define PANEL_WIDTH 28
#define MIC_PIN 2

BROSE9323 display(WIDTH, HEIGHT, PANEL_WIDTH);

// Global variable to control program execution
volatile bool stopProgram = false;

// Global variable to track animation start time
unsigned long animationStartTime = 0;
int currentAnimationIndex = 0;

// Animation duration in milliseconds
const unsigned long ANIMATION_DURATION = 10000;  // 10 seconds

const char text[] = "20 YRS OF HOMEMADE";
const uint8_t textsize = 1;

void clearDisplay(int delayMs = 0) {
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

  // Set pin 13 as analog input
  pinMode(MIC_PIN, INPUT);

  // Initialize serial communication for debugging
  Serial.begin(115200);

  randomSeed(analogRead(0));
  delay(100);
}

void randomFlip() {
  // Reset stop flag for this program
  stopProgram = false;

  // Program loop - runs until stopProgram is set to true or 30 seconds pass
  while (!stopProgram && (millis() - animationStartTime) < ANIMATION_DURATION) {
    // Check for serial input
    if (Serial.available() > 0) {
      stopProgram = true;
      break;
    }

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

  // Program loop - runs until stopProgram is set to true or 30 seconds pass
  while (!stopProgram && (millis() - animationStartTime) < ANIMATION_DURATION) {
    // Check for serial input
    if (Serial.available() > 0) {
      stopProgram = true;
      break;
    }

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

  // Buffer to store column colors (one extra column to handle the shift)
  int columnColors[WIDTH + 1];

  // Initialize with alternating pattern
  for (int i = 0; i < WIDTH + 1; i++) {
    columnColors[i] = (i % 2) ? 0 : 1;
  }

  // Program loop - runs until stopProgram is set to true or 30 seconds pass
  while (!stopProgram && (millis() - animationStartTime) < ANIMATION_DURATION) {
    // Check for serial input
    if (Serial.available() > 0) {
      stopProgram = true;
      break;
    }

    // Shift all column colors one step to the right
    for (int x = WIDTH; x > 0; x--) {
      columnColors[x] = columnColors[x - 1];
    }

    // Add new column color on the left (alternating pattern)
    static int columnCounter = 0;
    columnColors[0] = (columnCounter % 2) ? 0 : 1;
    columnCounter++;

    // Draw all columns based on the buffer
    for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        display.drawPixel(x, y, columnColors[x]);
      }
    }

    display.display();
    delay(50);  // Animation speed
  }
}

void lines() {
  // Reset stop flag for this program
  stopProgram = false;

  // Program loop - runs until stopProgram is set to true or 30 seconds pass
  while (!stopProgram && (millis() - animationStartTime) < ANIMATION_DURATION) {
    // Check for serial input
    if (Serial.available() > 0) {
      stopProgram = true;
      break;
    }

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
  clearDisplay();

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

void displayReceivedText(String customText) {
  animationStartTime = millis();
  clearDisplay();

  // Convert String to char array for compatibility
  char textBuffer[100];
  customText.toCharArray(textBuffer, sizeof(textBuffer));

  // Calculate text width for scrolling
  int textWidth = customText.length() * textsize * 6;

  for (int16_t x = display.width(); x > -textWidth; x--) {
    // Randomize first 3 rows (10 random pixels)
    for (int i = 0; i < 10; i++) {
      int row = random(0, 3);
      int col = random(0, display.width());
      display.drawPixel(col, row, random(2));
    }

    // Randomize last 3 rows (10 random pixels)
    for (int i = 0; i < 10; i++) {
      int row = random(display.height() - 3, display.height());
      int col = random(0, display.width());
      display.drawPixel(col, row, random(2));
    }

    // Draw scrolling custom text in the middle
    display.setCursor(x, 4);
    display.print(customText);
    display.display();

    // Check for timeout
    if ((millis() - animationStartTime) >= ANIMATION_DURATION * 1.5) {
      break;
    }

    // Check for serial input
    if (Serial.available() > 0) {
      stopProgram = true;
      break;
    }
  }
}

void matrix() {
  // Reset stop flag for this program
  stopProgram = false;

  // Matrix rain drops - each drop has position and speed
  struct RainDrop {
    int x;
    int y;
    int speed;
    int brightness;
  };

  RainDrop drops[WIDTH / 2];

  // Initialize rain drops
  for (int i = 0; i < WIDTH / 2; i++) {
    drops[i].x = random(0, WIDTH);  // Random x position instead of fixed
    drops[i].y = random(-HEIGHT, 0);
    drops[i].speed = random(1, 4);
    drops[i].brightness = random(1, 3);
  }

  // Program loop - runs until stopProgram is set to true or timeout
  while (!stopProgram && (millis() - animationStartTime) < ANIMATION_DURATION) {
    // Check for serial input
    if (Serial.available() > 0) {
      stopProgram = true;
      break;
    }

    // Clear screen to black manually
    for (int x = 0; x < WIDTH; x++) {
      for (int y = 0; y < HEIGHT; y++) {
        display.drawPixel(x, y, 0);
      }
    }

    // Update and draw rain drops
    for (int i = 0; i < WIDTH / 2; i++) {
      // Move drop down
      drops[i].y += drops[i].speed;

      // If drop goes off screen, reset it to top
      if (drops[i].y >= HEIGHT) {
        drops[i].y = random(-HEIGHT, 0);
        drops[i].speed = random(1, 4);
        drops[i].brightness = random(1, 3);
      }

      // Draw the rain drop trail
      for (int trail = 0; trail < 8; trail++) {
        int trailY = drops[i].y - trail;
        if (trailY >= 0 && trailY < HEIGHT) {
          // Fade brightness along the trail
          int brightness = drops[i].brightness - (trail / 2);
          if (brightness > 0) {
            display.drawPixel(drops[i].x, trailY, brightness);
          }
        }
      }
    }

    // Add random flashing effects
    if (random(100) < 5) {  // 5% chance each frame
      // Flash random horizontal line
      int flashY = random(0, HEIGHT);
      for (int x = 0; x < WIDTH; x++) {
        display.drawPixel(x, flashY, 1);
      }
    }

    // Add random sparkles
    for (int sparkle = 0; sparkle < 3; sparkle++) {
      if (random(100) < 10) {  // 10% chance each sparkle
        int sparkleX = random(0, WIDTH);
        int sparkleY = random(0, HEIGHT);
        display.drawPixel(sparkleX, sparkleY, 1);
      }
    }

    display.display();
    delay(50);
  }
}

void sound(bool simulate = false) {
  // Reset stop flag for this program
  stopProgram = false;
  int circleColor = 0;

  // Program loop - runs until stopProgram is set to true or timeout
  while (!stopProgram && (millis() - animationStartTime) < ANIMATION_DURATION) {
    // Read analog microphone value from pin 19
    int micValue;

    if (simulate) {
      // Generate random microphone values for simulation
      micValue = random(0, 50000);  // Random values between 0 and 50000
    } else {
      // Read actual microphone value
      micValue = pulseIn(MIC_PIN, HIGH);
    }

    if (DEBUG) {
      Serial.print("Microphone value: ");
      Serial.println(micValue);
    }
    // Check if sound level is above threshold (90)
    if (micValue > 90) {
      // Debug: output sensor value to serial

      // Calculate circle radius based on sound level (90-120 range)
      // Map 90-120 to radius 1-4
      int radius = map(micValue, 0, 30000, 2, 8);
      circleColor = random(2);

      // Debug: output circle details
      if (DEBUG) {
        Serial.print("Drawing circle - Radius: ");
        Serial.print(radius);
        Serial.print(", Value: ");
        Serial.print(micValue);

        Serial.print(", Color: ");
        Serial.println(circleColor);
      }

      // Choose random position for circle center
      int centerX = random(radius, WIDTH - radius);
      int centerY = random(radius, HEIGHT - radius);

      // Randomly choose circle color (black or white)

      // Draw filled circle manually using drawPixel
      for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
          // Check if pixel is within circle (using distance formula)
          if (x * x + y * y <= radius * radius) {
            int pixelX = centerX + x;
            int pixelY = centerY + y;
            // Only draw if pixel is within screen bounds
            if (pixelX >= 0 && pixelX < WIDTH && pixelY >= 0 && pixelY < HEIGHT) {
              display.drawPixel(pixelX, pixelY, circleColor);
            }
          }
        }
      }
      display.display();
      delay(100);
    }

    delay(10);
  }
}

void loop() {
  // Main loop - cycles through different animations every 30 seconds

  // Array of animation function names for reference
  const char* animationNames[] = {"matrix", "sound", "sweep", "randomFlip", "randomFlicker", "lines", "drawText"};
  int numAnimations = 7;

  while (true) {
    // Check for serial input before starting each animation
    if (Serial.available() > 0) {
      // Wait a bit for complete data to arrive
      delay(10);

      // Read the incoming text
      String receivedText = Serial.readString();
      receivedText.trim();  // Remove any whitespace

      if (receivedText.length() > 0) {
        stopProgram = true;
        Serial.print("Received text: ");
        Serial.println(receivedText);

        // Display the received text
        displayReceivedText(receivedText);
        stopProgram = false;
        // Continue with normal animation cycle
        continue;
      }
    }

    // Start the current animation
    if (DEBUG) {
      Serial.print("Starting animation: ");
      Serial.println(animationNames[currentAnimationIndex]);
    }

    // Record start time and reset stop flag
    animationStartTime = millis();
    stopProgram = false;

    // Run the current animation
    switch (currentAnimationIndex) {
      case 0:
        matrix();
        break;
      case 1:
        randomFlip();
        break;
      case 2:
        sweep();
        break;
      case 3:
        randomFlicker();
        break;
      case 4:
        lines();
        break;
      case 5:
        drawText();
        break;
    }

    // Move to next animation
    currentAnimationIndex = (currentAnimationIndex + 1) % numAnimations;

    // Brief pause between animations
    delay(10);
  }
}
