#include "file_manager.h"

#include <ctype.h>
#include <stdlib.h>


void loadFile(FileNode* file, char* fileName) {
  FILE* f = fopen(fileName, "r");
  if (f == NULL) {
    printf("Coudln't open file %s !\r\n", fileName);
    exit(0);
  }

  int column = 0;
  int row = 0;
  insertEmptyLineInFile(file, row);
  row++;

  char c;
  while (fscanf(f, "%c", &c) != EOF) {
    LineIdentifier line_id = identifierForCursor(file, row, column);


    if (iscntrl(c)) {
      if (c == '\n') {
        insertEmptyLineInFile(file, row);
        row++;
        column = 0;
      }
      else if (c == 9) {
        Char_U8 ch;
        ch.t[0] = ' ';
        for (int i = 0; i < 4; i++) {
          insertCharInLine(line_id.line, ch, line_id.relative_index);
          column++;
        }
      }
    }
    else {
      Char_U8 ch = readChar_U8FromFileWithFirst(f, c);
      insertCharInLine(line_id.line, ch, line_id.relative_index);
      column++;
    }
  }

  fclose(f);
}

void saveFile(FileNode* file, char* fileName) {
  FILE* f = fopen(fileName, "w");

  while (file != NULL) {
    for (int i = 0; i < file->element_number; i++) {
      LineNode* line = file->lines + i;
      while (line != NULL) {
        for (int j = 0; j < line->element_number; j++) {
          printChar_U8(f, line->ch[j]);
        }
        line = line->next;
      }
      fprintf(f, "\n");
    }
    file = file->next;
  }


  fclose(f);
}
