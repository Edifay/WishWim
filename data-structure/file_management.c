#include "file_management.h"

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include "../utils/tools.h"

//// -------------- CURSOR MANAGEMENT --------------

Cursor moveRight(Cursor cursor) {
  if (hasElementAfterLine(cursor.line_id)) {
    cursor.line_id.relative_column++;
    cursor.line_id.absolute_column++;
    cursor = moduloCursor(cursor);
  }
  else if (hasElementAfterFile(cursor.file_id)) {
    // If cursor is at the end of the line try to go on next line.
    cursor.file_id.relative_row++;
    cursor.file_id.absolute_row++;
    cursor = cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0));
  }

  return cursor;
}

Cursor moveLeft(Cursor cursor) {
  if (hasElementBeforeLine(cursor.line_id)) {
    cursor.line_id.relative_column--;
    cursor.line_id.absolute_column--;
    cursor = moduloCursor(cursor);
  }
  else if (hasElementBeforeFile(cursor.file_id)) {
    // If the cursor is at the begin of the line try to reach previous line.
    cursor.file_id.relative_row--;
    cursor.file_id.absolute_row--;
    cursor = cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), sizeLineNode(getLineForFileIdentifier(cursor.file_id))));
  }

  return cursor;
}

Cursor moveUp(Cursor cursor, int desiredColumn) {
  if (hasElementBeforeFile(cursor.file_id)) {
    cursor.file_id.relative_row--;
    cursor.file_id.absolute_row--;
    cursor = cursorOf(cursor.file_id, tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0), desiredColumn));
  }

  return cursor;
}

Cursor moveDown(Cursor cursor, int desiredColumn) {
  if (hasElementAfterFile(cursor.file_id)) {
    cursor.file_id.relative_row++;
    cursor.file_id.absolute_row++;
    cursor = cursorOf(cursor.file_id, tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0), desiredColumn));
  }

  return cursor;
}

Cursor deleteCharAtCursor(Cursor cursor) {
  if (cursor.line_id.absolute_column == 0) {
    if (cursor.file_id.absolute_row != 1) {
      cursor = concatNeighbordsLinesC(cursor);
    }
  }
  else {
    cursor = removeCharInLineC(cursor);
  }
  return cursor;
}

Cursor supprCharAtCursor(Cursor cursor) {
  Cursor old_cur = cursor;
  cursor = moveRight(cursor);
  if (old_cur.file_id.absolute_row != cursor.file_id.absolute_row || old_cur.line_id.absolute_column !=
      cursor.line_id.absolute_column) {
    cursor = deleteCharAtCursor(cursor);
  }
  return cursor;
}

Cursor deleteLineAtCursor(Cursor cursor) {
  if (cursor.file_id.absolute_row != 1 || hasElementAfterFile(cursor.file_id)) {
    cursor = removeLineInFileC(cursor);
  }
  else {
    destroyFullLine(cursor.line_id.line);
    cursor.line_id = moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0);
  }
  return cursor;
}


Cursor skipRightInvisibleChar(Cursor cursor) {
  Cursor tmp_cur;
  while (hasElementAfterLine(cursor.line_id) && isInvisible(getCharForLineIdentifier((tmp_cur = moveRight(cursor)).line_id))) {
    cursor = tmp_cur;
  }

  return cursor;
}

Cursor skipLeftInvisibleChar(Cursor cursor) {
  while (cursor.line_id.absolute_column != 0 && isInvisible(getCharForLineIdentifier(cursor.line_id))) {
    cursor = moveLeft(cursor);
  }

  return cursor;
}

Cursor moveToNextWord(Cursor cursor) {
  cursor = skipRightInvisibleChar(cursor);
  Cursor old_cur = cursor;

  Char_U8 repeated;
  if (hasElementAfterLine(cursor.line_id)) {
    repeated = getCharForLineIdentifier(moveRight(cursor).line_id);
  }

  Cursor tmp_cur;
  bool canWorkWithLetter = true;
  while (hasElementAfterLine(cursor.line_id) && (
           (canWorkWithLetter = isAWordLetter(getCharForLineIdentifier((tmp_cur = moveRight(cursor)).line_id)) &&
                                canWorkWithLetter) || areChar_U8Equals(getCharForLineIdentifier(tmp_cur.line_id),
                                                                       repeated))) {
    cursor = tmp_cur;
  }

  if (areCursorEqual(old_cur, cursor)) {
    return moveRight(cursor);
  }

  return cursor;
}

Cursor moveToPreviousWord(Cursor cursor) {
  cursor = skipLeftInvisibleChar(cursor);
  Cursor old_cur = cursor;

  Char_U8 repeated;
  if (cursor.line_id.absolute_column != 0) {
    repeated = getCharForLineIdentifier(cursor.line_id);
  }

  bool canWorkWithLetter = true;
  while (cursor.line_id.absolute_column != 0 && (
           (canWorkWithLetter = isAWordLetter(getCharForLineIdentifier(cursor.line_id)) && canWorkWithLetter) ||
           areChar_U8Equals(
             getCharForLineIdentifier(cursor.line_id), repeated))) {
    cursor = moveLeft(cursor);
  }

  if (areCursorEqual(old_cur, cursor)) {
    return moveLeft(cursor);
  }

  return cursor;
}

