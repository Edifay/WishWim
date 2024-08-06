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
} WorkspaceSettings;


void getWorkspaceSettingsForCurrentDir(WorkspaceSettings* settings, FileContainer* files, int file_count, bool showing_opened_file_window,
                                 bool showing_file_explorer_window, int file_explorer_size);

void destroyWorkspaceSettings(WorkspaceSettings* settings);

void saveWorkspaceSettings(char* dir_path, WorkspaceSettings* settings);

bool loadWorkspaceSettings(char* dir_path, WorkspaceSettings* settings);

cJSON* WorkspaceSettingsToJSON(WorkspaceSettings* settings);

void JSONToWorkspaceSettings(WorkspaceSettings* settings, cJSON* json);


#endif //DIR_SETTINGS_H
