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
#include "io_management/file_manager.h"
#include "utils/tools.h"


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


void printFile(FileNode* file, int row, int column, bool sep) {
  write(STDOUT_FILENO, "\x1b[13;H", 6);


  FileNode* temp = file;
  int row_temp = row - 1;

  FileIdentifier id = moduloFileIdentifier(file, row);
  file = id.file;
  row = id.relative_row;

  LineIdentifier line_id = identifierForCursor(file, row, column);

  int ava_here = MAX_ELEMENT_NODE - line_id.line->element_number;
  int ava_prev = line_id.line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line_id.line->prev->element_number;
  int ava_next = line_id.line->next == NULL ? 0 : MAX_ELEMENT_NODE - line_id.line->next->element_number;


  printf("COLUMN PREV %d | HERE %d | NEXT %d      =>  INDEX %d  & ABS INDEX %d     => FIXED %s\r\n", ava_prev, ava_here,
         ava_next, line_id.relative_column,
         column,
         line_id.line->fixed ? "true" : "false");


  ava_here = MAX_ELEMENT_NODE - file->element_number;
  ava_prev = file->prev == NULL ? 0 : MAX_ELEMENT_NODE - file->prev->element_number;
  ava_next = file->next == NULL ? 0 : MAX_ELEMENT_NODE - file->next->element_number;


  printf("ROW    PREV %d | HERE %d | NEXT %d      =>  ROW %d  & ABS ROW %d\r\n", ava_prev, ava_here, ava_next, row,
         row_temp);


  file = temp;
  row = row_temp;

  write(STDOUT_FILENO, "\x1b[15;H", 6); // Write at line 15

  FileNode* current_row = file;
  int current_row_index = 0;
  while (current_row != NULL) {
    for (int j = 0; j < current_row->element_number; j++) {
      LineNode* line = current_row->lines + j;

      int internal_index = 0;
      if (sep && line->prev != NULL) {
        printf("~");
        if (current_row_index == row) {
          column++;
          internal_index++;
        }
      }


      while (line != NULL) {
        for (int i = 0; i < line->element_number; i++) {
          printChar_U8(stdout, line->ch[i]);
        }

        internal_index += line->element_number;
        if (sep)
          printf("|");
        if (column > internal_index && sep && current_row_index == row) {
          column++;
          internal_index++;
        }

        line = line->next;
      }
      printf("\r\n");

      current_row_index++;
    }

    current_row = current_row->next;
  }

  moveCursorAt(15 + row, column + 1);
}