Cursor insertCharArrayAtCursor(Cursor cursor, char* chs) {
  // Duplicated search in project DUP_SCAN.

  int index = 0;

  char c;
  while ((c = chs[index++]) != '\0') {
#ifdef LOGS
    assert(checkFileIntegrity(root) == true);
#endif
    if (iscntrl(c)) {
      if (c == '\n') {
#ifdef LOGS
        printf("Enter\r\n");
#endif
        cursor = insertNewLineInLineC(cursor);
      }
      else if (c == 9) {
#ifdef LOGS
        printf("Tab\r\n");
#endif
        Char_U8 ch;
        ch.t[0] = ' ';
        for (int i = 0; i < 4; i++) {
          cursor = insertCharInLineC(cursor, ch);
        }
      }
      else {
#ifdef LOGS
        printf("Unsupported Char loaded from file : '%d'.\r\n", c);
#endif
        // exit(0);
      }
    }
    else {
      Char_U8 ch = readChar_U8FromCharArrayWithFirst(chs + index -1 , c);
      index += sizeChar_U8(ch) - 1;
#ifdef LOGS
      printChar_U8(stdout, ch);
      printf("\r\n");
#endif
      cursor = insertCharInLineC(cursor, ch);
    }
  }


  return cursor;
}

////// -------------- SELECTION MANAGEMENT --------------

bool isCursorPreviousThanOther(Cursor cursor, Cursor other) {
  if (cursor.file_id.absolute_row < other.file_id.absolute_row)
    return true;
  if (cursor.file_id.absolute_row > other.file_id.absolute_row)
    return false;
  assert(cursor.file_id.absolute_row == other.file_id.absolute_row);

  return cursor.line_id.absolute_column <= other.line_id.absolute_column;
}

bool isCursorStrictPreviousThanOther(Cursor cursor, Cursor other) {
  if (cursor.file_id.absolute_row < other.file_id.absolute_row)
    return true;
  if (cursor.file_id.absolute_row > other.file_id.absolute_row)
    return false;
  assert(cursor.file_id.absolute_row == other.file_id.absolute_row);

  return cursor.line_id.absolute_column < other.line_id.absolute_column;
}

bool isCursorBetweenOthers(Cursor cursor, Cursor cur1, Cursor cur2) {
  if (isCursorPreviousThanOther(cur1, cur2) == false) {
    Cursor tmp = cur1;
    cur1 = cur2;
    cur2 = tmp;
  }

  int row = cursor.file_id.absolute_row;
  int column = cursor.line_id.absolute_column;

  int row_start = cur1.file_id.absolute_row;
  int column_start = cur1.line_id.absolute_column;

  int row_end = cur2.file_id.absolute_row;
  int column_end = cur2.line_id.absolute_column;


  return (row_start < row || (row_start == row && column_start < column))
         && (row < row_end || (row == row_end && column <= column_end));
}


bool areCursorEqual(Cursor cur1, Cursor cur2) {
  return cur1.file_id.absolute_row == cur2.file_id.absolute_row && cur1.line_id.absolute_column == cur2.line_id.
         absolute_column;
}


bool isCursorDisabled(Cursor cursor) {
  return cursor.file_id.absolute_row == -1;
}

int charBetween2Curso(Cursor cur1, Cursor cur2) {
  int count = 0;
  while (isCursorPreviousThanOther(cur1, cur2)) {
    cur1 = moveRight(cur1);
    count++;
  }
  return count;
}

Cursor disableCursor(Cursor cursor) {
  cursor.file_id.absolute_row = -1;
  return cursor;
}

void setSelectCursorOn(Cursor cursor, Cursor* select_cursor) {
  if (isCursorDisabled(*select_cursor) == true) {
    *select_cursor = cursor;
  }
}

void setSelectCursorOff(Cursor* cursor, Cursor* select_cursor, SELECT_OFF_STYLE style) {
  if (isCursorDisabled(*select_cursor) == true)
    return;


  if (style == SELECT_OFF_RIGHT && isCursorPreviousThanOther(*cursor, *select_cursor)) {
    Cursor tmp = *cursor;
    *cursor = *select_cursor;
    *select_cursor = tmp;
  }
  else if (style == SELECT_OFF_LEFT && isCursorPreviousThanOther(*select_cursor, *cursor)) {
    Cursor tmp = *cursor;
    *cursor = *select_cursor;
    *select_cursor = tmp;
  }

  *select_cursor = disableCursor(*select_cursor);
}


void selectWord(Cursor* cursor, Cursor* select_cursor) {
  if (cursor->line_id.absolute_column != 0 && isAWordLetter(getCharForLineIdentifier(cursor->line_id)) == true) {
    *cursor = moveToPreviousWord(*cursor);
  }
  setSelectCursorOn(*cursor, select_cursor);
  *cursor = moveToNextWord(*cursor);
}

void deleteSelection(Cursor* cursor, Cursor* select_cursor) {
  if (isCursorDisabled(*select_cursor) == true) {
    return;
  }

  if (isCursorPreviousThanOther(*select_cursor, *cursor)) {
    Cursor tmp = *select_cursor;
    *select_cursor = *cursor;
    *cursor = tmp;
  }

  assert(isCursorPreviousThanOther(*cursor, *select_cursor));

  if (cursor->file_id.absolute_row == select_cursor->file_id.absolute_row) {
    // Need to delete part of a line.
    deleteLinePart(cursor->line_id, select_cursor->line_id.absolute_column - cursor->line_id.absolute_column);
  }
  else {
    deleteLinePart(cursor->line_id, tryToReachAbsColumn(cursor->line_id, INT_MAX).absolute_column - cursor->line_id.absolute_column);
    deleteLinePart(tryToReachAbsColumn(select_cursor->line_id, 0), select_cursor->line_id.absolute_column);
    deleteFilePart(tryToReachAbsRow(cursor->file_id, cursor->file_id.absolute_row), select_cursor->file_id.absolute_row - cursor->file_id.absolute_row - 1);
    *cursor = moduloCursor(*cursor);
    *cursor = supprCharAtCursor(*cursor);
  }

  *select_cursor = disableCursor(*select_cursor);
}
