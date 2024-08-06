#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

#include "file_structure.h"
#include "state_control.h"
#include "../advanced/tree-sitter/tree_manager.h"


typedef struct {
  IO_FileID io_file; // Describe the IO file on OS
  FileNode* root; // The root of the File object
  Cursor cursor; // The current cursor for the root File
  Cursor select_cursor; // The cursor used to make selection
  Cursor old_cur; // Old cursor used to flag cursor change
  int desired_column; // Used on line change to try to reach column
  int screen_x; // The x coord of the top left corner of the current viewport of the file
  int screen_y; // The y coord of the top left corner of the current viewport of the file
  int old_screen_x; // old screen_x used to flag screen_x changes
  int old_screen_y; // old screen_y used to flag screen_y changes
  History *history_root; // Root of History object for the current File
  History* history_frame; // Current node of the History. Before -> Undo, After -> Redo.
  FileHighlightDatas highlight_data; // Object which represent the highlight_data of the current file.
} FileContainer;


typedef enum {
  SELECT_OFF_RIGHT,
  SELECT_OFF_LEFT
} SELECT_OFF_STYLE;

////// -------------- FILE CONTAINER --------------

void destroyFileContainer(FileContainer *container);

void openNewFile(char* file_path, FileContainer** files, int* file_count, int* current_file, bool* refresh_ofw, bool* refresh_local_vars);

void closeFile(FileContainer** files, int* file_count, int* current_file, bool* refresh_ofw, bool* refresh_edw, bool* refresh_local_vars);

Cursor createRoot(IO_FileID file);

void setupFileContainer(char *args, FileContainer *container);

void setupLocalVars(FileContainer* files, int current_file, IO_FileID** io_file, FileNode*** root, Cursor** cursor, Cursor** select_cursor, Cursor** old_cur, int** desired_column,
                    int** screen_x, int** screen_y, int** old_screen_x, int** old_screen_y, History*** history_root, History*** history_frame, FileHighlightDatas **highlight_data);

bool isFileContainerEmpty(FileContainer *container);


////// -------------- CURSOR ACTIONS --------------

Cursor moveRight(Cursor cursor);
Cursor moveLeft(Cursor cursor);
Cursor moveUp(Cursor cursor, int desiredColumn);
Cursor moveDown(Cursor cursor, int desiredColumn);

Cursor deleteCharAtCursor(Cursor cursor);
Cursor supprCharAtCursor(Cursor cursor);

Cursor deleteLineAtCursor(Cursor cursor);

Cursor skipRightInvisibleChar(Cursor cursor);
Cursor skipLeftInvisibleChar(Cursor cursor);

Cursor moveToNextWord(Cursor cursor);
Cursor moveToPreviousWord(Cursor cursor);

Cursor insertCharArrayAtCursor(Cursor cursor, char* chs);

Cursor byteCursorToCursor(Cursor cursor, int row, int byte_column);

////// -------------- SELECTION MANAGEMENT --------------


bool isCursorPreviousThanOther(Cursor cursor, Cursor other);
bool isCursorStrictPreviousThanOther(Cursor cursor, Cursor other);
bool isCursorBetweenOthers(Cursor cursor, Cursor cur1, Cursor cur2);
bool areCursorEqual(Cursor cur1, Cursor cur2);
bool isCursorDisabled(Cursor cursor);

int charBetween2Curso(Cursor cur1, Cursor cur2);

Cursor disableCursor(Cursor cursor);

void setSelectCursorOn(Cursor cursor, Cursor* select_cursor);
void setSelectCursorOff(Cursor* cursor, Cursor* select_cursor, SELECT_OFF_STYLE style);

void selectWord(Cursor* cursor, Cursor* select_cursor);
void selectLine(Cursor *cursor, Cursor *select_cursor);

void deleteSelection(Cursor* cursor, Cursor* select_cursor);

void deleteSelectionWithHist(History **history_p, Cursor* cursor, Cursor* select_cursor);


#endif //FILE_MANAGEMENT_H
