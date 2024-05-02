#ifndef FILE_HISTORY_H
#define FILE_HISTORY_H

#define FILE_HISTORY_PATH "/.config/al/.file_history/"


void getLastFilePosition(char* fileName, int* row, int* column);

void setlastFilePosition(char* fileName, int row, int column);

unsigned long long hashFileName(char* fileName);

#endif //FILE_HISTORY_H
