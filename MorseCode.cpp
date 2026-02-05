#ifndef MorseCode_cpp
#define MorseCode_cpp

// #define DEBUG

#include "MorseCode.hpp"

#include <Arduino.h>
#include "MorseOutput.cpp"
unsigned short MorseOutput::tonePitch = 740;

unsigned short MorseCode::speedWpm = 15;
unsigned int MorseCode::dotDurationMs = 80;
char MorseCode::currentlyPlayingSymbol = MorseCode::CHAR_NIL;
char MorseCode::queuedSymbol = MorseCode::CHAR_NIL;
unsigned long MorseCode::lastEventTimestamp = 0L;
long MorseCode::eventInterval = 0L;
bool MorseCode::timerEnabled = false;
bool MorseCode::configModeEnabled = false;
char MorseCode::inputSymbolBuffer[];
char MorseCode::inputCharacterBuffer[];

constexpr char* MorseCode::FORWARD_MAPPING[];
constexpr char MorseCode::REVERSE_MAPPING[];
constexpr int MorseCode::REVERSE_MAPPING_START_INDICES_BY_LENGTH[];

const char* MorseCode::getCodeForCharacter(char input) {
  // Convert lowercase letters to uppercase
  int index = 0; 
  if ('a' <= input && input <= 'z') {
    input -= 32;
  }
  if ('A' <= input && input <= 'Z') {
    index = input - 54;
  } else if ('0' <= input && input <= '9') {
    index = input - 47;
  } else {
    switch (input) {
      case '.':
        index = 37;
        break;
      case ',':
        index = 38;
        break;
      case '?':
        index = 39;
        break;
      case '!':
        index = 40;
        break;
      case '=':
        index = 41;
        break;
      case '/':
        index = 42;
        break;
    }
  }

  Serial.println(MorseCode::REVERSE_MAPPING);

  return MorseCode::FORWARD_MAPPING[index];
}

