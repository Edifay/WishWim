#include "workspace_settings.h"

#include <assert.h>

#include "../config/config.h"

#include <string.h>
#include <sys/ttydefaults.h>
#include "../utils/constants.h"



void getWorkspaceSettingsForCurrentDir(WorkspaceSettings* settings, FileContainer* files, int file_count, int current_file, bool showing_opened_file_window,
                                       bool showing_file_explorer_window, int file_explorer_size) {
  settings->file_count = file_count;
  settings->current_opened_file = current_file;
  settings->files = malloc(file_count * sizeof(char *));
  for (int i = 0; i < file_count; i++) {
    int size = strlen(files[i].io_file.path_abs) + 1;
    settings->files[i] = malloc(size * sizeof(char));
    memcpy(settings->files[i], files[i].io_file.path_abs, size);
    settings->files[i][size - 1] = '\0';
  }
  settings->file_explorer_size = file_explorer_size;
  settings->showing_file_explorer_window = showing_file_explorer_window;
  settings->showing_opened_file_window = showing_opened_file_window;
}

void destroyWorkspaceSettings(WorkspaceSettings* settings) {
  for (int i = 0; i < settings->file_count; i++) {
    free(settings->files[i]);
  }
  free(settings->files);
}

void touchDirSettingsFolder() {
  char command[PATH_MAX + 100];
  sprintf(command, "mkdir -p ~/%s", CONFIG_FOLDER);
  system(command);
  sprintf(command, "mkdir -p ~/%s/%s", CONFIG_FOLDER, FOLDER_DIR_SETTINGS_NAME);
  system(command);
}

void saveWorkspaceSettings(char* dir_path, WorkspaceSettings* settings) {
  touchDirSettingsFolder();

  char abs_dir_path[PATH_MAX];
  realpath(dir_path, abs_dir_path);

  int hash_dir_path = hashFileName(abs_dir_path);

  sprintf(abs_dir_path, "%s/%s/%s/%d", getenv("HOME"), CONFIG_FOLDER, FOLDER_DIR_SETTINGS_NAME, hash_dir_path);

  FILE* f = fopen(abs_dir_path, "w");
  if (f == NULL) {
    fprintf(stderr, "Unable to save dir settings.\r\n");
    return;
  }

  cJSON* json_settings = WorkspaceSettingsToJSON(settings);
  char* json_text = cJSON_Print(json_settings);

  fprintf(f, "%s", json_text);

  cJSON_Delete(json_settings);

  free(json_text);
  fclose(f);
}

bool loadWorkspaceSettings(char* dir_path, WorkspaceSettings* settings) {
  char abs_dir_path[PATH_MAX];
  realpath(dir_path, abs_dir_path);

  int hash_dir_path = hashFileName(abs_dir_path);

  sprintf(abs_dir_path, "%s/%s/%s/%d", getenv("HOME"), CONFIG_FOLDER, FOLDER_DIR_SETTINGS_NAME, hash_dir_path);

  FILE* f = fopen(abs_dir_path, "r");
  if (f == NULL) {
    // init default values.
    settings->file_count = 0;
    settings->files = NULL;
    settings->showing_file_explorer_window = false;
    settings->showing_opened_file_window = false;
    settings->file_explorer_size = 0;
    // fprintf(stderr, "Unable to load dir settings.\r\n");
    return false;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char* file_content = malloc(fsize + 1);
  fread(file_content, fsize, 1, f);
  fclose(f);

  file_content[fsize] = 0;

  cJSON* json_settings = cJSON_Parse(file_content);

  free(file_content);

  JSONToWorkspaceSettings(settings, json_settings);

  cJSON_Delete(json_settings);

  return true;
}

cJSON* WorkspaceSettingsToJSON(WorkspaceSettings* settings) {
  cJSON* json_settings = cJSON_CreateObject();
  cJSON_AddNumberToObject(json_settings, "file_count", settings->file_count);
  cJSON_AddNumberToObject(json_settings, "current_opened_file", settings->current_opened_file);
  cJSON* file_array = cJSON_AddArrayToObject(json_settings, "files");
  for (int i = 0; i < settings->file_count; i++) {
    cJSON_AddItemToArray(file_array, cJSON_CreateString(settings->files[i]));
  }
  cJSON_AddBoolToObject(json_settings, "showing_opened_file_window", settings->showing_opened_file_window);
  cJSON_AddBoolToObject(json_settings, "showing_file_explorer_window", settings->showing_file_explorer_window);
  cJSON_AddNumberToObject(json_settings, "file_explorer_size", settings->file_explorer_size);

  return json_settings;
}

void JSONToWorkspaceSettings(WorkspaceSettings* settings, cJSON* json) {
  settings->file_count = cJSON_GetNumberValue(cJSON_GetObjectItem(json, "file_count"));
  settings->current_opened_file = cJSON_GetNumberValue(cJSON_GetObjectItem(json, "current_opened_file"));
  cJSON* file_array = cJSON_GetObjectItem(json, "files");
  settings->files = malloc(sizeof(char *) * settings->file_count);
  for (int i = 0; i < settings->file_count; i++) {
    char* value = cJSON_GetStringValue(cJSON_GetArrayItem(file_array, i));

    int size = strlen(value) + 1;
    settings->files[i] = malloc(size * sizeof(char));
    memcpy(settings->files[i], value, size);
    settings->files[i][size - 1] = '\0';
  }

  settings->showing_opened_file_window = cJSON_IsTrue(cJSON_GetObjectItem(json, "showing_opened_file_window"));
  settings->showing_file_explorer_window = cJSON_IsTrue(cJSON_GetObjectItem(json, "showing_file_explorer_window"));
  settings->file_explorer_size = cJSON_GetNumberValue(cJSON_GetObjectItem(json, "file_explorer_size"));
}

void setupWorkspace(WorkspaceSettings *loaded_settings, int* file_count, char*** file_names, GUIContext* gui_context, int* current_file_index) {
  loaded_settings->is_used = false;
  if (*file_count == 1 || *file_count == 0) {
    char* dir_name = *file_count == 0 ? getenv("PWD") : (*file_names)[0];

    if (isDir(dir_name)) {
      loaded_settings->dir_path = dir_name;
      loaded_settings->is_used = true;

      bool settings_exist = loadWorkspaceSettings(dir_name, loaded_settings);

      // consume dir name
      if (*file_count == 1) {
        (*file_count)--;
        (*file_names)++;
      }
      assert((*file_count) == 0);

      if (settings_exist) {
        // Setup opened files.
        *file_count = loaded_settings->file_count;
        *file_names = loaded_settings->files;


        // --- UI State ---

        // Current showed file.
        *current_file_index = loaded_settings->current_opened_file;

        // File Opened Window state.
        if (loaded_settings->showing_opened_file_window == true) {
          gui_context->ofw_height = OPENED_FILE_WINDOW_HEIGHT;
        }
        else {
          gui_context->ofw_height = 0;
        }

        // File Explorer Window state.
        if (loaded_settings->showing_file_explorer_window == true) {
          ungetch(CTRL('e'));
        }
      }
    }
  }
}