#ifndef UTF_8_EXTRACTOR_H
#define UTF_8_EXTRACTOR_H

#include <stdbool.h>
#include <stdio.h>

typedef struct {
  char t[4];
} Char_U8;

/**
 * Print the Char_U8 to the file.
 */
void printChar_U8(FILE* f, Char_U8 ch);

/**
 * Return on how many bytes is coded this Char_U8.
 */
int sizeChar_U8(Char_U8 ch);

/**
 * Return the first Char_U8 of the current file.
 */
Char_U8 readChar_U8FromFile(FILE* f);

/**
 * Return the first Char_U8 from FILE with first c.
 */
Char_U8 readChar_U8FromFileWithFirst(FILE* f, char c);
/**
 * Return the first Char_U8 from FILE with first c.
 * Using fd
 */
Char_U8 readChar_U8FromFileWithFirstUsingFd(int fd, char c);
/**
 * Return the first Char_U8 from sdtin.
 */
Char_U8 readChar_U8FromInput(char c);

/**
 * Return the first Char_U8 of char*.
 */
Char_U8 readChar_U8FromCharArray(char* ch);

/**
 * Return the first Char_U8 of char* with first c.
 */
Char_U8 readChar_U8FromCharArrayWithFirst(char* array, char c);


/**
 * Print in the file the bits.
 */
void printCharToBits(FILE* f, char ch);

/**
 * Print in the file the bits.
 */
void printChar_U8ToBits(FILE* f, Char_U8 ch);

/**
 * Test with stdin.
 */
void testUnitUtf8Extractor();

int charPrintSize(Char_U8 ch);

bool isBetween(Char_U8 ch, char begin, char end);

bool isAWordLetter(Char_U8 ch);

bool isInvisible(Char_U8 ch);

bool areChar_U8Equals(Char_U8 ch1, Char_U8 ch2);

#endif // UTF_8_EXTRACTOR_H
