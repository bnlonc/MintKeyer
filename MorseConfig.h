#ifndef MorseConfig_h
#define MorseConfig_h

class MorseConfig {
  public: 
    static bool configModeEnabled;
    static unsigned short speedWpm;
    
    static void enableConfigMode();
    static void pushInputSymbol(char symbol);
    static void handleCharacterInput();
    static void updateSpeed(unsigned short newSpeedWpm);
    static void exitConfigMode(bool success); 
    static void loadFlashValues();
  private:
    static constexpr unsigned short INPUT_SYMBOL_BUFFER_SIZE = 5;
    static char inputSymbolBuffer[INPUT_SYMBOL_BUFFER_SIZE];
    static constexpr unsigned short INPUT_CHARACTER_BUFFER_SIZE = 3;
    static char inputCharacterBuffer[INPUT_CHARACTER_BUFFER_SIZE];
    
    static void storeFlashValues();
    static void handleConfigCommand();
};

#endif