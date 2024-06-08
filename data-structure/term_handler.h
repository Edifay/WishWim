#ifndef NCURSES_HANDLER_H
#define NCURSES_HANDLER_H
#include <wchar.h>
#include <ncurses.h>


#include "file_management.h"
#include "file_structure.h"
#include "../io_management/io_manager.h"
#include "../io_management/io_explorer.h"


Cursor createRoot(IO_FileID file);

void printChar_U8ToNcurses(WINDOW* w, Char_U8 ch);

void printEditor(WINDOW* ftw, WINDOW* lnw, WINDOW* ofw, Cursor cursor, Cursor select_cursor, int screen_x, int screen_y);

void moveScreenToMatchCursor(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

void centerCursorOnScreen(WINDOW* w, Cursor cursor, int* screen_x, int* screen_y);

void handleEditorClick(int edws_offset_x, int edws_offset_y, Cursor* cursor, Cursor* select_cursor, int* desired_column, int* screen_x, int* screen_y, MEVENT* m_event,
                       bool* button1_down);

int getScreenXForCursor(Cursor cursor, int screen_x);

LineIdentifier getLineIdForScreenX(LineIdentifier line_id, int screen_x, int x_click);

void printOpenedFile(int argc, FileContainer* files, int max_opened_file, int current_file, int current_file_offset, WINDOW* ofw);

void printFileExplorer(ExplorerFolder* pwd, WINDOW* few);

// true if found, false if not. if file_index == -1 => Res_folder was clicked. If file_index != -1 => file clicked is res_folder.files[file_index].
bool getClickedFile(ExplorerFolder* pwd, int y_click, ExplorerFolder** res_folder, int* file_index);

void setDesiredColumn(Cursor cursor, int* desired_column);

void resizeEditorWindows(WINDOW** ftw, WINDOW** lnw, int y_file_editor, int lnw_width, int few_width);

/* Unix call, use 'man wcwidth' to see explication. */
int wcwidth(const wint_t wc);

#endif //NCURSES_HANDLER_H
