#ifndef KEY_MANAGEMENT_H
#define KEY_MANAGEMENT_H
#include <ncurses.h>

#endif //KEY_MANAGEMENT_H

typedef long long time_val;

#define TIME_BETWEEN_EVENT 200 /*ms*/

// KEY_BIND

#define KEY_CTRL_DELETE 8
#define KEY_SUPPR 330
#define KEY_TAB '\t'
#define KEY_CTRL_MAJ_SLASH 31

#define KEY_MAJ_RIGHT 402
#define KEY_MAJ_LEFT 393
#define KEY_MAJ_UP 337
#define KEY_MAJ_DOWN 336
#define KEY_CTRL_RIGHT 567
#define KEY_CTRL_LEFT 552
#define KEY_CTRL_DOWN 532
#define KEY_CTRL_UP 573
#define KEY_CTRL_MAJ_RIGHT 568
#define KEY_CTRL_MAJ_LEFT 553
#define KEY_CTRL_MAJ_DOWN 533
#define KEY_CTRL_MAJ_UP 574


// MOUSE BIND

#define BEGIN_MOUSE_LISTEN 588
#define MOUSE_IN_OUT 589

void detectComplexEvents(MEVENT* event);
