#ifndef NCURSES_HANDLER_H
#define NCURSES_HANDLER_H
#include <wchar.h>
#include <ncurses.h>


#include "file_structure.h"
#include "../io_management/io_manager.h"


Cursor createRoot(IO_FileID file);

void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch);

void printFile(WINDOW* file_w, WINDOW* line_w, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

int getScreenXForCursor(Cursor cursor, int screen_x);

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click);

void setDesiredColumn(Cursor cursor, int* desired_column);

void resizeEditorWindows(WINDOW** file_w, WINDOW** line_w, int y_file_editor, int new_start_x);

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);

#endif //NCURSES_HANDLER_H
