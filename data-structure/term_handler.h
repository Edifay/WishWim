#ifndef NCURSES_HANDLER_H
#define NCURSES_HANDLER_H
#include <wchar.h>
#include <ncurses.h>


#include "file_management.h"
#include "file_structure.h"
#include "../io_management/io_explorer.h"
#include "../advanced/theme.h"

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);

////// -------------- PRINT FUNCTIONS --------------

void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch);

void printEditor(WINDOW* ftw, WINDOW* lnw, WINDOW* ofw, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void printOpenedFile(FileContainer* files, int file_count, int current_file, int current_file_offset, WINDOW* ofw);

void printFileExplorer(ExplorerFolder* pwd, WINDOW* few, int few_x_offset, int few_y_offset, int selected_line);


////// -------------- CLICK FUNCTIONS --------------

void handleEditorClick(int edws_offset_x, int edws_offset_y, Cursor* cursor, Cursor* select_cursor, int* desired_column, int* screen_x, int* screen_y, MEVENT* m_event,
                       bool button1_down);

void handleOpenedFileClick(FileContainer* files, int* file_count, int* current_file, MEVENT m_event, int* current_file_offset, WINDOW* ofw, bool* refresh_ofw,
                           bool* refresh_local_vars, bool mouse_drag);

void handleFileExplorerClick(FileContainer** files, int* file_count, int* current_file, ExplorerFolder* pwd, int* few_y_offset, int* few_x_offset, int* few_width,
                             int* few_selected_line, int edws_offset_y, WINDOW** few, WINDOW** ofw, WINDOW** lnw, WINDOW** ftw, MEVENT m_event, bool* refresh_few,
                             bool* refresh_ofw, bool* refresh_edw, bool* refresh_local_vars);

// true if found, false if not. if file_index == -1 => Res_folder was clicked. If file_index != -1 => file clicked is res_folder.files[file_index].
bool getFileClickedFileExplorer(ExplorerFolder* pwd, int y_click, int few_x_offsset, int few_y_offset, ExplorerFolder** res_folder, int* file_index);

////// -------------- RESIZE FUNCTIONS --------------

void resizeEditorWindows(WINDOW** ftw, WINDOW** lnw, int y_file_editor, int lnw_width, int few_width);

void resizeOpenedFileWindow(WINDOW** ofw, bool* refresh_ofw, int edws_offset_y, int few_width);

////// -------------- UTILS FUNCTIONS --------------

void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

int getScreenXForCursor(Cursor cursor, int screen_x);

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click);

void setDesiredColumn(Cursor cursor, int* desired_column);


////// ---------------- COLOR FUNCTIONS ---------------

void initColorsForTheme(HighlightThemeList theme_list, int* color_index, int* color_pair);


void highlightFilePart(WINDOW* ftw, int start_row, int start_column, int length, attr_t attr, NCURSES_PAIRS_T color, Cursor cursor, Cursor select, Cursor* tmp, int screen_y,
                       int screen_x);

#endif //NCURSES_HANDLER_H