int main(int argc, char** args) {
  enableRawMode();
  clearFullScreen(); // Clear screen

  // Alloc memory for one line.
  FileNode* file = malloc(sizeof(FileNode));
  initEmptyFileNode(file);

  // Current column
  int column = 0;
  int row = 0;
  if (argc >= 2) {
    printf("LOADING FILE \r\n");
    loadFile(file, args[1]);
    printf("LOAD ENDED \r\n");
    // insert first row
  }
  else {
    insertEmptyLineInFile(file, row);
  }

  row++;
  clearFullScreen();
  printFile(file, row, column, SEPARATOR);


  char c;
  while (1) {
    checkFileIntegrity(file);
    LineIdentifier line_id = identifierForCursor(file, row, column);
    c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
      die("read");
    }
    if (c == CTRL_KEY('q')) {
      break;
    }
    if (c == CTRL_KEY('s')) {
      if (argc < 2) {
        printf("\r\nNo opened file\r\n");
        exit(0);
      }
      saveFile(file, args[1]);
    }

    if (c == 0)
      continue;

    if (iscntrl(c)) {
      // printf("%d\r\n", c);


      if (c == 127) {
        if (column == 0) {
          initNewWrite(line_id.line, line_id.relative_column);
          printf("CURRENT COLUMN %d\r\n", column);
          printf("CURRENT ROW %d\r\n", row);

          printf("Here concat lines !\r\n");

          if (row != 1) {
            FileIdentifier file_id = moduloFileIdentifier(file, row);
            Cursor cursor = concatNeighbordsLines(cursorOf(file_id, line_id));
            printf("Return get %p\r\n", cursor.line_id.line);

            row = getAbsoluteFileIndex(cursor.file_id);
            column = getAbsoluteLineIndex(cursor.line_id);
            printf("AFTER COLUMN %d\r\n", column);
            printf("AFTER ROW %d\r\n", row);
          }

          printFile(file, row, column, SEPARATOR);
        }
        else {
          initNewWrite(line_id.line, line_id.relative_column);
          printf("CURRENT COLUMN %d\r\n", column);
          printf("CURRENT ROW %d\r\n", row);

          removeCharInLine(line_id.line, line_id.relative_column);
          column--;
          printf("AFTER COLUMN %d\r\n", column);
          printf("AFTER ROW %d\r\n", row);

          // printLine(line_id.line, line_id.relative_index, SEPARATOR);
          printFile(file, row, column, SEPARATOR);
        }
      }
      else if (c == 13) {
        initNewWrite(line_id.line, line_id.relative_column);
        FileIdentifier file_id = moduloFileIdentifier(file, row);
        Cursor cur = insertNewLineInLine(cursorOf(file_id, line_id));
        column = 0;
        row++;
        /*insertEmptyLineInFile(file, row);
        row++;
        column = 0;*/
        printFile(file, row, column, SEPARATOR);
      }
      else if (c == 9) {
        initNewWrite(line_id.line, line_id.relative_column);
        Char_U8 ch;
        ch.t[0] = ' ';
        for (int i = 0; i < 4; i++) {
          insertCharInLine(line_id.line, ch, line_id.relative_column);
          column++;
        }
        // printLine(line_id.line, line_id.relative_index, SEPARATOR);
        // printf("%d ('%c')\r\n", c, c);
        printFile(file, row, column, SEPARATOR);
      }
      else if (c == '\x1b') {
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
          die("read");

        if (c == '[') {
          if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
            die("read");

          if (c == 'C') {
            // move caret
            if (line_id.line->element_number != line_id.relative_column ||  isEmptyLine(line_id.line->next) != false) {
              // TODO this cond is wrong. If the next cell is empty error !
              column++;
              initNewWrite(line_id.line, line_id.relative_column);
              // printLine(line_id.line, line_id.relative_index, SEPARATOR);
              printFile(file, row, column, SEPARATOR);
            }
            else if (line_id.line->element_number == line_id.relative_column && isEmptyLine(line_id.line->next) == true) {
              FileIdentifier fileTempId = moduloFileIdentifier(file, row);
              if (fileTempId.file->element_number != fileTempId.relative_row || fileTempId.file->next != NULL) {
                initNewWrite(line_id.line, line_id.relative_column);
                row++;
                column = 0;
                printFile(file, row, column, SEPARATOR);
              }
            }
          }
          else if (c == 'D') {
            if (column != 0) {
              // move caret
              column--;
              initNewWrite(line_id.line, line_id.relative_column);
              // printLine(line_id.line, line_id.relative_index, SEPARATOR);
              printFile(file, row, column, SEPARATOR);
            }
            else {
              if (row != 1) {
                initNewWrite(line_id.line, line_id.relative_column);
                row--;
                column = sizeLineNode(getLineForFileIdentifier(moduloFileIdentifier(file, row)));
                printFile(file, row, column, SEPARATOR);
              }
            }
          }
          else if (c == 'A') {
            if (row != 1) {
              row--;
              FileIdentifier file_id = moduloFileIdentifier(file, row);
              column = min(sizeLineNode(file_id.file->lines + file_id.relative_row - 1), column);
              initNewWrite(line_id.line, line_id.relative_column);
              printFile(file, row, column, SEPARATOR);
            }
          }
          else if (c == 'B') {
            if (row != sizeFileNode(file)) {
              row++;
              FileIdentifier file_id = moduloFileIdentifier(file, row);
              column = min(sizeLineNode(file_id.file->lines + file_id.relative_row - 1), column);
              initNewWrite(line_id.line, line_id.relative_column);
              printFile(file, row, column, SEPARATOR);
            }
          }
          else if (c == 'F') {
            initNewWrite(line_id.line, line_id.relative_column);
            column += sizeLineNode(line_id.line) - line_id.relative_column;
            printFile(file, row, column, SEPARATOR);
          }
          else if (c == '3') {
            if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
              die("read");

            if (c == '~') {
              printf("DEL !\r\n");
              initNewWrite(line_id.line, line_id.relative_column + 1);
              removeCharInLine(line_id.line, line_id.relative_column + 1);
              // printLine(line_id.line, line_id.relative_index, SEPARATOR);
              printFile(file, row, column, SEPARATOR);
            }
            else {
              printf("Unsupported char case 1. %c\r\n", c);
              exit(0);
            }
          }
          else {
            printf("Unsupported char case 2. %c\r\n", c);
            exit(0);
          }
        }
        else {
          printf("Unsupported char case 3.\r\n");
          exit(0);
        }
      }
    }
    else {
      initNewWrite(line_id.line, line_id.relative_column);
      Char_U8 ch = readChar_U8FromInput(c);
      insertCharInLine(line_id.line, ch, line_id.relative_column);
      column++;
      // printLine(line_id.line, line_id.relative_index, SEPARATOR);
      // printf("%d ('%c')\r\n", c, c);
      printFile(file, row, column, SEPARATOR);
    }
  }

  destroyFullFile(file);

  printf("\n\r");

  return 0;
}
