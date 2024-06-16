//
// Created by arnaud on 31/03/24.
//

#ifndef TOOLS_H
#define TOOLS_H
#include <ncurses.h>

typedef long long time_val;

time_val timeInMilliseconds(void);

time_val diff2Time(time_val start, time_val end);

int min(int a, int b);

int max(int a, int b);

int numberOfDigitOfNumber(int n);

unsigned long long hashFileName(char* fileName);

void printToNcursesNCharFromString(WINDOW *w, char *str, int n);

#endif //TOOLS_H
