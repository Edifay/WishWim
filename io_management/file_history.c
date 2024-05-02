#include "file_history.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>

void createDir() {
  char command[20 + strlen(FILE_HISTORY_PATH) + strlen(getenv("HOME"))];
  sprintf(command, "mkdir %s%s -p", getenv("HOME"),FILE_HISTORY_PATH);
  system(command);
}


void getLastFilePosition(char* fileName, int* row, int* column) {
  createDir();

  int path_len = 20 /* Size of str can contain int */ + strlen(getenv("HOME")) + strlen(FILE_HISTORY_PATH);
  char path[path_len + 10];
  sprintf(path, "%s%s%d", getenv("HOME"), FILE_HISTORY_PATH, hashFileName(fileName));

  FILE* f = fopen(path, "r");

  if (f == NULL) {
    *row = 1;
    *column = 0;
    return;
  }

  fscanf(f, " %d %d ", row, column);

  fclose(f);
}

void setlastFilePosition(char* fileName, int row, int column) {
  createDir();

  int path_len = 20 /* Size of str can contain int */ + strlen(getenv("HOME")) + strlen(FILE_HISTORY_PATH);
  char path[path_len + 10];
  sprintf(path, "%s%s%d", getenv("HOME"), FILE_HISTORY_PATH, hashFileName(fileName));

  FILE* f = fopen(path, "w");

  if (f == NULL) {
    printf("Error saving history.\r\n");
    return;
  }

  fprintf(f, "%d %d", row, column);

  fclose(f);
}

int hashFileName(char* fileName) {
  int length = strlen(fileName);
  int value = pow(length, 6);

  for (int i = 0; i < length; i++) {
    value += i * i * fileName[i];
  }

  if (value < 0)
    return -value;
  return value;
}
