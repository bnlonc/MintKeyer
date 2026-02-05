#ifndef MorseOutput_cpp
#define MorseOutput_cpp

#include <Arduino.h>

const int AUDIO_PIN = A5;

class MorseOutput {
  public: 
    static unsigned short tonePitch;
    static void turnOn() {
      // WHITE
      tone(AUDIO_PIN, MorseOutput::tonePitch);
    }
    static void turnOff() {
      // RGB OFF
      noTone(AUDIO_PIN);
    }
};

#endif