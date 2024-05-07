#ifndef FILE_HISTORY_H
#define FILE_HISTORY_H

#define FILE_HISTORY_PATH "/.config/al/.file_history/"


void getLastFilePosition(char* fileName, int* row, int* column, int *screen_x, int *screen_y);

void setlastFilePosition(char* fileName, int row, int column, int screen_x, int screen_y);

unsigned long long hashFileName(char* fileName);

#endif //FILE_HISTORY_H
