#ifndef DIR_SETTINGS_H
#define DIR_SETTINGS_H
#include <stdbool.h>
#include <cjson/cJSON.h>

#include "../data-management/file_management.h"

#define FOLDER_DIR_SETTINGS_NAME ".dir_settings"

typedef struct {
  // UI Layout
  bool showing_opened_file_window;
  bool showing_file_explorer_window;
  int file_explorer_size;

  // Opened files.
  char** files;
  int file_count;

  // Dir path
  char *dir_path;
} DirSettings;


void getDirSettingsForCurrentDir(DirSettings* settings, FileContainer* files, int file_count, bool showing_opened_file_window,
                                 bool showing_file_explorer_window, int file_explorer_size);

void destroyDirSettings(DirSettings* settings);

void saveDirSettings(char* dir_path, DirSettings* settings);

bool loadDirSettings(char* dir_path, DirSettings* settings);

cJSON* DirSettingsToJSON(DirSettings* settings);

void JSONToDirSettings(DirSettings* settings, cJSON* json);


#endif //DIR_SETTINGS_H