void MorseCode::update(const bool dotKeyHeld, const bool dashKeyHeld, const bool configButtonHeld) {

  #ifdef DEBUG
  Serial.println("-----");
  #endif

  const unsigned long currentTimestamp = millis();
  // if the event timer is expired 
  if (MorseCode::timerEnabled && currentTimestamp - MorseCode::lastEventTimestamp >= MorseCode::eventInterval) {
    #ifdef DEBUG
    Serial.println("Expired timer detected!");
    #endif
    // if something is currently playing: 1. stop it 2. clear the currently playing symbol 3. set a new event timer for one dot from now 
    if (MorseCode::currentlyPlayingSymbol != MorseCode::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Just finished a symbol, starting timer for intra-character space.");
      #endif
      MorseOutput::turnOff();
      MorseCode::currentlyPlayingSymbol = MorseCode::CHAR_NIL;
      MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs);
    } 
    // else if a symbol is queued: 1. copy the symbol from queue to playing 2. clear the queue 3. start playing sound 4. set event timer as appropriate
    else if (MorseCode::queuedSymbol != MorseCode::CHAR_NIL) {
      MorseCode::currentlyPlayingSymbol = MorseCode::queuedSymbol;
      MorseCode::queuedSymbol = MorseCode::CHAR_NIL;
      MorseOutput::turnOn();
      if (MorseCode::currentlyPlayingSymbol == MorseCode::CHAR_DOT) {
        if (MorseCode::configModeEnabled) {
          MorseCode::pushToBuffer(MorseCode::inputSymbolBuffer, MorseCode::INPUT_SYMBOL_BUFFER_SIZE, MorseCode::CHAR_DOT);
        }
        #ifdef DEBUG
        Serial.println("Dot was queued, playing it now.");
        #endif
        MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs);
      } else {
        if (MorseCode::configModeEnabled) {
          MorseCode::pushToBuffer(MorseCode::inputSymbolBuffer, MorseCode::INPUT_SYMBOL_BUFFER_SIZE, MorseCode::CHAR_DASH);
        }
        #ifdef DEBUG
        Serial.println("Dash was queued, playing it now.");
        #endif
        MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs * 3);
      }
    } 
    // else if nothing is playing or queued, clear the timer
    else {
      MorseCode::timerEnabled = false;
      if (MorseCode::configModeEnabled && !dotKeyHeld && !dashKeyHeld) {
        // Get the character buffer contents
        const char inputChar = getCharacterFromSymbols(MorseCode::inputSymbolBuffer);
        // Clear the symbol buffer 
        memset(MorseCode::inputSymbolBuffer, 0, sizeof(MorseCode::inputSymbolBuffer));
        // Store the character
        MorseCode::pushToBuffer(MorseCode::inputCharacterBuffer, MorseCode::INPUT_CHARACTER_BUFFER_SIZE, inputChar);
        // Handle processing of commands 
        MorseCode::handleConfigCommand();
        if (MorseCode::inputCharacterBuffer[MorseCode::INPUT_CHARACTER_BUFFER_SIZE - 1] != '\0') {
          Serial.println("Exiting config mode");
          MorseCode::configModeEnabled = false;
        }
      }
    }
  } else {
    #ifdef DEBUG
    if (timerEnabled) {
      Serial.println("Active timer; has not yet expired.");
    } else {
      Serial.println("No active timer.");
    }
    #endif
    // if a symbol is playing...
    if (MorseCode::currentlyPlayingSymbol != MorseCode::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Something is playing...");
      #endif
      // ...and a symbol is queued...
      if (MorseCode::queuedSymbol != MorseCode::CHAR_NIL) {
        #ifdef DEBUG
        Serial.println("And something is queued. Doing nothing.");
        #endif
        // ...do nothing
        return;
      } 
      // ...and nothing is queued
      else {
        #ifdef DEBUG
        Serial.println("But nothing is queued.");
        #endif
        // if the opposite key switch to what is currently playing is being held, queue the opposite symbol to what is currently playing 
        if (MorseCode::currentlyPlayingSymbol == MorseCode::CHAR_DOT && dashKeyHeld) {
          #ifdef DEBUG
          Serial.println("Opposite key held, queueing a dash.");
          #endif
          MorseCode::queuedSymbol = MorseCode::CHAR_DASH;
        } else if (MorseCode::currentlyPlayingSymbol == MorseCode::CHAR_DASH && dotKeyHeld) {
          #ifdef DEBUG
          Serial.println("Opposite key held, queueing a dot.");
          #endif
          MorseCode::queuedSymbol = MorseCode::CHAR_DOT;
        } else {
          #ifdef DEBUG
          Serial.println("Opposite key not held. Doing nothing.");
          #endif
        }
      }
    } 
    // if a symbol isn't playing and the timer is disabled
    else if (!timerEnabled) {
      #ifdef DEBUG
      Serial.println("Nothing is playing and no timer is set.");
      #endif
      if (dotKeyHeld) {
        #ifdef DEBUG
        Serial.println("Handling initial press of dot key");
        #endif
        MorseCode::currentlyPlayingSymbol = MorseCode::CHAR_DOT;
        MorseOutput::turnOn();
        MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs);
        if (MorseCode::configModeEnabled) {
          MorseCode::pushToBuffer(MorseCode::inputSymbolBuffer, MorseCode::INPUT_SYMBOL_BUFFER_SIZE, MorseCode::CHAR_DOT);
        }
      } else if (dashKeyHeld) {
        #ifdef DEBUG
        Serial.println("Handling initial press of dash key");
        #endif
        MorseCode::currentlyPlayingSymbol = MorseCode::CHAR_DASH;
        MorseOutput::turnOn();
        MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs * 3);
        if (MorseCode::configModeEnabled) {
          MorseCode::pushToBuffer(MorseCode::inputSymbolBuffer, MorseCode::INPUT_SYMBOL_BUFFER_SIZE, MorseCode::CHAR_DASH);
        }
      } else if (configButtonHeld) {
        Serial.println("Entering config mode");
        MorseCode::configModeEnabled = true;
        // Clear out the input buffers
        memset(MorseCode::inputSymbolBuffer, 0, sizeof(MorseCode::inputSymbolBuffer));
        memset(MorseCode::inputCharacterBuffer, 0, sizeof(MorseCode::inputCharacterBuffer));
      }
    } else {
      #ifdef DEBUG
      Serial.println("Nothing is playing, but a timer is set. Doing nothing.");
      #endif
    }
  }
}

