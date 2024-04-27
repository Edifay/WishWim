#include "clipboard_manager.h"

#include <ctype.h>
#include <stdlib.h>

#include "../data-structure/file_management.h"


bool saveToClipBoard(Cursor begin, Cursor end) {
  if (isCursorDisabled(begin) || isCursorDisabled(end)) {
    return true;
  }

  if (areCursorEqual(begin, end))
    return true;

  if (isCursorPreviousThanOther(end, begin)) {
    Cursor tmp = end;
    end = begin;
    begin = tmp;
  }

  FILE* mktemp_result = popen("mktemp /tmp/al-XXXXXX", "r");
  char tmp_file[100];

  if (mktemp_result == NULL) {
    return false;
  }

  fscanf(mktemp_result, " %s ", tmp_file);
  fclose(mktemp_result);

  FILE* f_out = fopen(tmp_file, "w");
  if (f_out == NULL) {
    return false;
  }

  while (isCursorPreviousThanOther(begin, end) && areCursorEqual(begin, end) == false) {
    // may improve performance.
    begin = moveRight(begin);
    if (begin.line_id.absolute_column == 0) {
      printChar_U8(f_out, readChar_U8FromCharArray("\n"));
    }
    else {
      printChar_U8(f_out, getCharForLineIdentifier(begin.line_id));
    }
  }

  fclose(f_out);

  char x_clip_command[200];
  sprintf(x_clip_command, "xclip -selection clipboard < %s ", tmp_file);
  int result_xlip = system(x_clip_command);

  if (result_xlip != 0) {
    return false;
  }

  char rm_tmp_file_command[200];
  sprintf(rm_tmp_file_command, "rm %s", tmp_file);

  return true;
}

Cursor loadFromClipBoard(Cursor cursor) {
  FILE* f = popen("xclip -selection clipboard -out", "r");
  if (f == NULL) {
    return cursor;
  }

  FileNode* root = cursor.file_id.file;

  char c;
  while (fscanf(f, "%c", &c) != EOF) {
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
      Char_U8 ch = readChar_U8FromFileWithFirst(f, c);
#ifdef LOGS
      printChar_U8(stdout, ch);
      printf("\r\n");
#endif
      cursor = insertCharInLineC(cursor, ch);
    }
  }

  fclose(f);

  return cursor;
}
