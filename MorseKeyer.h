#ifndef MorseKeyer_h
#define MorseKeyer_h

class MorseKeyer {
  public: 
    static void update(const bool dotKeyHeld, const bool dashKeyHeld, const bool configButtonHeld);
    static void startAsyncSymbolPlay(char symbolToPlay, unsigned long currentTimestamp);
  private:
    static char lastPlayedSymbol;
    static char currentlyPlayingSymbol;
    static char queuedSymbol;
    static unsigned long lastEventTimestamp;
    static long eventInterval;
    static bool timerEnabled;
    
    static void setTimer(unsigned long currentTimestamp, long duration);
};

#endif