void MorseCode::setTimer(unsigned long currentTimestamp, long duration) {
  MorseCode::lastEventTimestamp = currentTimestamp;
  MorseCode::eventInterval = duration;
  MorseCode::timerEnabled = true;
}

char MorseCode::getCharacterFromSymbols(char* inputString) {
  int offset = 0;
  int index = 0;

  while (inputString[index] != '\0') {
    if (inputString[index] == MorseCode::CHAR_DASH) {
      offset += 1;
    }
    ++index;
    offset <<= 1;
  }

  offset >>= 1;

  Serial.print("Decoded input to char ");
  Serial.println(MorseCode::REVERSE_MAPPING[MorseCode::REVERSE_MAPPING_START_INDICES_BY_LENGTH[index] + offset]);

  return MorseCode::REVERSE_MAPPING[MorseCode::REVERSE_MAPPING_START_INDICES_BY_LENGTH[index] + offset];
}

void MorseCode::handleConfigCommand() {
  char debugBuffer[32];
  switch (MorseCode::inputCharacterBuffer[0]) {
    case 'S': 
      if (MorseCode::inputCharacterBuffer[1] >= '2') {
        sprintf(debugBuffer, "Updating speed to %d", inputCharacterBuffer[1] - 48);
        Serial.println(debugBuffer);
        updateSpeed(inputCharacterBuffer[1] - 48);
      } else if (MorseCode::inputCharacterBuffer[2] != '\0') {
        sprintf(debugBuffer, "Updating speed to %d", (inputCharacterBuffer[1] - 48) * 10 + (inputCharacterBuffer[2] - 48));
        Serial.println(debugBuffer);
        updateSpeed((inputCharacterBuffer[1] - 48) * 10 + (inputCharacterBuffer[2] - 48));
      } 
      break;
    case 'T':
      if (MorseCode::inputCharacterBuffer[2] != '\0') {
        MorseOutput::tonePitch = (inputCharacterBuffer[1] - 48) * 100 + (inputCharacterBuffer[2] - 48) * 10;
      }
      break;
    case 'Q': 
      switch (MorseCode::inputCharacterBuffer[1]) {
        case 'S':
          MorseCode::sendChar((MorseCode::speedWpm / 10) + 48);
          MorseCode::sendChar((MorseCode::speedWpm % 10) + 48);
          break;
        case 'T':
          MorseCode::sendChar((MorseOutput::tonePitch / 100) + 48);
          MorseCode::sendChar(((MorseOutput::tonePitch % 100) / 10) + 48);
          MorseCode::sendChar('0');
          break;
      }
      break;
  }
}

void MorseCode::sendString(char* stringToSend) {
  for (unsigned short index = 0; stringToSend[index] != '\0'; ++index) {
    MorseCode::sendChar(stringToSend[index]);
  }
}

void MorseCode::sendChar(char input) {
  #ifdef DEBUG
  Serial.print("Starting send of char ");
  Serial.println(input);
  #endif
  const char* codeString = MorseCode::getCodeForCharacter(input);
  #ifdef DEBUG
  Serial.print("Got char code ");
  Serial.println(codeString);
  #endif
  int index = 0;

  while (codeString[index] != '\0') {
    if (codeString[index] == MorseCode::CHAR_DOT) {
      MorseCode::sendDot();
    } else {
      MorseCode::sendDash();
    }
    ++index;

    delay(MorseCode::dotDurationMs);
  }
}

void MorseCode::sendDot() {
  MorseOutput::turnOn();
  delay(MorseCode::dotDurationMs);
  MorseOutput::turnOff();
}

void MorseCode::sendDash() {
  MorseOutput::turnOn();
  delay(MorseCode::dotDurationMs * 3);
  MorseOutput::turnOff();
}

void MorseCode::updateSpeed(unsigned short newSpeedWpm) {
  MorseCode::speedWpm = newSpeedWpm;
  MorseCode::dotDurationMs = 60000 / (50 * speedWpm);
}

void MorseCode::pushToBuffer(char* buffer, int bufferSize, char character) {
  Serial.print("Putting character ");
  Serial.println(character);

  unsigned short index = 0;
  while (buffer[index] != '\0' && index < bufferSize) {
    index++;
  }
  if (index < bufferSize) {
    buffer[index] = character;
  } 

  Serial.print("Buffer is now ");
  Serial.println(buffer);
}

#endif