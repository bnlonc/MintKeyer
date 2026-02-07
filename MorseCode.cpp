#ifndef MorseCode_cpp
#define MorseCode_cpp

// #define DEBUG

#include "MorseCode.hpp"

#include <Arduino.h>
#include "MorseOutput.cpp"
unsigned short MorseOutput::tonePitch = 740;

#include <NanoBLEFlashPrefs.h>
NanoBLEFlashPrefs myFlashPrefs;
typedef struct flashStruct
{
  unsigned short tonePitch;
  unsigned short speedWpm;
} flashPrefs;
flashPrefs globalPrefs;

unsigned short MorseCode::speedWpm = 15;
unsigned int MorseCode::dotDurationMs = 80;
char MorseCode::lastPlayedSymbol = MorseCode::CHAR_NIL;
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
    // if something is currently playing: 1. stop it 2. clear the currently playing symbol 3. set a new event timer for 
    // one dot from now for the inter-symbol space 
    if (MorseCode::currentlyPlayingSymbol != MorseCode::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Just finished a symbol, starting timer for intra-character space.");
      #endif
      MorseOutput::turnOff();
      MorseCode::lastPlayedSymbol = MorseCode::currentlyPlayingSymbol;
      MorseCode::currentlyPlayingSymbol = MorseCode::CHAR_NIL;
      MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs);
    } 
    // else if a symbol is queued: 1. copy the symbol from queue to playing 2. clear the queue 3. start playing sound 4. set event timer as appropriate
    else if (MorseCode::queuedSymbol != MorseCode::CHAR_NIL) {
      MorseCode::startAsyncSymbolPlay(MorseCode::queuedSymbol, currentTimestamp);
      MorseCode::queuedSymbol = MorseCode::CHAR_NIL;
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
        if (MorseCode::configModeEnabled && MorseCode::inputCharacterBuffer[MorseCode::INPUT_CHARACTER_BUFFER_SIZE - 1] != '\0') {
          MorseCode::exitConfigMode(false);
        }
      }
    }
  } else {
    // The event timer is disabled or not expired yet.
    #ifdef DEBUG
    if (timerEnabled) {
      Serial.println("Active timer; has not yet expired.");
    } else {
      Serial.println("No active timer.");
    }
    #endif
    // If a symbol is playing...
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
    // If a symbol isn't playing and the timer is disabled
    else if (!timerEnabled) {
      #ifdef DEBUG
      Serial.println("Nothing is playing and no timer is set.");
      #endif
      if (dotKeyHeld) {
        #ifdef DEBUG
        Serial.println("Handling initial press of dot key");
        #endif
        MorseCode::startAsyncSymbolPlay(MorseCode::CHAR_DOT, currentTimestamp);
      } else if (dashKeyHeld) {
        #ifdef DEBUG
        Serial.println("Handling initial press of dash key");
        #endif
        MorseCode::startAsyncSymbolPlay(MorseCode::CHAR_DASH, currentTimestamp);
      } else if (configButtonHeld) {
        Serial.println("Entering config mode");
        MorseCode::configModeEnabled = true;
        MorseOutput::playStartTone();
        // Clear out the input buffers
        memset(MorseCode::inputSymbolBuffer, 0, sizeof(MorseCode::inputSymbolBuffer));
        memset(MorseCode::inputCharacterBuffer, 0, sizeof(MorseCode::inputCharacterBuffer));
      }
    } 
    // If nothing is playing, the timer is enabled, and nothing is queued 
    else if (MorseCode::queuedSymbol == MorseCode::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Nothing is playing, but a timer is set. Doing nothing.");
      #endif
      // Check if another key is being pressed during the post-symbol waiting period and queue it if so 
      if (MorseCode::lastPlayedSymbol == MorseCode::CHAR_DOT && dashKeyHeld) {
        MorseCode::queuedSymbol = MorseCode::CHAR_DASH;
      } else if (MorseCode::lastPlayedSymbol == MorseCode::CHAR_DASH && dotKeyHeld) {
        MorseCode::queuedSymbol = MorseCode::CHAR_DOT;
      }
    }
    // If nothing is playing, the timer is enabled, and there's something already waiting in the queue 
    else {
      #ifdef DEBUG
      Serial.println("Nothing is playing, a timer is set, and something is already queued. Doing nothing.");
      #endif
    }
  }
}

