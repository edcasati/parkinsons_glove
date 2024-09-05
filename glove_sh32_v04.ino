// Define the pins
const int outputPins[] = {PC0, PC1, PC2, PC3};
const int numPins = 4;
const int modeIndicatorPin = PC6; // LED connected to PC6

// Define the button pins
const int modeButtonPin = PC4;
const int startStopButtonPin = PC5;

// Variables to store the current mode, button states, and start/stop state
int currentMode = 1;
int numModes = 4; // Total number of modes, including Mode_4
bool lastModeButtonState = HIGH; // Assume the mode button is normally high
bool lastStartStopButtonState = HIGH; // Assume the start/stop button is normally high
bool modeRunning = false; // Track whether the modes are running
bool modeButtonPressed = false;
bool startStopButtonPressed = false;

void setup() {
  // Set all output pins as outputs
  for (int i = 0; i < numPins; i++) {
    pinMode(outputPins[i], OUTPUT);
    digitalWrite(outputPins[i], LOW); // Start with all outputs LOW
  }

  // Set the button pins as input
  pinMode(modeButtonPin, INPUT);
  pinMode(startStopButtonPin, INPUT);

  // Set mode indicator pin as output
  pinMode(modeIndicatorPin, OUTPUT);
  digitalWrite(modeIndicatorPin, LOW); // Start with mode indicator LED off

  // Set up initial mode
  currentMode = 1; // Start with Mode_1

  // Initialize the random number generator
  randomSeed(analogRead(0)); // Use an unconnected analog pin to generate a random seed
}

void loop() {
  // Check for button presses
  checkButtonPresses();

  // Only run the mode if the start/stop button has enabled it
  if (modeRunning) {
    // Call the appropriate mode function
    switch (currentMode) {
      case 1:
        Mode_1();
        break;
      case 2:
        Mode_2();
        break;
      case 3:
        Mode_3(); // Modified Mode_3 with special handling for 11
        break;
      case 4:
        Mode_4(); // Random pattern mode
        break;
      default:
        Mode_1(); // Default to Mode_1 if something goes wrong
    }
  } else {
    // If stopped, turn off all outputs
    for (int i = 0; i < numPins; i++) {
      digitalWrite(outputPins[i], LOW);
    }
  }
}

void checkButtonPresses() {
  bool currentModeButtonState = digitalRead(modeButtonPin);
  bool currentStartStopButtonState = digitalRead(startStopButtonPin);

  // Detect mode button press (falling edge)
  if (lastModeButtonState == HIGH && currentModeButtonState == LOW) {
    modeButtonPressed = true;
  }

  // Detect start/stop button press (falling edge)
  if (lastStartStopButtonState == HIGH && currentStartStopButtonState == LOW) {
    startStopButtonPressed = true;
  }

  // Handle mode button press
  if (modeButtonPressed) {
    // Stop the current running mode
    modeRunning = false;

    // Change the mode (even if not running)
    currentMode++;
    if (currentMode > numModes) {
      currentMode = 1; // Wrap around to Mode_1 if we exceed the number of modes
    }

    // Flash the mode indicator LED the same number of times as the current mode
    flashModeIndicator(currentMode);

    modeButtonPressed = false;
  }

  // Handle start/stop button press
  if (startStopButtonPressed) {
    modeRunning = !modeRunning; // Toggle the running state
    startStopButtonPressed = false;
  }

  // Update the last button states
  lastModeButtonState = currentModeButtonState;
  lastStartStopButtonState = currentStartStopButtonState;
}

void flashModeIndicator(int mode) {
  // Flash the LED connected to PC6 `mode` number of times
  for (int i = 0; i < mode; i++) {
    digitalWrite(modeIndicatorPin, HIGH);
    delay(200); // LED ON for 200ms
    digitalWrite(modeIndicatorPin, LOW);
    delay(200); // LED OFF for 200ms
  }
}

