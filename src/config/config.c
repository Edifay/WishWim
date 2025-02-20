#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>


bool configExist() {
  char path[PATH_MAX];
  sprintf(path, "%s/%s", getenv("HOME"), CONFIG_PATH);

  FILE* f = fopen(path, "r");
  if (f == NULL)
    return false;
  fclose(f);
  return true;
}

void touchConfig() {
  char command[PATH_MAX + 100];
  sprintf(command, "mkdir -p ~/%s", CONFIG_FOLDER);
  system(command);
  sprintf(command, "touch ~/%s", CONFIG_PATH);
  system(command);
}

cJSON* loadConfig() {
  if (configExist() == false) {
    touchConfig();

    char path[PATH_MAX];
    sprintf(path, "%s/%s", getenv("HOME"), CONFIG_PATH);

    FILE* f = fopen(path, "w");
    if (f == NULL) {
      printf("ERROR opening config file.\n");
    }
    fprintf(f, "%s", DEFAULT_CONFIG);
    fclose(f);
  }

  char path[PATH_MAX];
  sprintf(path, "%s/%s", getenv("HOME"), CONFIG_PATH);

  FILE* f = fopen(path, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char* file_content = malloc(fsize + 1);
  fread(file_content, fsize, 1, f);
  fclose(f);

  file_content[fsize] = 0;

  cJSON* json = cJSON_Parse(file_content);

  free(file_content);
  return json;
}
