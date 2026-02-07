#ifndef MorseOutput_cpp
#define MorseOutput_cpp

#include <Arduino.h>

const int AUDIO_PIN = A5;

class MorseOutput {
  public: 
    static unsigned short tonePitch;
    static void turnOn() {
      tone(AUDIO_PIN, MorseOutput::tonePitch);
    }
    static void turnOff() {
      noTone(AUDIO_PIN);
    }
    static void playStartTone() {
      const float rootNote = MorseOutput::tonePitch * 0.7937;
      const float fifthNote = MorseOutput::tonePitch * 1.1892;
      tone(AUDIO_PIN, rootNote);
      delay(CONFIG_NOTE_DURATION);
      tone(AUDIO_PIN, MorseOutput::tonePitch);
      delay(CONFIG_NOTE_DURATION);
      tone(AUDIO_PIN, fifthNote);
      delay(CONFIG_NOTE_DURATION);
      noTone(AUDIO_PIN);
      delay(250);
    }
    static void playSuccessTone() {
      const float fifthNote = MorseOutput::tonePitch * 1.1892;
      const float rootNote = MorseOutput::tonePitch * 0.7937;
      tone(AUDIO_PIN, fifthNote);
      delay(CONFIG_NOTE_DURATION);
      tone(AUDIO_PIN, MorseOutput::tonePitch);
      delay(CONFIG_NOTE_DURATION);
      tone(AUDIO_PIN, rootNote);
      delay(CONFIG_NOTE_DURATION);
      noTone(AUDIO_PIN);
      delay(250);
    }
    static void playFailureTone() {
      const float highNote = MorseOutput::tonePitch * 1.1225;
      const float lowNote = MorseOutput::tonePitch * 0.7937;
      tone(AUDIO_PIN, highNote);
      delay(CONFIG_NOTE_DURATION);
      tone(AUDIO_PIN, lowNote);
      delay(CONFIG_NOTE_DURATION);
      noTone(AUDIO_PIN);
      delay(250);
    }
  private: 
    static constexpr unsigned short CONFIG_NOTE_DURATION = 100;
};

#endif