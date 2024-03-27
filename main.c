#include "data-structure/utf_8_extractor.h"
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "data-structure/file_structure.h"


#define CTRL_KEY(k) ((k)&0x1f)

#define SEPARATOR true


void PrintAt(int x, int y, char c) {
  printf("\033[%d;%dH%c", x, y, c);
}

void moveCursorAt(int line, int column) {
  char text[20];
  sprintf(text, "\x1b[%d;%dH", line, column);
  write(STDOUT_FILENO, text, strlen(text));
}

void clearFullScreen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
}

void initNewWrite() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  printf("-------------------------------------\n\r");
}

struct termios orig_termios;

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}

void printLine(LineNode *line, int index, bool sep) {
  write(STDOUT_FILENO, "\x1b[15;H", 6); // Write at line 15

  int internal_index = 0;
  while (line != NULL) {
    for (int i = 0; i < line->element_number; i++) {
      printChar_U8(stdout, line->ch[i]);
    }

    internal_index += line->element_number;
    if (sep)
      printf("|");
    if (index > internal_index && sep) {
      index++;
      internal_index++;
    }

    line = line->next;
  }
  printf("\r\n");

  moveCursorAt(15, index + 1);
}


int main(int argc, char **args) {
  enableRawMode();
  clearFullScreen(); // Clear screen

  // Alloc memory for one line.
  LineNode *line = malloc(sizeof(LineNode));
  initEmptyLineNode(line);

  // Current column
  int column = 0;

  char c;
  while (1) {
    c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
      die("read");
    }
    if (c == CTRL_KEY('q')) {
      break;
    }

    if (iscntrl(c)) {
      // printf("%d\r\n", c);
      if (c == 127) {
        initNewWrite();
        removeChar(line, column);
        printLine(line, column, SEPARATOR);
      } else if (c == '\x1b') {

        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
          die("read");

        if (c == '[') {

          if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

          if (c == 'C') { // move caret
            column++;
          } else if (c == 'D') { // move caret
            column--;
          }
          initNewWrite();
          printLine(line, column, SEPARATOR);
        } else {
          printf("Unsupported char.\r\n");
          exit(0);
        }
        
      }
    } else {
      initNewWrite();
      Char_U8 ch = readChar_U8FromInput(c);
      insertChar(line, ch, column++);
      printLine(line, column, SEPARATOR);
      // printf("%d ('%c')\r\n", c, c);
    }
  }

  destroyLine(line);

  printf("\n\r");

  return 0;
}
