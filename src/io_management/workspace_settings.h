#ifndef DIR_SETTINGS_H
#define DIR_SETTINGS_H
#include <stdbool.h>

#include "../../lib/cJSON/cJSON.h"
#include "../data-management/file_management.h"
#include "../terminal/term_handler.h"

#define FOLDER_DIR_SETTINGS_NAME ".workspace_settings"

typedef struct {
  // Is used
  bool is_used;

  // UI Layout
  bool showing_opened_file_window;
  bool showing_file_explorer_window;
  int file_explorer_size;

  // Opened files.
  char** files;
  int file_count;
  int current_opened_file;

  // Dir path
  char* dir_path;
} WorkspaceSettings;




void getWorkspaceSettingsForCurrentDir(WorkspaceSettings* settings, FileContainer* files, int file_count, int current_file, bool showing_opened_file_window,
                                       bool showing_file_explorer_window, int file_explorer_size);

void destroyWorkspaceSettings(WorkspaceSettings* settings);

void saveWorkspaceSettings(char* dir_path, WorkspaceSettings* settings);

bool loadWorkspaceSettings(char* dir_path, WorkspaceSettings* settings);

cJSON* WorkspaceSettingsToJSON(WorkspaceSettings* settings);

void JSONToWorkspaceSettings(WorkspaceSettings* settings, cJSON* json);

void setupWorkspace(WorkspaceSettings *loaded_settings, int* file_count, char*** file_names, GUIContext* gui_context, int* current_file_index) ;


#endif //DIR_SETTINGS_H
