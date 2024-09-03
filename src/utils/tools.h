//
// Created by arnaud on 31/03/24.
//

#ifndef TOOLS_H
#define TOOLS_H
#include <ncurses.h>

#include "../io_management/io_manager.h"

typedef long long time_val;

time_val timeInMilliseconds(void);

time_val diff2Time(time_val start, time_val end);

int min(int a, int b);

int max(int a, int b);

int numberOfDigitOfNumber(int n);

unsigned long long hashFileName(char* fileName);

void printToNcursesNCharFromString(WINDOW* w, char* str, int n);

char* whereis(char* prog);

void getLocalURI(char* realive_abs_path, char* uri);

bool isDir(char *path);

bool getLanguageForFile(char *lang, IO_FileID io_file);

#endif //TOOLS_H
