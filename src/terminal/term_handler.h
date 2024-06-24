#ifndef NCURSES_HANDLER_H
#define NCURSES_HANDLER_H
#include <wchar.h>
#include <ncurses.h>


#include "../data-management/file_management.h"
#include "../data-management/file_structure.h"
#include "../io_management/io_explorer.h"
#include "../advanced/theme.h"

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);

////// -------------- PRINT FUNCTIONS --------------

void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch);

void printEditor(WINDOW* ftw, WINDOW* lnw, WINDOW* ofw, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void printOpenedFile(FileContainer* files, int file_count, int current_file, int current_file_offset, WINDOW* ofw);

void printFileExplorer(ExplorerFolder* pwd, WINDOW* few, int few_x_offset, int few_y_offset, int selected_line);


////// -------------- RESIZE FUNCTIONS --------------

void resizeEditorWindows(WINDOW** ftw, WINDOW** lnw, int y_file_editor, int lnw_width, int few_width);

void resizeOpenedFileWindow(WINDOW** ofw, bool* refresh_ofw, int edws_offset_y, int few_width);

////// -------------- UTILS FUNCTIONS --------------

void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

int getScreenXForCursor(Cursor cursor, int screen_x);

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click);

void setDesiredColumn(Cursor cursor, int* desired_column);


#endif //NCURSES_HANDLER_H
