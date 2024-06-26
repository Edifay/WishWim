#ifndef CONTANTS_H
#define CONTANTS_H

#define SCROLL_SPEED 3
#define NO_EVENT_MOUSE 268435456


#define FILE_NAME_SEPARATOR " | "
#define OPENED_FILE_WINDOW_HEIGHT 2 // 2 to enable 0 to disable

// use make -B after changing the TAB_SIZE
#define TAB_CHAR_USE false
#define TAB_SIZE 2

#define FILE_EXPLORER_WIDTH 30
#define FILE_EXPLORER_TREE_OFFSET 1


#define COLOR_HOVER 8


#define DEFAULT_COLOR_PAIR 1
#define DEFAULT_COLOR_HOVER_PAIR 1001
#include <cjson/cJSON.h>
#include "../advanced/tree-sitter/tree_manager.h"


extern int color_index;
extern int color_pair;

extern cJSON *config;
extern ParserList parsers;


#endif //CONTANTS_H
