#ifndef NCURSES_HANDLER_H
#define NCURSES_HANDLER_H
#include <wchar.h>
#include <ncurses.h>


#include "../data-management/file_management.h"
#include "../data-management/file_structure.h"
#include "../io_management/io_explorer.h"

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);


////// -------------- WINDOWS MANAGEMENTS --------------

typedef struct {
  // Init GUI vars
  WINDOW* ftw; // File Text Window
  WINDOW* lnw ; // Line Number Window
  WINDOW* ofw; // Opened Files Window
  WINDOW* few; // File Explorer Window
  bool refresh_edw; // Need to reprint editor window
  bool refresh_ofw; // Need to reprint opened file window
  bool refresh_few; // Need to reprint file explorer window
  WINDOW* focus_w; // Used to set the window where start mouse drag

  // EDW Datas

  // OFW Datas
  int current_file_offset;
  int ofw_height; // Height of Opened Files Window. 0 => Disabled on start.   OPENED_FILE_WINDOW_HEIGHT => Enabled on start.

  // Few Datas
  int few_width; // File explorer width
  int saved_few_width;
  int few_x_offset; /* TODO unused */
  int few_y_offset; // Y Scroll state of File Explorer Window
  int few_selected_line;
} GUIContext;


void initGUIContext(GUIContext* gui_context);

void initNCurses(GUIContext* gui_context);
////// -------------- PRINT FUNCTIONS --------------

void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch);

void printEditor(GUIContext *gui_context, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void printOpenedFile(GUIContext* gui_context, FileContainer* files, int file_count, int current_file);

void printFileExplorer(GUIContext *gui_context, ExplorerFolder* pwd);


////// -------------- RESIZE FUNCTIONS --------------

void resizeEditorWindows(GUIContext *gui_context, int lnw_new_width);

void resizeOpenedFileWindow(GUIContext *gui_context);

void switchShowFew(GUIContext *gui_context);

////// -------------- UTILS FUNCTIONS --------------

void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

int getScreenXForCursor(Cursor cursor, int screen_x);

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click);

void setDesiredColumn(Cursor cursor, int* desired_column);


#endif //NCURSES_HANDLER_H
