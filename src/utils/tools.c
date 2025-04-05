#include "tools.h"

#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <asm-generic/errno-base.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <sys/time.h>

bool areStringEquals(String str1, String str2) {
  return strcmp(str1.content, str2.content) == 0;
}


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

bool getLanguageStringIDForFile(char* lang_id, IO_FileID io_file) {
  /// ---- FILE NAME ----

  if (io_file.status == NONE) {
    return false;
  }

  // Make
  if (strcmp(basename(io_file.path_abs), "Makefile") == 0) {
    strcpy(lang_id, "make");
    return true;
  }

  // Bash
  if (strcmp(basename(io_file.path_abs), "config") == 0 || strcmp(basename(io_file.path_abs), ".bashrc") == 0) {
    strcpy(lang_id, "bash");
    return true;
  }

  /// ---- FILE EXTENSION ----

  // extracting extension
  char* dot = strrchr(io_file.path_args, '.');
  if (dot != NULL)
    strncpy(lang_id, dot + 1, 99);

  // ADD_NEW_LANGUAGE

  // c
  if (strcmp(lang_id, "h") == 0 || strcmp(lang_id, "c") == 0) {
    strcpy(lang_id, "c");
    return true;
  }
  // python
  if (strcmp(lang_id, "py") == 0) {
    strcpy(lang_id, "python");
    return true;
  }
  // markdown
  if (strcmp(lang_id, "md") == 0) {
    strcpy(lang_id, "markdown");
    return true;
  }
  // java
  if (strcmp(lang_id, "java") == 0) {
    strcpy(lang_id, "java");
    return true;
  }
  // c++
  if (strcmp(lang_id, "cpp") == 0 || strcmp(lang_id, "cc") == 0) {
    strcpy(lang_id, "cpp");
    return true;
  }
  // c#
  if (strcmp(lang_id, "cs") == 0) {
    strcpy(lang_id, "c-sharp");
    return true;
  }
  // css / scss
  if (strcmp(lang_id, "css") == 0 || strcmp(lang_id, "scss") == 0) {
    strcpy(lang_id, "css");
    return true;
  }
  // dart
  if (strcmp(lang_id, "dart") == 0) {
    strcpy(lang_id, "dart");
    return true;
  }
  // go-lang
  if (strcmp(lang_id, "go") == 0) {
    strcpy(lang_id, "go");
    return true;
  }
  // java-script
  if (strcmp(lang_id, "js") == 0) {
    strcpy(lang_id, "javascript");
    return true;
  }
  // json
  if (strcmp(lang_id, "json") == 0) {
    strcpy(lang_id, "json");
    return true;
  }
  // bash/shell
  if (strcmp(lang_id, "sh") == 0 || strcmp(lang_id, "conf") == 0) {
    strcpy(lang_id, "bash");
    return true;
  }
  // scheme implementation
  if (strcmp(lang_id, "scm") == 0) {
    strcpy(lang_id, "query");
    return true;
  }
  // vhdl
  if (strcmp(lang_id, "vhd") == 0) {
    strcpy(lang_id, "vhdl");
    return true;
  }
  // lua
  if (strcmp(lang_id, "lua") == 0) {
    strcpy(lang_id, "lua");
    return true;
  }
  // asm
  if (strcmp(lang_id, "s") == 0) {
    strcpy(lang_id, "asm");
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


char* loadFullFile(const char* path, long* length) {
  FILE* f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  *length = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char* string = malloc(*length + 1);
  fread(string, *length, 1, f);
  fclose(f);

  string[*length] = 0;

  return string;
}


// Fonction qui crée récursivement les répertoires comme `mkdir -p`
int mkdir_p(const char *path, mode_t mode) {
  char tmp[1024];
  char *p = NULL;
  size_t len;

  snprintf(tmp, sizeof(tmp), "%s", path);
  len = strlen(tmp);
  if (tmp[len - 1] == '/')
    tmp[len - 1] = '\0';

  for (p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';
      if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
        perror("mkdir");
        return -1;
      }
      *p = '/';
    }
  }
  if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
    perror("mkdir");
    return -1;
  }
  return 0;
}