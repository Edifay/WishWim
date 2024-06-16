#ifndef FILE_HISTORY_H
#define FILE_HISTORY_H

#include "io_manager.h"
#include "../data-structure/file_management.h"


#define FILE_HISTORY_PATH "/.config/al/.file_history/"


void getLastFilePosition(char* fileName, int* row, int* column, int *screen_x, int *screen_y);

void setlastFilePosition(char* fileName, int row, int column, int screen_x, int screen_y);

void fetchSavedCursorPosition(IO_FileID file, Cursor* cursor, int* screen_x, int* screen_y);

#endif //FILE_HISTORY_H
