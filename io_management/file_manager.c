#include "file_manager.h"

#include <ctype.h>
#include <stdlib.h>


void loadFile(FileNode* file, char* fileName) {
  FILE* f = fopen(fileName, "r");
  if (f == NULL) {
    printf("Coudln't open file %s !\r\n", fileName);
    exit(0);
  }

  FileIdentifier file_id = moduloFileIdentifier(file, 0);
  file_id = insertEmptyLineInFile(file_id.file, file_id.relative_row);

  LineIdentifier line_id = moduloLineIdentifier(getLineForFileIdentifier(file_id), 0);


  char c;
  while (fscanf(f, "%c", &c) != EOF) {
#ifdef LOGS
    checkFileIntegrity(file);
#endif
    if (iscntrl(c)) {
      if (c == '\n') {
#ifdef LOGS
        printf("Enter\r\n");
#endif
        Cursor cur = insertNewLineInLine(cursorOf(file_id, line_id));
        file_id = cur.file_id;
        line_id = cur.line_id;
      }
      else if (c == 9) {
#ifdef LOGS
        printf("Tab\r\n");
#endif
        Char_U8 ch;
        ch.t[0] = ' ';
        for (int i = 0; i < 4; i++) {
          line_id = insertCharInLine(line_id.line, ch, line_id.relative_column);
        }
      }
      else {
#ifdef LOGS
        printf("Unsupported Char loaded from file : '%d'.\r\n", c);
#endif
        // exit(0);
      }
    }
    else {
      Char_U8 ch = readChar_U8FromFileWithFirst(f, c);
#ifdef LOGS
      printChar_U8(stdout, ch);
      printf("\r\n");
#endif
      line_id = insertCharInLine(line_id.line, ch, line_id.relative_column);
    }
  }

  fclose(f);
}

void saveFile(FileNode* file, char* fileName) {
  FILE* f = fopen(fileName, "w");
  bool first = true;
  while (file != NULL) {
    for (int i = 0; i < file->element_number; i++) {
      if (!first) {
        fprintf(f, "\n");
      }
      else {
        first = false;
      }
      LineNode* line = file->lines + i;
      while (line != NULL) {
        for (int j = 0; j < line->element_number; j++) {
          printChar_U8(f, line->ch[j]);
        }
        line = line->next;
      }
    }
    file = file->next;
  }


  fclose(f);
}
