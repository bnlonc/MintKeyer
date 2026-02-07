#ifndef MorseOutput_h
#define MorseOutput_h

#include <Arduino.h>

class MorseOutput {
  public: 
    static constexpr int AUDIO_PIN = A5;
    static unsigned short tonePitch;
    static unsigned int dotDurationMs;

    static void initializeAudioOutputPin();
    static void setDefaultTonePitch(unsigned short value);
    static void sendString(char* stringToSend);
    static void sendChar(char input);
    static void sendDot();
    static void sendDash();
    static void turnOn();
    static void turnOff(); 
    static void playStartTone();
    static void playSuccessTone();
    static void playFailureTone();
  private: 
    static constexpr unsigned short CONFIG_NOTE_DURATION = 100;
    static constexpr unsigned short DEFAULT_TONE_PITCH = 740;

    static void playNotes(unsigned int noteCount, const float* notes, const float* durations);
};

#endif