#ifndef MorseUtils_cpp
#define MorseUtils_cpp

#include "MorseUtils.h"
#include <Arduino.h>

constexpr char* MorseUtils::FORWARD_MAPPING[];
constexpr char MorseUtils::REVERSE_MAPPING[];
constexpr unsigned short MorseUtils::REVERSE_MAPPING_START_INDICES_BY_LENGTH[];

const char* MorseUtils::getCodeForCharacter(char input) {
  // Convert lowercase letters to uppercase
  int index = 0; 
  if ('a' <= input && input <= 'z') {
    input -= 32;
  }
  if ('A' <= input && input <= 'Z') {
    index = input - 54;
  } else if ('0' <= input && input <= '9') {
    index = input - 47;
  } else {
    switch (input) {
      case '.':
        index = 37;
        break;
      case ',':
        index = 38;
        break;
      case '?':
        index = 39;
        break;
      case '!':
        index = 40;
        break;
      case '=':
        index = 41;
        break;
      case '/':
        index = 42;
        break;
    }
  }

  Serial.println(MorseUtils::REVERSE_MAPPING);

  return MorseUtils::FORWARD_MAPPING[index];
}

char MorseUtils::getCharacterFromSymbols(char* inputString, unsigned short inputStringLength) {
  int offset = 0;
  int index = 0;

  #ifdef DEBUG
  Serial.print("Decoding input string ");
  Serial.println(inputString);
  #endif

  while (inputString[index] != '\0' && index < inputStringLength) {
    if (inputString[index] == MorseUtils::CHAR_DASH) {
      offset += 1;
    }
    ++index;
    offset <<= 1;
  }
  offset >>= 1;

  #ifdef DEBUG
  Serial.print("Decoded input to char ");
  Serial.println(MorseUtils::REVERSE_MAPPING[MorseUtils::REVERSE_MAPPING_START_INDICES_BY_LENGTH[index] + offset]);
  #endif

  return MorseUtils::REVERSE_MAPPING[MorseUtils::REVERSE_MAPPING_START_INDICES_BY_LENGTH[index] + offset];
}

bool MorseUtils::isDigitOrNull(char character) {
  return (character >= '0' && character <= '9') || character == '\0';
}

void MorseUtils::pushToBuffer(char* buffer, int bufferSize, char character) {
  #ifdef DEBUG
  Serial.print("Putting character ");
  Serial.println(character);
  #endif
  unsigned short index = 0;
  while (buffer[index] != '\0' && index < bufferSize) {
    index++;
  }
  if (index < bufferSize) {
    buffer[index] = character;
  } 
}

int MorseUtils::stringToInt(char* string, unsigned short stringLength) {
  bool isNegative;
  int startIndex;
  
  if (string[0] == '-') {
    isNegative = true;
    startIndex = 1;
  } else {
    isNegative = false;
    startIndex = 0;
  }
  
  int output = string[startIndex] + CHAR_TO_INT_ASCII_CONVERSION_FACTOR;
  for (unsigned int index = startIndex + 1; index < stringLength; ++index) {
    output *= 10;
    output += string[index] + CHAR_TO_INT_ASCII_CONVERSION_FACTOR;
  }

  if (isNegative) {
    output *= -1;
  }
  return output;
}

#endif