#ifndef MorseCodeUtils_h
#define MorseCodeUtils_h

class MorseUtils {
  public: 
    static constexpr char CHAR_NIL = '\0';
    static constexpr char CHAR_DOT = '.';
    static constexpr char CHAR_DASH = '-';

    static const char* getCodeForCharacter(char input);
    static char getCharacterFromSymbols(char* inputString, unsigned short inputStringLength);
    static bool isDigitOrNull(char character);
    static void pushToBuffer(char* buffer, int bufferSize, char character);
    static int stringToInt(char* string, unsigned short stringLength);
  private: 
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
    static constexpr unsigned short REVERSE_MAPPING_START_INDICES_BY_LENGTH[] = {0, 1, 3, 7, 15, 31};
    static constexpr short CHAR_TO_INT_ASCII_CONVERSION_FACTOR = -48;
};

#endif