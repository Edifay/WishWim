#include <assert.h>
#include <stdlib.h>

#include "state_control.h"
#include "file_management.h"


void initHistory(History* history) {
  history->action.action = ACTION_NONE;
  history->action.time = 0;
  history->prev = NULL;
  history->next = NULL;
}


// TODO implement
Cursor undo(History** history_p, Cursor cursor) {
  History* history = *history_p;

  // If hisotory is at root return and do nothing. Cannot undo nothing ;).
  if (history->action.action == ACTION_NONE) {
    return cursor;
  }

  time_val canceled_time_action = history->action.time;
  // TODO implement keep reverse action to redo.
  cursor = doReverseAction(&history->action, cursor);
  history->action.time = canceled_time_action;

  *history_p = history->prev;

  if (diff2Time(history->action.time, history->prev->action.time) < TIME_CONSIDER_UNIQUE_UNDO) {
    return undo(history_p, cursor);
  }

  return cursor;
}

Cursor redo(History** history_p, Cursor cursor) {
  History* history = *history_p;

  // If hisotory is at root return and do nothing. Cannot undo nothing ;).
  if (history->next == NULL) {
    return cursor;
  }

  *history_p = history->next;
  history = *history_p;

  time_val canceled_time_action = history->action.time;
  // TODO implement keep reverse action to redo.
  cursor = doReverseAction(&history->action, cursor);
  history->action.time = canceled_time_action;


  if (history->next != NULL && diff2Time(history->action.time, history->next->action.time) < TIME_CONSIDER_UNIQUE_UNDO) {
    return redo(history_p, cursor);
  }

  return cursor;
}

// TODO implement
void saveAction(History** history_p, Action action) {
  History* history = *history_p;
  if (action.action == ACTION_NONE) {
    return;
  }

  destroyEndOfHistory(history);
  assert(history->next == NULL);

  assert(history != NULL);
  history->next = malloc(sizeof(History));

  assert(history->next != NULL);
  history->next->prev = history;
  history->next->next = NULL;
  history->next->action = action;

  history = history->next;

  *history_p = history;
}

Cursor doReverseAction(Action* action_p, Cursor cursor) {
  Action action = *action_p;
  Cursor tmp;
  ACTION_TYPE type = action.action;
  switch (type) {
    case DELETE_ONE:
      tmp.file_id = tryToReachAbsRow(cursor.file_id, action.cur.file_id.absolute_row);
      tmp.line_id = moduloLineIdentifierR(getLineForFileIdentifier(tmp.file_id), action.cur.line_id.absolute_column);
      if (action.unique_ch == '\n') {
        cursor = insertNewLineInLineC(tmp);
        destroyAction(action);
        *action_p = createInsertAction(tmp, cursor);
        return cursor;
      }
      cursor = insertCharInLineC(tmp, readChar_U8FromInput(action.unique_ch));
      destroyAction(action);
      *action_p = createInsertAction(tmp, cursor);
      return cursor;
    case DELETE:
      tmp.file_id = tryToReachAbsRow(cursor.file_id, action.cur.file_id.absolute_row);
      tmp.line_id = moduloLineIdentifierR(getLineForFileIdentifier(tmp.file_id), action.cur.line_id.absolute_column);
      cursor = insertCharArrayAtCursor(tmp, action.ch);
      destroyAction(action);
      *action_p = createInsertAction(tmp, cursor);
      return cursor;
    case INSERT:
      destroyAction(action);
      *action_p = createDeleteAction(action.cur, action.cur_end);
      deleteSelection(&action.cur, &action.cur_end);
      return action.cur;
    case ACTION_NONE:
      return cursor;
    default:
      assert(false);
  }
}

Action createDeleteAction(Cursor cur1, Cursor cur2) {
  if (isCursorDisabled(cur1) || isCursorDisabled(cur2)) {
    Action action;
    action.action = ACTION_NONE;
    action.time = 0;
    return action;
  }

  if (isCursorPreviousThanOther(cur2, cur1)) {
    Cursor tmp = cur1;
    cur1 = cur2;
    cur2 = tmp;
  }

  Action action;
  action.time = timeInMilliseconds();
  action.cur = cur1;
  // Not used.
  action.cur_end = disableCursor(cur1);

  Cursor it = cur1;
  int byte_between_2_cursor = 0;
  while (isCursorStrictPreviousThanOther(it, cur2)) {
    Cursor tmp = it;
    it = moveRight(it);

    if (hasElementBeforeLine(it.line_id) == true) {
      byte_between_2_cursor += sizeChar_U8(getCharForLineIdentifier(it.line_id));
    }
    else {
      assert(tmp.file_id.absolute_row != it.file_id.absolute_row);
      byte_between_2_cursor++; // Add one byte alloc to save '\n' char.
    }
  }

  if (byte_between_2_cursor == 0) {
    action.action = ACTION_NONE;
    action.time = 0;
    return action;
  }

  assert(byte_between_2_cursor != 0);

  // If there is only 1 element to save we don't use malloc. We use unique_ch.
  if (byte_between_2_cursor == 1) {
    action.action = DELETE_ONE;
    action.ch = NULL;
    if (hasElementBeforeLine(it.line_id) == true) {
      action.unique_ch = getCharForLineIdentifier(it.line_id).t[0];
    }
    else {
      action.unique_ch = '\n';
    }
    return action;
  }


  action.action = DELETE;
  action.unique_ch = '\0';
  action.ch = malloc(byte_between_2_cursor * sizeof(char) + 1 /*for EOF char*/);
  assert(action.ch != NULL);

  it = cur1;
  int index = 0;
  while (isCursorStrictPreviousThanOther(it, cur2)) {
    it = moveRight(it);

    if (hasElementBeforeLine(it.line_id) == true) {
      Char_U8 ch = getCharForLineIdentifier(it.line_id);
      int ch_size = sizeChar_U8(ch);
      for (int i = 0; i < ch_size; i++) {
        action.ch[index] = ch.t[i];
        index++;
      }
    }
    else {
      action.ch[index] = '\n';
      index++;
    }
  }

  action.ch[index] = '\0';

  return action;
}

Action createInsertAction(Cursor cur1, Cursor cur2) {
  Action action;
  action.action = INSERT;
  action.cur = cur1;
  action.cur_end = cur2;
  action.time = timeInMilliseconds();
  action.unique_ch = '\0';
  action.ch = NULL;
  return action;
}

void destroyAction(Action action) {
  assert(action.action != ACTION_NONE);
  free(action.ch);
}

void destroyEndOfHistory(History* history) {
  assert(history != NULL);
  History* tmp = history;

  history = history->next;
  while (history != NULL) {
    History* tmp = history;
    history = history->next;

    assert(tmp->action.action != ACTION_NONE);
    destroyAction(tmp->action);
    free(tmp);
  }

  tmp->next = NULL;
}
