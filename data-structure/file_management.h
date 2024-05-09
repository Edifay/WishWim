#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H

#include "file_structure.h"


typedef enum {
  SELECT_OFF_RIGHT,
  SELECT_OFF_LEFT
} SELECT_OFF_STYLE;


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

////// -------------- SELECTION MANAGEMENT --------------


bool isCursorPreviousThanOther(Cursor cursor, Cursor other);
bool isCursorBetweenOthers(Cursor cursor, Cursor cur1, Cursor cur2);
bool areCursorEqual(Cursor cur1, Cursor cur2);
bool isCursorDisabled(Cursor cursor);

Cursor disableCursor(Cursor cursor);

void setSelectCursorOn(Cursor cursor, Cursor* select_cursor);
void setSelectCursorOff(Cursor* cursor, Cursor* select_cursor, SELECT_OFF_STYLE style);

void selectWord(Cursor* cursor, Cursor* select_cursor);

void deleteSelection(Cursor* cursor, Cursor* select_cursor);

#endif //FILE_MANAGEMENT_H