void MorseCode::startAsyncSymbolPlay(char symbolToPlay, unsigned long currentTimestamp) {
  MorseCode::currentlyPlayingSymbol = symbolToPlay;
  MorseOutput::turnOn();
  if (symbolToPlay == MorseCode::CHAR_DOT) {
    if (MorseCode::configModeEnabled) {
      MorseCode::pushToBuffer(MorseCode::inputSymbolBuffer, MorseCode::INPUT_SYMBOL_BUFFER_SIZE, MorseCode::CHAR_DOT);
    }
    MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs);
  } else {
    if (MorseCode::configModeEnabled) {
      MorseCode::pushToBuffer(MorseCode::inputSymbolBuffer, MorseCode::INPUT_SYMBOL_BUFFER_SIZE, MorseCode::CHAR_DASH);
    }
    MorseCode::setTimer(currentTimestamp, MorseCode::dotDurationMs * 3);
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
      if (!isDigitOrNull(MorseCode::inputCharacterBuffer[1]) || !isDigitOrNull(MorseCode::inputCharacterBuffer[2])) {
        Serial.println("Exiting S for index 1");
        exitConfigMode(false);
        break;
      }
      if (MorseCode::inputCharacterBuffer[1] >= '5') {
        sprintf(debugBuffer, "Updating speed to %d", inputCharacterBuffer[1] - 48);
        Serial.println(debugBuffer);
        updateSpeed(inputCharacterBuffer[1] - 48);
        exitConfigMode(true);
      } else if (MorseCode::inputCharacterBuffer[2] != '\0') {
        sprintf(debugBuffer, "Updating speed to %d", (inputCharacterBuffer[1] - 48) * 10 + (inputCharacterBuffer[2] - 48));
        Serial.println(debugBuffer);
        updateSpeed((inputCharacterBuffer[1] - 48) * 10 + (inputCharacterBuffer[2] - 48));
        exitConfigMode(true);
      } 
      break;
    case 'T':
      if (!isDigitOrNull(MorseCode::inputCharacterBuffer[1]) || !isDigitOrNull(MorseCode::inputCharacterBuffer[2])) {
        Serial.println("Exiting S for index 1");
        exitConfigMode(false);
        break;
      }
      if (MorseCode::inputCharacterBuffer[2] != '\0') {
        MorseOutput::tonePitch = (inputCharacterBuffer[1] - 48) * 100 + (inputCharacterBuffer[2] - 48) * 10;
        storeFlashValues();
        exitConfigMode(true);
      }
      break;
    case 'Q': 
      switch (MorseCode::inputCharacterBuffer[1]) {
        case 'S':
          delay(MorseCode::dotDurationMs * 7);
          MorseCode::sendChar((MorseCode::speedWpm / 10) + 48);
          delay(MorseCode::dotDurationMs);
          MorseCode::sendChar((MorseCode::speedWpm % 10) + 48);
          exitConfigMode(true);
          break;
        case 'T':
          delay(MorseCode::dotDurationMs * 7);
          MorseCode::sendChar((MorseOutput::tonePitch / 100) + 48);
          delay(MorseCode::dotDurationMs);
          MorseCode::sendChar(((MorseOutput::tonePitch % 100) / 10) + 48);
          delay(MorseCode::dotDurationMs);
          MorseCode::sendChar('0');
          exitConfigMode(true);
          break;
        case '\0':
          break;
        default: 
          Serial.println("Exiting default for Q index 1");
          exitConfigMode(false);
          break;
      }
      break;
    default:
      Serial.println("Exiting default for index 0");
      exitConfigMode(false);
      break;
  }
}

bool MorseCode::isDigitOrNull(char character) {
  return (character >= '0' && character <= '9') || character == '\0';
}

void MorseCode::exitConfigMode(bool success) {
  Serial.println("Exiting config mode");
  delay(400);
  MorseCode::configModeEnabled = false;
  if (success) {
    MorseOutput::playSuccessTone();
  } else {
    MorseOutput::playFailureTone();
  }
}

void MorseCode::sendString(char* stringToSend) {
  for (unsigned short index = 0; stringToSend[index] != '\0'; ++index) {
    MorseCode::sendChar(stringToSend[index]);
    delay(MorseCode::dotDurationMs * 3);
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
  storeFlashValues();
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

void MorseCode::storeFlashValues() {
  globalPrefs.tonePitch = MorseOutput::tonePitch;
  globalPrefs.speedWpm = MorseCode::speedWpm;
  myFlashPrefs.writePrefs(&globalPrefs, sizeof(globalPrefs));
}

void MorseCode::loadFlashValues() {
  myFlashPrefs.readPrefs(&globalPrefs, sizeof(globalPrefs));
  MorseOutput::tonePitch = globalPrefs.tonePitch;
  MorseCode::updateSpeed(globalPrefs.speedWpm);
}

#endif