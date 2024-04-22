#ifndef FILE_MANAGEMENT_H
#define FILE_MANAGEMENT_H
#include "file_structure.h"


////// -------------- CURSOR ACTIONS --------------

Cursor moveRight(Cursor cursor);
Cursor moveLeft(Cursor cursor);
Cursor moveUp(Cursor cursor);
Cursor moveDown(Cursor cursor);

Cursor deleteCharAtCursor(Cursor cursor);
Cursor supprCharAtCursor(Cursor cursor);

Cursor deleteLineAtCursor(Cursor cursor);

Cursor skipRightInvisibleChar(Cursor cursor);
Cursor skipLeftInvisibleChar(Cursor cursor);


Cursor moveToNextWord(Cursor cursor);
Cursor moveToPreviousWord(Cursor cursor);

bool isCursorPreviousThanOther(Cursor cursor, Cursor other);

bool isCursorBetweenOthers(Cursor cursor, Cursor cur1, Cursor cur2);

bool areCursorEqual(Cursor cur1, Cursor cur2);


////// -------------- SELECTION MANAGEMENT --------------
// TODO Implement selection.


#endif //FILE_MANAGEMENT_H
