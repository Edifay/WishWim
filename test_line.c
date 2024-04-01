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

#define SEPARATOR false


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

void initNewWrite(LineNode* line, int index) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  printf("-------------------------------------\n\r");
}

struct termios orig_termios;

void die(const char* s) {
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

void printLine(LineNode* line, int index, bool sep) {
  write(STDOUT_FILENO, "\x1b[14;H", 6); // Write at line 15


  LineNode* temp = line;
  int index_temp = index;

  LineIdentifier id = moduloLineIdentifier(line, index);
  line = id.line;
  index = id.relative_column;

  int ava_here = MAX_ELEMENT_NODE - line->element_number;
  int ava_prev = line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line->prev->element_number;
  int ava_next = line->next == NULL ? 0 : MAX_ELEMENT_NODE - line->next->element_number;

  printf("PREV %d | HERE %d | NEXT %d      =>  INDEX %d  & ABS INDEX %d\r\n", ava_prev, ava_here, ava_next, index,
         index_temp);

  line = temp;
  index = index_temp;

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


int main(int argc, char** args) {
  enableRawMode();
  clearFullScreen(); // Clear screen

  // Alloc memory for one line.
  LineNode* line = malloc(sizeof(LineNode));
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

    if (c == 0)
      continue;

    if (iscntrl(c)) {
      // printf("%d\r\n", c);

      if (c == 127) {
        initNewWrite(line, column);
        removeCharInLine(line, column);
        column--;
        printLine(line, column, SEPARATOR);
      }
      else if (c == '\x1b') {
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
          die("read");

        if (c == '[') {
          if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

          if (c == 'C') {
            // move caret
            column++;
            initNewWrite(line, column);
            printLine(line, column, SEPARATOR);
          }
          else if (c == 'D') {
            // move caret
            column--;
            initNewWrite(line, column);
            printLine(line, column, SEPARATOR);
          }
          else if (c == '3') {
            if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
              die("read");

            if (c == '~') {
              printf("DEL !\r\n");
              initNewWrite(line, column + 1);
              removeCharInLine(line, column + 1);
              printLine(line, column, SEPARATOR);
            }
            else {
              printf("Unsupported char. %c\r\n", c);
              exit(0);
            }
          }
          else {
            printf("Unsupported char. %c\r\n", c);
            exit(0);
          }
        }
        else {
          printf("Unsupported char.\r\n");
          exit(0);
        }
      }
    }
    else {
      initNewWrite(line, column);
      Char_U8 ch = readChar_U8FromInput(c);
      insertCharInLine(line, ch, column++);
      printLine(line, column, SEPARATOR);
      // printf("%d ('%c')\r\n", c, c);
    }
  }

  destroyFullLine(line);

  printf("\n\r");

  return 0;
}
