#include "file_history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void createConfigDir() {
  char command[20 + strlen(FILE_HISTORY_PATH) + strlen(getenv("HOME"))];
  sprintf(command, "mkdir %s%s -p", getenv("HOME"),FILE_HISTORY_PATH);
  system(command);
}


void getLastFilePosition(char* fileName, int* row, int* column, int *screen_x, int *screen_y) {
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

int powInt(int x, int y) {
  int res = x;

  for(int i = 1 ; i < y ; i++) {
    res *= x;
  }

  return res;
}


// TODO patch abs fileName
unsigned long long hashFileName(char* fileName) {
  int length = strlen(getenv("PWD")) + strlen(fileName);
  unsigned long long value = powInt(length, 4);

  char newStr[length + 10];
  sprintf(newStr, "%s/%s", getenv("PWD"), fileName);

  for (int i = 0; i < length + 1 /* +1 for slash added*/; i++) {
    value += i * i * i * i * newStr[i];
  }

  return value;
}

void fetchSavedCursorPosition(int argc, char** args, Cursor* cursor, int* screen_x, int* screen_y) {
  if (argc >= 2) {
    int loaded_row;
    int loaded_column;

    getLastFilePosition(args[1], &loaded_row, &loaded_column, screen_x, screen_y);

    cursor->file_id = tryToReachAbsRow(cursor->file_id, loaded_row);
    cursor->line_id = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(cursor->file_id), 0), loaded_column);

    // TODO may check for screen_x and screen_y to be not too far from code.
  }
}