void Mode_3() {
  // Round-Robin pattern
  static int currentPin = 0;
  static unsigned long lastMillis = 0;
  static bool pinState = false; // false = OFF, true = ON
  static int cycleCount = 0;
  static bool inPause = false;

  unsigned long interval = pinState ? 100 : 67;

  if (millis() - lastMillis >= interval) {
    if (inPause) {
      if (millis() - lastMillis >= 1328) {
        inPause = false;
        cycleCount = 0;
      }
    } else {
      pinState = !pinState;

      if (pinState) {
        digitalWrite(outputPins[currentPin], HIGH);
      } else {
        digitalWrite(outputPins[currentPin], LOW);
        currentPin = (currentPin + 1) % numPins;

        if (currentPin == 0) {
          cycleCount++;
          if (cycleCount >= 3) {
            inPause = true;
            lastMillis = millis(); // Start the pause timer
            return;
          }
        }
      }

      lastMillis = millis();
    }
  }
}

void Mode_2() {
  // All at same time
  static unsigned long lastMillis = 0;
  static bool pinState = false;
  static int cycleCount = 0;
  static bool inPause = false;

  unsigned long interval = pinState ? 100 : 67;

  if (millis() - lastMillis >= interval) {
    if (inPause) {
      if (millis() - lastMillis >= 1328) {
        inPause = false;
        cycleCount = 0;
      }
    } else {
      pinState = !pinState;

      for (int i = 0; i < numPins; i++) {
        digitalWrite(outputPins[i], pinState ? HIGH : LOW);
      }

      if (!pinState) {
        cycleCount++;
        if (cycleCount >= 3) {
          inPause = true;
          lastMillis = millis();
          return;
        }
      }

      lastMillis = millis();
    }
  }
}

void Mode_1() {
  // Special Pattern
  static const int pattern[] = {
    3, 2, 4, 1, 1, 2, 4, 3, 2, 4, 3, 1, 11, 11, 11, 11, 11, 11, 11, 11, 
    2, 4, 3, 1, 2, 1, 3, 4, 1, 3, 4, 2
  };
  static int patternIndex = 0;
  static bool pinState = false;
  static unsigned long lastMillis = 0;
  static int cycleCount = 0;
  static bool inPause = false;

  // Default interval
  unsigned long interval = pinState ? 100 : 67;

  // If the current pattern value is 11, change the interval to 167 ms
  if (pattern[patternIndex] == 11) {
    interval = 167;
  }

  if (millis() - lastMillis >= interval) {
    if (inPause) {
      if (millis() - lastMillis >= 1328) {
        inPause = false;
        cycleCount = 0;
      }
    } else {
      pinState = !pinState;

      if (pinState) {
        if (pattern[patternIndex] <= numPins) {
          // Turn ON the corresponding pin
          digitalWrite(outputPins[pattern[patternIndex] - 1], HIGH);
        }
      } else {
        if (pattern[patternIndex] <= numPins) {
          // Turn OFF the corresponding pin
          digitalWrite(outputPins[pattern[patternIndex] - 1], LOW);
        }

        // Move to the next element in the pattern array
        patternIndex = (patternIndex + 1) % (sizeof(pattern) / sizeof(pattern[0]));

        // If we have completed a full cycle (reached the end of the pattern array)
        if (patternIndex == 0) {
          cycleCount++;
          if (cycleCount >= 3) {
            inPause = true;
            lastMillis = millis();
            return;
          }
        }
      }

      lastMillis = millis();
    }
  }
}

void Mode_4() {
  // Random Pattern
  static unsigned long lastMillis = 0;
  static bool pinState = false;
  static int cycleCount = 0;
  static bool inPause = false;

  unsigned long interval = pinState ? 100 : 67;

  if (millis() - lastMillis >= interval) {
    if (inPause) {
      if (millis() - lastMillis >= 1328) {
        inPause = false;
        cycleCount = 0;
      }
    } else {
      pinState = !pinState;

      // Generate random pattern for all pins
      for (int i = 0; i < numPins; i++) {
        digitalWrite(outputPins[i], random(0, 2) ? HIGH : LOW); // Randomly set each pin HIGH or LOW
      }

      if (!pinState) {
        cycleCount++;
        if (cycleCount >= 3) {
          inPause = true;
          lastMillis = millis();
          return;
        }
      }

      lastMillis = millis();
    }
  }
}
