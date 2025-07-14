#ifndef CONFIG_H
#define CONFIG_H
#include <stdbool.h>

#include "../../lib/cJSON/cJSON.h"

#define CONFIG_PATH ".config/al/config"
#define CONFIG_FOLDER ".config/al/"
#define CONFIG_FILE_NAME "config"

#define DEFAULT_CONFIG "{\n    \"default_path\": \"%s/.config/al\"\n}\n"

// TODO implement a config file for theme and parser highlight.scm. Using JSON.
bool configExist();

void touchConfig();

cJSON *loadConfig();

#endif //CONFIG_H
