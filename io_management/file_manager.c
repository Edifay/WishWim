#include "file_manager.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

Cursor initWrittableFileFromFile(char* fileName) {
  Cursor cursor = initNewWrittableFile();
  loadFile(cursor, fileName);
  return cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0));
}


void loadFile(Cursor cursor, char* fileName) {
  cursor = moduloCursor(cursor);
  // If relative_row == 0 no line created. Use initNewWrittableFile() before.
  assert(cursor.file_id.relative_row != 0);

  FILE* f = fopen(fileName, "r");
  if (f == NULL) {
    printf("Coudln't open file %s !\r\n", fileName);
    exit(0);
  }

  FileNode* root = cursor.file_id.file;

  char c;
  while (fscanf(f, "%c", &c) != EOF) {
#ifdef LOGS
    checkFileIntegrity(root);
#endif
    if (iscntrl(c)) {
      if (c == '\n') {
#ifdef LOGS
        printf("Enter\r\n");
#endif
        cursor = insertNewLineInLineC(cursor);
      }
      else if (c == 9) {
#ifdef LOGS
        printf("Tab\r\n");
#endif
        Char_U8 ch;
        ch.t[0] = ' ';
        for (int i = 0; i < 4; i++) {
          cursor = insertCharInLineC(cursor, ch);
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
      cursor = insertCharInLineC(cursor, ch);
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
