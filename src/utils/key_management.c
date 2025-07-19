#include <ncurses.h>

#include "key_management.h"

#include "constants.h"
#include "tools.h"

time_val lastPress = 0;
int last_press_x = 0;
int last_press_y = 0;

time_val lastRelease = 0;
time_val lastClick = 0;
int last_clicked_x = 0;
int last_clicked_y = 0;

void printIfPresent(MEVENT* event, int value, char* print) {
  if (event->bstate & value) {
    fprintf(stderr, "%s, ", print);
  }
}

void printEventList(MEVENT* event) {
  printIfPresent(event, BUTTON1_PRESSED, "BUTTON1_PRESSED");
  printIfPresent(event, BUTTON1_RELEASED, "BUTTON1_RELEASED");
  printIfPresent(event, BUTTON1_CLICKED, "BUTTON1_CLICKED");
  printIfPresent(event, BUTTON1_DOUBLE_CLICKED, "BUTTON1_DOUBLE_CLICKED");


  printIfPresent(event, BUTTON2_PRESSED, "BUTTON2_PRESSED");
  printIfPresent(event, BUTTON2_RELEASED, "BUTTON2_RELEASED");
  printIfPresent(event, BUTTON2_CLICKED, "BUTTON2_CLICKED");
  printIfPresent(event, BUTTON2_DOUBLE_CLICKED, "BUTTON2_DOUBLE_CLICKED");


  printIfPresent(event, BUTTON3_PRESSED, "BUTTON3_PRESSED");
  printIfPresent(event, BUTTON3_RELEASED, "BUTTON3_RELEASED");
  printIfPresent(event, BUTTON3_CLICKED, "BUTTON3_CLICKED");

  printIfPresent(event, BUTTON4_PRESSED, "BUTTON4_PRESSED");
  printIfPresent(event, BUTTON4_RELEASED, "BUTTON4_RELEASED");
  printIfPresent(event, BUTTON4_CLICKED, "BUTTON4_CLICKED");

  printIfPresent(event, BUTTON5_PRESSED, "BUTTON5_PRESSED");
  printIfPresent(event, BUTTON5_RELEASED, "BUTTON5_RELEASED");
  printIfPresent(event, BUTTON5_CLICKED, "BUTTON5_CLICKED");
}


// Hard to implement bc there are different behaviour depend on mouse or touchpad is used.
void detectComplexMouseEvents(MEVENT* event) {
  time_val current_time = timeInMilliseconds();

  // fprintf(stderr, "BEGIN > id: %d, pos: (%d, %d, %d), masks: ", event->id, event->x, event->y, event->z);
  // printEventList(event);
  // fprintf(stderr, "\n    ==>");

  event->bstate = event->bstate & ~BUTTON1_CLICKED;


  if (event->bstate & BUTTON1_PRESSED && event->bstate & BUTTON1_RELEASED) {
    event->bstate = event->bstate & ~BUTTON1_PRESSED;
    event->bstate = event->bstate & ~BUTTON1_RELEASED;
    event->bstate |= BUTTON1_CLICKED;
  }
  else if (event->bstate & BUTTON1_RELEASED || event->bstate & BUTTON1_PRESSED) {
    if (diff2Time(current_time, lastPress) < TIME_BETWEEN_EVENT && event->x == last_press_x && event->y == last_press_y) {
      if (event->bstate & BUTTON1_RELEASED && diff2Time(current_time, lastClick) < 2 * TIME_BETWEEN_EVENT && event->x == last_clicked_x && event->y == last_clicked_y) {
        event->bstate |= BUTTON1_DOUBLE_CLICKED;
        event->bstate = event->bstate & ~BUTTON1_CLICKED;
      }
      else {
        event->bstate |= BUTTON1_CLICKED;
        event->bstate = event->bstate & ~BUTTON1_PRESSED;
        event->bstate = event->bstate | BUTTON1_RELEASED;
      }
    }
  }

  // if (event->bstate & BUTTON1_CLICKED) {
  // if (diff2Time(current_time, lastClick) < 2 * TIME_BETWEEN_EVENT && event->x == last_clicked_x && event->y == last_clicked_y) {
  // event->bstate |= BUTTON1_DOUBLE_CLICKED;
  // event->bstate = event->bstate & ~BUTTON1_CLICKED;
  // }
  // }
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

    lastPress = current_time;
    last_press_x = event->x;
    last_press_y = event->y;
  }
  if (event->bstate & BUTTON1_DOUBLE_CLICKED) {
    // printf("Detected double click !\n\r");
    // lastClick = 0;
  }
  // printf("--------------\r\n");


  // fprintf(stderr, "Mouse_event : %d, (%d, %d, %d), id: %d\n", event->bstate, event->x, event->y, event->z, event->id);

  // TODO upgrade detectComplex event to release the scroll. On some machines mouse_mouvement keep sending the 'state' of a button
  // like if a button wasn't release the mouse mouve will keep emitting the button down.
  if (event->bstate & BUTTON4_PRESSED) {
    MEVENT tmp_event;
    tmp_event.bstate = BUTTON4_RELEASED;
    tmp_event.x = event->x;
    tmp_event.y = event->y;
    tmp_event.z = event->z;
    ungetmouse(&tmp_event);
    // fprintf(stderr, "Emitting :");
    // printEventList(&tmp_event);
    // fprintf(stderr, "\n");
  }

  if (event->bstate & BUTTON5_PRESSED) {
    MEVENT tmp_event;
    tmp_event.bstate = BUTTON5_RELEASED;
    tmp_event.x = event->x;
    tmp_event.y = event->y;
    tmp_event.z = event->z;
    ungetmouse(&tmp_event);
    // fprintf(stderr, "Emitting :");
    // printEventList(&tmp_event);
    // fprintf(stderr, "\n");
  }

  // fprintf(stderr, "END > id: %d, pos: (%d, %d, %d), masks: ", event->id, event->x, event->y, event->z);
  // printEventList(event);
  // fprintf(stderr, "\n");
}
