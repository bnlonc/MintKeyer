#include <Arduino.h>
#include "MorseKeyer.h"
#include "MorseConfig.h"
#include "MorseOutput.h"

// #define PROFILER
// #define LED_DEMO

const int BUTTON_PIN = D2;
const int RIGHT_KEY_PIN = D4;
const int LEFT_KEY_PIN = D3;

void setup() {
  // Set up serial 
  Serial.begin(9600);

  // Initialize pins
  #ifdef LED_DEMO
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  #endif

  MorseOutput::initializeAudioOutputPin();
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LEFT_KEY_PIN, INPUT_PULLUP);
  pinMode(RIGHT_KEY_PIN, INPUT_PULLUP);

  MorseConfig::loadFlashValues();
  MorseOutput::sendString("OK");
}

#ifdef PROFILER
unsigned long secondStartTime = millis();
unsigned int cycleCount = 0;
#endif 

#ifdef LED_DEMO
unsigned short red = 0;
unsigned short grn = 255;
unsigned short blu = 255;
unsigned short phase = 1; 
unsigned short numberOfLedUpdatesToSkip = 5; 
unsigned short skipCount = 0;
#endif 

void loop() {
  #ifdef PROFILER
  if (millis() - secondStartTime >= 1000) {
    char messageBuffer[32];
    sprintf(messageBuffer, "CyclesPerSecond:%u", cycleCount);
    Serial.println(messageBuffer);
    
    #ifdef LED_DEMO
    /* If the profiler and LED demo are both enabled, dynamically set the speed of the LED colour cycling based on the
    number of cycles executed in the last second. */
    numberOfLedUpdatesToSkip = cycleCount / 1000;
    sprintf(messageBuffer, "UpdatesToSkip:%i", numberOfLedUpdatesToSkip);
    Serial.println(messageBuffer);
    #endif

    secondStartTime = millis();
    cycleCount = 0L;
  } else {
    ++cycleCount;
  }
  #endif

  const bool buttonPressed = (digitalRead(BUTTON_PIN) == 0);
  const bool dotButtonPressed = (digitalRead(LEFT_KEY_PIN) == 0);
  const bool dashButtonPressed = (digitalRead(RIGHT_KEY_PIN) == 0);

  MorseKeyer::update(dotButtonPressed, dashButtonPressed, buttonPressed);

  #ifdef LED_DEMO
  /* The skip-related logic is used to only update the colour of the LED once every numberOfLedUpdatesToSkip calls. This is an 
  aesthetic thing to stop the LED from cycling way too fast when the program is running fast due to the debug flag in 
  the MorseKeyer class being disabled. */
  if (skipCount >= numberOfLedUpdatesToSkip) {
    skipCount = 0; 
    cycleRgb();
  } else {
    ++skipCount;
  }
  #endif
}

#ifdef LED_DEMO
void cycleRgb() {
  /* For the purposes of demonstrating basic multitasking using millis(), this function updates the RGB LED on the 
  Arduino by one step along the colour wheel every time it's called. It does this by iterating through six phases as 
  follows: 
  Initial state: Start with red at max, green and blue off 
  1. Change from red to yellow by increasing brightness of green until it's at max
  2. Change from yellow to green by reducing brightness of red until it's off
  3. Change from green to cyan by increasing brightness of blue until it's at max
  4. Change from cyan to blue by reducing brightness of green until it's off 
  5. Change from blue to violet by increasing brightness of red until it's at max
  6. Change from violet to red by reducing brightness of blue until it's off 
  (repeat)
  */
  switch (phase) {
    case 1: 
      --grn;
      break;
    case 2:
      ++red;
      break;
    case 3:
      --blu;
      break;
    case 4:
      ++grn;
      break;
    case 5:
      --red;
      break;
    case 6:
      ++blu;
      break;
  }
  if ((red == 0 || red == 255) && (grn == 0 || grn == 255) && (blu == 0 || blu == 255)) {
    if (++phase > 6) {
      phase = 1;
    }
  }
  analogWrite(LEDR, red);
  analogWrite(LEDG, grn);
  analogWrite(LEDB, blu);
}
#endif