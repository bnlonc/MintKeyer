#ifndef MorseKeyer_cpp
#define MorseKeyer_cpp

#include <Arduino.h>

#include "MorseKeyer.h"
#include "MorseUtils.h"
#include "MorseConfig.h"
#include "MorseOutput.h"

char MorseKeyer::lastPlayedSymbol = MorseUtils::CHAR_NIL;
char MorseKeyer::currentlyPlayingSymbol = MorseUtils::CHAR_NIL;
char MorseKeyer::queuedSymbol = MorseUtils::CHAR_NIL;
unsigned long MorseKeyer::lastEventTimestamp = 0L;
long MorseKeyer::eventInterval = 0L;
bool MorseKeyer::timerEnabled = false;

void MorseKeyer::update(const bool dotKeyHeld, const bool dashKeyHeld, const bool configButtonHeld) {
  #ifdef DEBUG
  Serial.println("-----");
  #endif

  const unsigned long currentTimestamp = millis();
  // Handle the paddles 
  // if the event timer is set and expired 
  if (MorseKeyer::timerEnabled && currentTimestamp - MorseKeyer::lastEventTimestamp >= MorseKeyer::eventInterval) {
    #ifdef DEBUG
    Serial.println("Expired timer detected!");
    #endif
    // if something is currently playing: 1. stop it 2. clear the currently playing symbol 3. set a new event timer for 
    // one dot from now for the inter-symbol space 
    if (MorseKeyer::currentlyPlayingSymbol != MorseUtils::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Just finished a symbol, starting timer for intra-character space.");
      #endif
      MorseOutput::turnOff();
      MorseKeyer::lastPlayedSymbol = MorseKeyer::currentlyPlayingSymbol;
      MorseKeyer::currentlyPlayingSymbol = MorseUtils::CHAR_NIL;
      MorseKeyer::setTimer(currentTimestamp, MorseOutput::dotDurationMs);
    } 
    // else if a symbol is queued: 1. copy the symbol from queue to playing 2. clear the queue 3. start playing sound 4. set event timer as appropriate
    else if (MorseKeyer::queuedSymbol != MorseUtils::CHAR_NIL) {
      MorseKeyer::startAsyncSymbolPlay(MorseKeyer::queuedSymbol, currentTimestamp);
      MorseKeyer::queuedSymbol = MorseUtils::CHAR_NIL;
    } 
    // else if nothing is playing or queued, clear the timer
    else {
      MorseKeyer::timerEnabled = false;
      if (MorseConfig::configModeEnabled && !dotKeyHeld && !dashKeyHeld) {
        MorseConfig::handleCharacterInput();
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
    if (MorseKeyer::currentlyPlayingSymbol != MorseUtils::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Something is playing...");
      #endif
      // ...and a symbol is queued...
      if (MorseKeyer::queuedSymbol != MorseUtils::CHAR_NIL) {
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
        if (MorseKeyer::currentlyPlayingSymbol == MorseUtils::CHAR_DOT && dashKeyHeld) {
          #ifdef DEBUG
          Serial.println("Opposite key held, queueing a dash.");
          #endif
          MorseKeyer::queuedSymbol = MorseUtils::CHAR_DASH;
        } else if (MorseKeyer::currentlyPlayingSymbol == MorseUtils::CHAR_DASH && dotKeyHeld) {
          #ifdef DEBUG
          Serial.println("Opposite key held, queueing a dot.");
          #endif
          MorseKeyer::queuedSymbol = MorseUtils::CHAR_DOT;
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
        MorseKeyer::startAsyncSymbolPlay(MorseUtils::CHAR_DOT, currentTimestamp);
      } else if (dashKeyHeld) {
        #ifdef DEBUG
        Serial.println("Handling initial press of dash key");
        #endif
        MorseKeyer::startAsyncSymbolPlay(MorseUtils::CHAR_DASH, currentTimestamp);
      } else if (configButtonHeld) {
        MorseConfig::handleConfigButton(currentTimestamp);
      } else {
        MorseConfig::cancelResetHold();
      }
    } 
    // If nothing is playing, the timer is enabled, and nothing is queued 
    else if (MorseKeyer::queuedSymbol == MorseUtils::CHAR_NIL) {
      #ifdef DEBUG
      Serial.println("Nothing is playing, but a timer is set. Doing nothing.");
      #endif
      // Check if another key is being pressed during the post-symbol waiting period and queue it if so 
      if (MorseKeyer::lastPlayedSymbol == MorseUtils::CHAR_DOT && dashKeyHeld) {
        MorseKeyer::queuedSymbol = MorseUtils::CHAR_DASH;
      } else if (MorseKeyer::lastPlayedSymbol == MorseUtils::CHAR_DASH && dotKeyHeld) {
        MorseKeyer::queuedSymbol = MorseUtils::CHAR_DOT;
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

void MorseKeyer::startAsyncSymbolPlay(char symbolToPlay, unsigned long currentTimestamp) {
  // start of anomaly
  MorseKeyer::currentlyPlayingSymbol = symbolToPlay;
  MorseOutput::turnOn();
  if (symbolToPlay == MorseUtils::CHAR_DOT) {
    if (MorseConfig::configModeEnabled) {
      MorseConfig::pushInputSymbol(MorseUtils::CHAR_DOT);
    }
    MorseKeyer::setTimer(currentTimestamp, MorseOutput::dotDurationMs);
  } else {
    if (MorseConfig::configModeEnabled) {
      MorseConfig::pushInputSymbol(MorseUtils::CHAR_DASH);
    }
    MorseKeyer::setTimer(currentTimestamp, MorseOutput::dotDurationMs * 3);
  }
}

void MorseKeyer::setTimer(unsigned long currentTimestamp, long duration) {
  MorseKeyer::lastEventTimestamp = currentTimestamp;
  MorseKeyer::eventInterval = duration;
  MorseKeyer::timerEnabled = true;
}

#endif