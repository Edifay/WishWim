#include "tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/time.h>

time_val timeInMilliseconds(void) {
  struct timeval tv;

  gettimeofday(&tv,NULL);
  return (((time_val)tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

time_val diff2Time(time_val start, time_val end) {
  time_val diff = start - end;
  if (diff < 0)
    return -diff;
  return diff;
}


int min(int a, int b) {
  if (a < b) return a;
  return b;
}

int max(int a, int b) {
  if (a > b) return a;
  return b;
}

int numberOfDigitOfNumber(int n) {
  char page_number[40];
  sprintf(page_number, "%d", n);
  return strlen(page_number);
}

int powInt(int x, int y) {
  int res = x;

  for (int i = 1; i < y; i++) {
    res *= x;
  }

  return res;
}

/**
 * Give as fileName the absolute path of the file !
 */
unsigned long long hashFileName(char* fileName) {
  int length = strlen(fileName);
  unsigned long long value = powInt(length, 4);

  for (int i = 0; i < length; i++) {
    value += i * i * i * i * fileName[i];
  }

  return value;
}

void printToNcursesNCharFromString(WINDOW* w, char* str, int n) {
  for (int i = 0; i < n && str[i] != '\0'; i++) {
    wprintw(w, "%c", str[i]);
  }
}


char* whereis(char* prog) {
  char command[15 + strlen(prog)];
  sprintf(command, "whereis %s", prog);

  FILE* f = popen(command, "r");
  if (f == NULL) {
    return NULL;
  }

  char* path = malloc(PATH_MAX);
  char tmp_shit[PATH_MAX];
  int scan_res = fscanf(f, " %s %s ", tmp_shit, path);
  if (scan_res != 2) {
    free(path);
    return NULL;
  }
  fclose(f);

  return path;
}


void getLocalURI(char* realive_abs_path, char* uri) {
  char abs_path[PATH_MAX];
  realpath(realive_abs_path, abs_path);
  sprintf(uri, "file://%s", abs_path);
}
