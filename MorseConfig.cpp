#ifndef MorseConfig_cpp
#define MorseConfig_cpp

#include "MorseConfig.h"
#include "MorseOutput.h"
#include "MorseUtils.h"

bool MorseConfig::configModeEnabled = false;
unsigned short MorseConfig::speedWpm = 15;
char MorseConfig::inputSymbolBuffer[];
char MorseConfig::inputCharacterBuffer[];

#include <NanoBLEFlashPrefs.h>
NanoBLEFlashPrefs myFlashPrefs;
typedef struct flashStruct
{
  unsigned short tonePitch;
  unsigned short speedWpm;
} flashPrefs;
flashPrefs globalPrefs;

void MorseConfig::enableConfigMode() {
  #ifdef DEBUG
  Serial.println("Entering config mode");
  #endif
  MorseConfig::configModeEnabled = true;
  MorseOutput::playStartTone();
  // Clear out the input buffers
  memset(MorseConfig::inputSymbolBuffer, 0, sizeof(MorseConfig::inputSymbolBuffer));
  memset(MorseConfig::inputCharacterBuffer, 0, sizeof(MorseConfig::inputCharacterBuffer));
}

void MorseConfig::pushInputSymbol(char symbol) {
  #ifdef DEBUG
  Serial.print("Pushing symbol to input symbol buffer: ");
  Serial.println(symbol);
  #endif
  MorseUtils::pushToBuffer(MorseConfig::inputSymbolBuffer, MorseConfig::INPUT_SYMBOL_BUFFER_SIZE, symbol);
}

void MorseConfig::handleCharacterInput() {
  // Get the character buffer contents
  const char inputChar = MorseUtils::getCharacterFromSymbols(MorseConfig::inputSymbolBuffer, MorseConfig::INPUT_SYMBOL_BUFFER_SIZE);
  // Clear the symbol buffer 
  memset(MorseConfig::inputSymbolBuffer, 0, sizeof(MorseConfig::inputSymbolBuffer));
  // Store the character
  MorseUtils::pushToBuffer(MorseConfig::inputCharacterBuffer, MorseConfig::INPUT_CHARACTER_BUFFER_SIZE, inputChar);
  // Handle processing of commands 
  MorseConfig::handleConfigCommand();
  if (MorseConfig::configModeEnabled && MorseConfig::inputCharacterBuffer[MorseConfig::INPUT_CHARACTER_BUFFER_SIZE - 1] != '\0') {
    MorseConfig::exitConfigMode(false);
  }
}

void MorseConfig::updateSpeed(unsigned short newSpeedWpm) {
  MorseConfig::speedWpm = newSpeedWpm;
  MorseOutput::dotDurationMs = 60000 / (50 * speedWpm);
  storeFlashValues();
}

void MorseConfig::exitConfigMode(bool success) {
  #ifdef DEBUG
  Serial.println("Exiting config mode");
  #endif
  delay(400);
  MorseConfig::configModeEnabled = false;
  if (success) {
    MorseOutput::playSuccessTone();
  } else {
    MorseOutput::playFailureTone();
  }
}

void MorseConfig::loadFlashValues() {
  myFlashPrefs.readPrefs(&globalPrefs, sizeof(globalPrefs));
  MorseOutput::tonePitch = globalPrefs.tonePitch;
  MorseConfig::updateSpeed(globalPrefs.speedWpm);
}

void MorseConfig::storeFlashValues() {
  globalPrefs.tonePitch = MorseOutput::tonePitch;
  globalPrefs.speedWpm = MorseConfig::speedWpm;
  myFlashPrefs.writePrefs(&globalPrefs, sizeof(globalPrefs));
}

void MorseConfig::handleConfigCommand() {
  #ifdef DEBUG
  char debugBuffer[32];
  #endif
  switch (MorseConfig::inputCharacterBuffer[0]) {
    case 'S': 
      #ifdef DEBUG
        Serial.println("Matched S for index 0");
      #endif
      if (!MorseUtils::isDigitOrNull(MorseConfig::inputCharacterBuffer[1]) || !MorseUtils::isDigitOrNull(MorseConfig::inputCharacterBuffer[2])) {
        #ifdef DEBUG
        Serial.println("Exiting S for index 1");
        #endif
        MorseConfig::exitConfigMode(false);
        break;
      }
      if (MorseConfig::inputCharacterBuffer[1] >= '5') {
        const int newSpeed = MorseUtils::stringToInt(&(MorseConfig::inputCharacterBuffer[1]), 1);
        #ifdef DEBUG
        sprintf(debugBuffer, "Updating speed to %d", newSpeed);
        Serial.println(debugBuffer);
        #endif
        updateSpeed(newSpeed);
        MorseConfig::exitConfigMode(true);
      } else if (MorseConfig::inputCharacterBuffer[2] != '\0') {
        const int newSpeed = MorseUtils::stringToInt(&(MorseConfig::inputCharacterBuffer[1]), 2);
        #ifdef DEBUG
        sprintf(debugBuffer, "Updating speed to %d", newSpeed);
        Serial.println(debugBuffer);
        #endif
        updateSpeed(newSpeed);
        MorseConfig::exitConfigMode(true);
      } 
      break;
    case 'T':
      if (!MorseUtils::isDigitOrNull(MorseConfig::inputCharacterBuffer[1]) || !MorseUtils::isDigitOrNull(MorseConfig::inputCharacterBuffer[2])) {
        #ifdef DEBUG
        Serial.println("Exiting T for index 1");
        #endif
        MorseConfig::exitConfigMode(false);
        break;
      }
      if (MorseConfig::inputCharacterBuffer[2] != '\0') {
        const int newTone = MorseUtils::stringToInt(&(MorseConfig::inputCharacterBuffer[1]), 2) * 10;
        #ifdef DEBUG
        sprintf(debugBuffer, "Updating tone to %d", newTone);
        Serial.println(debugBuffer);
        delay(1000);
        #endif
        MorseOutput::tonePitch = newTone;
        storeFlashValues();
        MorseConfig::exitConfigMode(true);
      }
      break;
    case 'Q': 
      switch (MorseConfig::inputCharacterBuffer[1]) {
        case 'S':
          delay(MorseOutput::dotDurationMs * 7);
          MorseOutput::sendChar((MorseConfig::speedWpm / 10) + 48);
          delay(MorseOutput::dotDurationMs);
          MorseOutput::sendChar((MorseConfig::speedWpm % 10) + 48);
          MorseConfig::exitConfigMode(true);
          break;
        case 'T':
          delay(MorseOutput::dotDurationMs * 7);
          MorseOutput::sendChar((MorseOutput::tonePitch / 100) + 48);
          delay(MorseOutput::dotDurationMs);
          MorseOutput::sendChar(((MorseOutput::tonePitch % 100) / 10) + 48);
          delay(MorseOutput::dotDurationMs);
          MorseOutput::sendChar('0');
          MorseConfig::exitConfigMode(true);
          break;
        case '\0':
          break;
        default: 
          #ifdef DEBUG
          Serial.println("Exiting default for Q index 1");
          #endif
          MorseConfig::exitConfigMode(false);
          break;
      }
      break;
    default:
      #ifdef DEBUG
      Serial.println("Exiting default for index 0");
      #endif
      MorseConfig::exitConfigMode(false);
      break;
  }
}

#endif