#include "viewport_history.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io_manager.h"

void createConfigDir() {
  char command[20 + strlen(FILE_HISTORY_PATH) + strlen(getenv("HOME"))];
  sprintf(command, "mkdir %s%s -p", getenv("HOME"),FILE_HISTORY_PATH);
  system(command);
}


void getLastFilePosition(char* fileName, int* row, int* column, int* screen_x, int* screen_y) {
  createConfigDir();

  int path_len = 60 /* Size of str can contain int */ + strlen(getenv("HOME")) + strlen(FILE_HISTORY_PATH);
  char path[path_len + 10];
  sprintf(path, "%s%s%lld", getenv("HOME"), FILE_HISTORY_PATH, hashFileName(fileName));

  FILE* f = fopen(path, "r");

  if (f == NULL) {
    *row = 1;
    *column = 0;
    return;
  }

  fscanf(f, " %d %d %d %d", row, column, screen_x, screen_y);

  fclose(f);
}

void setlastFilePosition(char* fileName, int row, int column, int screen_x, int screen_y) {
  createConfigDir();

  int path_len = 20 /* Size of str can contain int */ + strlen(getenv("HOME")) + strlen(FILE_HISTORY_PATH);
  char path[path_len + 10];
  sprintf(path, "%s%s%lld", getenv("HOME"), FILE_HISTORY_PATH, hashFileName(fileName));

  FILE* f = fopen(path, "w");

  if (f == NULL) {
    printf("Error saving history.\r\n");
    return;
  }

  fprintf(f, "%d %d \n %d %d", row, column, screen_x, screen_y);

  fclose(f);
}



void fetchSavedCursorPosition(IO_FileID file, Cursor* cursor, int* screen_x, int* screen_y) {
  if (file.status == EXIST) {
    int loaded_row;
    int loaded_column;

    getLastFilePosition(file.path_abs, &loaded_row, &loaded_column, screen_x, screen_y);
    // if assert don't pass file_history is corrupted.
    assert(loaded_row >= 1);
    assert(loaded_column >= 0);

    cursor->file_id = tryToReachAbsRow(cursor->file_id, loaded_row);
    cursor->line_id = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(cursor->file_id), 0), loaded_column);

    // TODO may check for screen_x and screen_y to be not too far from code.
  }
}
