#include "utf_8_extractor.h"

#include <ncurses.h>
#include <stdlib.h>
#include <wchar.h>

#include "../utils/constants.h"

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);

/**
 * Print the Char_U8 to the file.
 */
void printChar_U8(FILE* f, Char_U8 ch) {
  int size = sizeChar_U8(ch);
  for (int i = 0; i < size; i++) {
    fprintf(f, "%c", ch.t[i]);
  }
}

/**
 * Return on how many bytes is coded this Char_U8.
 */
int sizeChar_U8(Char_U8 ch) {
  if ((ch.t[0] >> 7 & 0b1) == 0) return 1;
  if ((ch.t[0] >> 5 & 0b1) == 0) return 2;
  if ((ch.t[0] >> 4 & 0b1) == 0) return 3;
  return 4;
}

/**
 * Return the first Char_U8 of the current file.
 */
Char_U8 readChar_U8FromFile(FILE* f) {
  Char_U8 ch;
  fscanf(f, "%c", ch.t);
  int size = sizeChar_U8(ch);

  for (char i = 1; i < size; i++) {
    // scan end of the char
    fscanf(f, "%c", ch.t + i);
  }

  for (char i = size; i < 4; i++) {
    // optional, fill the rest of the Char_U8 with 0
    ch.t[i] = 0;
  }
  return ch;
}

/**
 * Return the first Char_U8 from FILE with first c.
 */
Char_U8 readChar_U8FromFileWithFirst(FILE* f, char c) {
  Char_U8 ch;
  ch.t[0] = c;
  int size = sizeChar_U8(ch);

  for (char i = 1; i < size; i++) {
    // scan end of the char
    fscanf(f, "%c", ch.t + i);
  }

  for (char i = size; i < 4; i++) {
    // optional, fill the rest of the Char_U8 with 0
    ch.t[i] = 0;
  }
  return ch;
}


/**
 * Return the first Char_U8 from sdtin.
 */
Char_U8 readChar_U8FromInput(char c) {
  Char_U8 ch;
  ch.t[0] = c;
  int size = sizeChar_U8(ch);

  for (char i = 1; i < size; i++) {
    // scan end of the char
    fscanf(stdin, "%c", ch.t + i);
  }

  for (char i = size; i < 4; i++) {
    // optional, fill the rest of the Char_U8 with 0
    ch.t[i] = 0;
  }
  return ch;
}

/**
 * Return the first Char_U8 of char*.
 */
Char_U8 readChar_U8FromCharArray(char* array) {
  // TODO becareful, wrong formed arrays may produce seg fault.
  Char_U8 ch;
  ch.t[0] = array[0];
  int size = sizeChar_U8(ch);

  for (char i = 1; i < size; i++) {
    // scan end of the char
    ch.t[i] = array[i];
  }

  for (char i = size; i < 4; i++) {
    // optional, fill the rest of the Char_U8 with 0
    ch.t[i] = 0;
  }
  return ch;
}


/**
 * Return the first Char_U8 of char* starting with c.
 */
Char_U8 readChar_U8FromCharArrayWithFirst(char* array, char c) {
  // TODO becareful, wrong formed arrays may produce seg fault.
  Char_U8 ch;
  ch.t[0] = c;
  int size = sizeChar_U8(ch);

  for (char i = 1; i < size; i++) {
    // scan end of the char
    ch.t[i] = array[i];
  }

  for (char i = size; i < 4; i++) {
    // optional, fill the rest of the Char_U8 with 0
    ch.t[i] = 0;
  }
  return ch;
}

/**
 * Print in the file the bits.
 */
void printCharToBits(FILE* f, char ch) {
  for (int i = 7; i >= 0; i--) {
    char result = (ch >> i) & 0b1;
    fprintf(f, "%d", result);
  }
  fprintf(f, " ");
}


/**
 * Print in the file the bits.
 */
void printChar_U8ToBits(FILE* f, Char_U8 ch) {
  printCharToBits(f, ch.t[0]);
  printCharToBits(f, ch.t[1]);
  printCharToBits(f, ch.t[2]);
  printCharToBits(f, ch.t[3]);
}

void testUnitUtf8Extractor() {
  Char_U8 ch = readChar_U8FromFile(stdin);

  printf("Size of ");
  printChar_U8ToBits(stdout, ch);
  printf("contained in %d bytes.\n", sizeChar_U8(ch));

  printf("Char : ");
  printChar_U8(stdout, ch);
  printf("\n");
}


int charPrintSize(Char_U8 ch) {
  if (ch.t[0] == '\t') {
    return TAB_SIZE;
  }

  if (sizeChar_U8(ch) == 1) // If char is ascii avoid convert and call wcwidth we can instant return size 1. Will not work with control char from ascii.
    return 1;

  wchar_t wc;
  mbtowc(&wc, ch.t, 4);
  return wcwidth(wc);
}


bool isBetween(Char_U8 ch, char begin, char end) {
  return ch.t[0] >= begin && ch.t[0] <= end;
}

bool isAWordLetter(Char_U8 ch) {
  return
      isBetween(ch, 'a', 'z')
      || isBetween(ch, 'A', 'Z')
      || isBetween(ch, '0', '9')
      || areChar_U8Equals(ch, readChar_U8FromCharArray("_"))
      || areChar_U8Equals(ch, readChar_U8FromCharArray("ç"))
      || areChar_U8Equals(ch, readChar_U8FromCharArray("é"))
      || areChar_U8Equals(ch, readChar_U8FromCharArray("è"))
      || areChar_U8Equals(ch, readChar_U8FromCharArray("à"))
      || areChar_U8Equals(ch, readChar_U8FromCharArray("û"))
      || areChar_U8Equals(ch, readChar_U8FromCharArray("ê"));
}

bool isInvisible(Char_U8 ch) {
  return ch.t[0] == ' ' || ch.t[0] == '\t';
}

bool areChar_U8Equals(Char_U8 ch1, Char_U8 ch2) {
  int size1 = sizeChar_U8(ch1);
  int size2 = sizeChar_U8(ch2);
  if (size1 != size2) return false;

  for (int i = 0; i < size1; i++) {
    if (ch1.t[i] != ch2.t[i]) return false;
  }

  return true;
}
