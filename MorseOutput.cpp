#ifndef MorseOutput_cpp
#define MorseOutput_cpp

#include <Arduino.h>
#include "MorseOutput.h"
#include "MorseUtils.h"

constexpr int MorseOutput::AUDIO_PIN;
unsigned short MorseOutput::tonePitch = MorseOutput::DEFAULT_TONE_PITCH;
unsigned int MorseOutput::dotDurationMs = 80;

void MorseOutput::initializeAudioOutputPin() {
  pinMode(MorseOutput::AUDIO_PIN, OUTPUT);
}

void MorseOutput::setDefaultTonePitch(unsigned short value) {
  MorseOutput::tonePitch = MorseOutput::DEFAULT_TONE_PITCH;
} 

void MorseOutput::sendString(char* stringToSend) {
  for (unsigned short index = 0; stringToSend[index] != '\0'; ++index) {
    MorseOutput::sendChar(stringToSend[index]);
    delay(MorseOutput::dotDurationMs * 3);
  }
}

void MorseOutput::sendChar(char input) {
  #ifdef DEBUG
  Serial.print("Starting send of char ");
  Serial.println(input);
  #endif
  const char* codeString = MorseUtils::getCodeForCharacter(input);
  #ifdef DEBUG
  Serial.print("Got char code ");
  Serial.println(codeString);
  #endif
  int index = 0;

  while (codeString[index] != '\0') {
    if (codeString[index] == MorseUtils::CHAR_DOT) {
      MorseOutput::sendDot();
    } else {
      MorseOutput::sendDash();
    }
    ++index;

    delay(MorseOutput::dotDurationMs);
  }
}

void MorseOutput::sendDot() {
  MorseOutput::turnOn();
  delay(MorseOutput::dotDurationMs);
  MorseOutput::turnOff();
}

void MorseOutput::sendDash() {
  MorseOutput::turnOn();
  delay(MorseOutput::dotDurationMs * 3);
  MorseOutput::turnOff();
}

void MorseOutput::turnOn() {
  tone(MorseOutput::AUDIO_PIN, MorseOutput::tonePitch);
}

void MorseOutput::turnOff() {
  noTone(MorseOutput::AUDIO_PIN);
}

void MorseOutput::playStartTone() {
  // Play a nice major chord, with the currently-configured Morse code output tone serving as the third 
  const float rootNote = MorseOutput::tonePitch * 0.7937;
  const float fifthNote = MorseOutput::tonePitch * 1.1892;
  
  float notes[3];
  notes[0] = rootNote;
  notes[1] = MorseOutput::tonePitch;
  notes[2] = fifthNote;

  float durations[3] = {MorseOutput::CONFIG_NOTE_DURATION, MorseOutput::CONFIG_NOTE_DURATION, MorseOutput::CONFIG_NOTE_DURATION};
  MorseOutput::playNotes(3, notes, durations);
}

void MorseOutput::playSuccessTone() {
  // Play a nice major chord, with the currently-configured Morse code output tone serving as the third 
  const float fifthNote = MorseOutput::tonePitch * 1.1892;
  const float rootNote = MorseOutput::tonePitch * 0.7937;
  
  float notes[3];
  notes[0] = fifthNote;
  notes[1] = MorseOutput::tonePitch;
  notes[2] = rootNote;
  
  float durations[3] = {MorseOutput::CONFIG_NOTE_DURATION, MorseOutput::CONFIG_NOTE_DURATION, MorseOutput::CONFIG_NOTE_DURATION};
  
  MorseOutput::playNotes(3, notes, durations);
}

void MorseOutput::playFailureTone() {
  // Play a nasty tritone with each note equidistant from the currently-configured Morse code output tone
  const float highNote = MorseOutput::tonePitch * 1.1225;
  const float lowNote = MorseOutput::tonePitch * 0.7937;

  float notes[2];
  float durations[2];
  
  notes[0] = highNote;
  durations[0] = MorseOutput::CONFIG_NOTE_DURATION;
  notes[1] = lowNote;
  durations[1] = MorseOutput::CONFIG_NOTE_DURATION;
  
  MorseOutput::playNotes(2, notes, durations);
}

void MorseOutput::playNotes(unsigned int noteCount, const float* notes, const float* durations) {
  for (unsigned short index = 0; index < noteCount; ++index) {
    if (tone[index] == 0) {
      noTone(MorseOutput::AUDIO_PIN);
    } else {
      tone(MorseOutput::AUDIO_PIN, notes[index]);
    }
    delay(durations[index]);
  }
  noTone(MorseOutput::AUDIO_PIN);
  delay(MorseOutput::CONFIG_NOTE_DURATION * 2);
}

#endif