#include "file_management.h"
#include "../utils/tools.h"

////// -------------- CURSOR MANAGEMENT --------------

Cursor moveRight(Cursor cursor) {
  if (cursor.line_id.line->element_number != cursor.line_id.relative_column || isEmptyLine(cursor.line_id.line->next) ==
      false) {
    cursor.line_id.relative_column++;
    cursor.line_id.absolute_column++;
    cursor = moduloCursor(cursor);
  }
  else if (cursor.line_id.line->element_number == cursor.line_id.relative_column && isEmptyLine(
             cursor.line_id.line->next) == true) {
    if (cursor.file_id.file->element_number != cursor.file_id.relative_row || isEmptyFile(cursor.file_id.file->next) ==
        false) {
      cursor.file_id.relative_row++;
      cursor.file_id.absolute_row++;
      cursor = cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), 0));
    }
  }
  return cursor;
}

Cursor moveLeft(Cursor cursor) {
  if (cursor.line_id.relative_column != 0) {
    cursor.line_id.relative_column--;
    cursor.line_id.absolute_column--;
    cursor = moduloCursor(cursor);
  }
  else {
    if (cursor.file_id.file->prev != NULL || cursor.file_id.relative_row != 1) {
      cursor.file_id.relative_row--;
      cursor.file_id.absolute_row--;
      cursor = cursorOf(cursor.file_id,
                        moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id),
                                              sizeLineNode(getLineForFileIdentifier(cursor.file_id))));
    }
  }
  return cursor;
}

Cursor moveUp(Cursor cursor) {
  if (cursor.file_id.file->prev != NULL || cursor.file_id.relative_row != 1) {
    cursor.file_id.relative_row--;
    cursor.file_id.absolute_row--;
    int col = min(sizeLineNode(getLineForFileIdentifier(cursor.file_id)), getAbsoluteLineIndex(cursor.line_id));
    cursor = cursorOf(cursor.file_id, moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), col));
  }
  return cursor;
}

Cursor moveDown(Cursor cursor) {
  if (cursor.file_id.relative_row != cursor.file_id.file->element_number || isEmptyFile(
        cursor.file_id.file->next) == false) {
    cursor.file_id.relative_row++;
    cursor.file_id.absolute_row++;
    int col = min(sizeLineNode(getLineForFileIdentifier(cursor.file_id)), getAbsoluteLineIndex(cursor.line_id));
    cursor.line_id = moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), col);
    cursor = moduloCursor(cursor);
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
  while (hasElementAfterLine(cursor.line_id) && isInvisible(
           getCharForLineIdentifier((tmp_cur = moveRight(cursor)).line_id))) {
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
           (canWorkWithLetter = isALetter(getCharForLineIdentifier((tmp_cur = moveRight(cursor)).line_id)) &&
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
    repeated = getCharForLineIdentifier(moveLeft(cursor).line_id);
  }

  bool canWorkWithLetter = true;
  while (cursor.line_id.absolute_column != 0 && (
           (canWorkWithLetter = isALetter(getCharForLineIdentifier(cursor.line_id)) && canWorkWithLetter) || areChar_U8Equals(
             getCharForLineIdentifier(cursor.line_id), repeated))) {
    cursor = moveLeft(cursor);
  }

  if (areCursorEqual(old_cur, cursor)) {
    return moveLeft(cursor);
  }

  return cursor;
}


bool areCursorEqual(Cursor cur1, Cursor cur2) {
  return cur1.file_id.absolute_row == cur2.file_id.absolute_row && cur1.line_id.absolute_column == cur2.line_id.
         absolute_column;
}
