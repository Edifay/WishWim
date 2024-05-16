
#include <ncurses.h>

#include "key_management.h"
#include "tools.h"

time_val lastPress = 0;
int last_press_x = 0;
int last_press_y = 0;

time_val lastRelease = 0;
time_val lastClick = 0;
int last_clicked_x = 0;
int last_clicked_y = 0;



void detectComplexEvents(MEVENT* event) {
  time_val current_time = timeInMilliseconds();
  // if (event->bstate & (BUTTON1_PRESSED | BUTTON1_RELEASED)) {
  // event->bstate |= BUTTON1_CLICKED;
  // }

  if (event->bstate & BUTTON1_RELEASED || event->bstate & BUTTON1_PRESSED) {
    if (diff2Time(current_time, lastPress) < TIME_BETWEEN_EVENT && event->x == last_press_x && event->y == last_press_y) {
      event->bstate |= BUTTON1_CLICKED;
    }
  }

  if (event->bstate & BUTTON1_CLICKED) {
    if (diff2Time(current_time, lastClick) < 2 * TIME_BETWEEN_EVENT && event->x == last_clicked_x && event->y == last_clicked_y) {
      event->bstate |= BUTTON1_DOUBLE_CLICKED;
      event->bstate = event->bstate & ~BUTTON1_CLICKED;
    }
  }
  // write(STDOUT_FILENO, "\x1b[2J", 4);
  // char text[20];
  // sprintf(text, "\x1b[%d;%dH", 0, 0);
  // write(STDOUT_FILENO, text, strlen(text));


  if (event->bstate & BUTTON1_PRESSED) {
    // printf("Pressed\n\r");
    lastPress = current_time;
    last_press_x = event->x;
    last_press_y = event->y;
  }
  if (event->bstate & BUTTON1_RELEASED) {
    // printf("Released\r\n");
    lastRelease = current_time;
  }
  if (event->bstate & BUTTON1_CLICKED) {
    // printf("Detected click !\n\r");
    lastClick = current_time;
    last_clicked_x = event->x;
    last_clicked_y = event->y;
  }
  // if (event->bstate & BUTTON1_DOUBLE_CLICKED) {
    // printf("Detected double click !\n\r");
    // lastClick = 0;
  // }
}
