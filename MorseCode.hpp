#ifndef MorseCode_hpp
#define MorseCode_hpp

class MorseCode {
private:
  static constexpr char CHAR_NIL = '\0';
  static constexpr char CHAR_DOT = '.';
  static constexpr char CHAR_DASH = '-';
  static constexpr char* FORWARD_MAPPING[] = {
    "........", // Error
    "-----",  // 0
    ".----",  // 1
    "..---",  // 2
    "...--",  // 3
    "....-",  // 4
    ".....",  // 5
    "-....",  // 6
    "--...",  // 7
    "---..",  // 8
    "----.",  // 9
    ".-",     // A
    "-...",   // B
    "-.-.",   // C
    "-..",    // D
    ".",      // E
    "..-.",   // F
    "--.",    // G
    "....",   // H
    "..",     // I
    ".---",   // J
    "-.-",    // K
    ".-..",   // L
    "--",     // M
    "-.",     // N
    "---",    // O
    ".--.",   // P
    "--.-",   // Q
    ".-.",    // R
    "...",    // S
    "-",      // T
    "..-",    // U
    "...-",   // V
    ".--",    // W
    "-..-",   // X
    "-.--",   // Y
    "--..",   // Z
    ".-.-.-", // .
    "--..--", // ,
    "..--..", // ?
    "-.-.--", // !
    "-...-",  // =
    "-..-."  // /
  };
  static constexpr char REVERSE_MAPPING[] = {
    '\0', // (empty input)
    'E',  // .
    'T',  // -
    'I',  // ..
    'A',  // .-
    'N',  // -.
    'M',  // --
    'S',  // ...
    'U',  // ..-
    'R',  // .-.
    'W',  // .--
    'D',  // -..
    'K',  // -.-
    'G',  // --.
    'O',  // ---
    'H',  // ....
    'V',  // ...-
    'F',  // ..-.
    '\0', // ..--
    'L',  // .-..
    '\0', // .-.-
    'P',  // .--.
    'J',  // .---
    'B',  // -...
    'X',  // -..-
    'C',  // -.-.
    'Y',  // -.--
    'Z',  // --..
    'Q',  // --.-
    '\0', // ---.
    '\0', // ----
    '5',  // .....
    '4',  // ....-
    '\0', // ...-.
    '3',  // ...--
    '\0', // ..-..
    '\0', // ..-.-
    '\0', // ..--.
    '2',  // ..---
    '\0', // .-...
    '\0', // .-..-
    '\0', // .-.-.
    '\0', // .-.--
    '\0', // .--..
    '\0', // .--.-
    '\0', // .---.
    '1',  // .----
    '6',  // -....
    '=', // -...-
    '\0', // -..-.
    '\0', // -..--
    '\0', // -.-..
    '\0', // -.-.-
    '\0', // -.--.
    '\0', // -.---
    '7',  // --...
    '\0', // --..-
    '\0', // --.-.
    '\0', // --.--
    '8',  // ---..
    '\0', // ---.-
    '9',  // ----.
    '0'   // -----
  };
  static constexpr int REVERSE_MAPPING_START_INDICES_BY_LENGTH[] = {0, 1, 3, 7, 15, 31};
  static unsigned short speedWpm;
  static unsigned int dotDurationMs;
  static char lastPlayedSymbol;
  static char currentlyPlayingSymbol;
  static char queuedSymbol;
  static unsigned long lastEventTimestamp;
  static long eventInterval;
  static bool timerEnabled;
  static bool configModeEnabled;
  static constexpr unsigned short INPUT_SYMBOL_BUFFER_SIZE = 5;
  static char inputSymbolBuffer[INPUT_SYMBOL_BUFFER_SIZE];
  static constexpr unsigned short INPUT_CHARACTER_BUFFER_SIZE = 3;
  static char inputCharacterBuffer[INPUT_CHARACTER_BUFFER_SIZE];

  static const char* getCodeForCharacter(char input);
  static void setTimer(unsigned long currentTimestamp, long duration);
  static char getCharacterFromSymbols(char* inputString);
  static void sendChar(char input);
  static void handleConfigCommand();
  static void updateSpeed(unsigned short newSpeedWpm);
  static void pushToBuffer(char* buffer, int bufferSize, char character);
public: 
  static void update(const bool dotKeyHeld, const bool dashKeyHeld, const bool configButtonHeld);
  static void startAsyncSymbolPlay(char symbolToPlay, unsigned long currentTimestamp);
  static bool isDigitOrNull(char character);
  static void exitConfigMode(bool success);
  static void sendString(char* stringToSend);
  static void sendDot();
  static void sendDash();
  static void storeFlashValues();
  static void loadFlashValues();
};

#endif