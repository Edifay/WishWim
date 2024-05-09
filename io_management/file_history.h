#ifndef FILE_HISTORY_H
#define FILE_HISTORY_H

#include "../data-structure/file_management.h"


#define FILE_HISTORY_PATH "/.config/al/.file_history/"


void getLastFilePosition(char* fileName, int* row, int* column, int *screen_x, int *screen_y);

void setlastFilePosition(char* fileName, int row, int column, int screen_x, int screen_y);

unsigned long long hashFileName(char* fileName);

void fetchSavedCursorPosition(int argc, char** args, Cursor* cursor, int* screen_x, int* screen_y);

#endif //FILE_HISTORY_H
