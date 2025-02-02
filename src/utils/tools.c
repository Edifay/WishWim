#include "tools.h"

#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include <sys/stat.h>
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

bool isDir(char* path) {
  struct stat file_info;
  stat(path, &file_info);
  return S_ISDIR(file_info.st_mode);
}

bool getLanguageForFile(char* lang_name, IO_FileID io_file) {

  /// ---- FILE NAME ----

  // Make
  if (strcmp(basename(io_file.path_abs), "Makefile") == 0) {
    strcpy(lang_name, "make");
    return true;
  }

  // Bash
  if (strcmp(basename(io_file.path_abs), "config") == 0 || strcmp(basename(io_file.path_abs), ".bashrc") == 0) {
    strcpy(lang_name, "bash");
    return true;
  }

  /// ---- FILE EXTENSION ----

  // extracting extension
  char* dot = strrchr(io_file.path_args, '.');
  if (dot != NULL)
    strncpy(lang_name, dot + 1, 99);

  // ADD_NEW_LANGUAGE

  // c
  if (strcmp(lang_name, "h") == 0 || strcmp(lang_name, "c") == 0) {
    strcpy(lang_name, "c");
    return true;
  }
  // python
  if (strcmp(lang_name, "py") == 0) {
    strcpy(lang_name, "python");
    return true;
  }
  // markdown
  if (strcmp(lang_name, "md") == 0) {
    strcpy(lang_name, "markdown");
    return true;
  }
  // java
  if (strcmp(lang_name, "java") == 0) {
    strcpy(lang_name, "java");
    return true;
  }
  // c++
  if (strcmp(lang_name, "cpp") == 0 || strcmp(lang_name, "cc") == 0) {
    strcpy(lang_name, "cpp");
    return true;
  }
  // c#
  if (strcmp(lang_name, "cs") == 0) {
    strcpy(lang_name, "c-sharp");
    return true;
  }
  // css / scss
  if (strcmp(lang_name, "css") == 0 || strcmp(lang_name, "scss") == 0) {
    strcpy(lang_name, "css");
    return true;
  }
  // dart
  if (strcmp(lang_name, "dart") == 0) {
    strcpy(lang_name, "dart");
    return true;
  }
  // go-lang
  if (strcmp(lang_name, "go") == 0) {
    strcpy(lang_name, "go");
    return true;
  }
  // java-script
  if (strcmp(lang_name, "js") == 0) {
    strcpy(lang_name, "javascript");
    return true;
  }
  // json
  if (strcmp(lang_name, "json") == 0) {
    strcpy(lang_name, "json");
    return true;
  }
  // bash/shell
  if (strcmp(lang_name, "sh") == 0) {
    strcpy(lang_name, "bash");
    return true;
  }
  // scheme implementation
  if (strcmp(lang_name, "scm") == 0) {
    strcpy(lang_name, "query");
    return true;
  }
  // vhdl
  if (strcmp(lang_name, "vhd") == 0) {
    strcpy(lang_name, "vhdl");
    return true;
  }

  return false;
}


// copy from http://www.cse.yorku.ca/~oz/hash.html
int hashString(unsigned char* str) {
  unsigned long hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}
