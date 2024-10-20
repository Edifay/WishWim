#include "clipboard_manager.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "../data-management/file_management.h"


void createClipBoardTmpDir() {
  char command[20 + strlen(CLIPBOARD_PATH)];
  sprintf(command, "mkdir %s -p",CLIPBOARD_PATH);
  system(command);
}


bool saveToClipBoard(Cursor begin, Cursor end) {
  createClipBoardTmpDir();

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

  system("touch /tmp/al/clipboard/last_clip");

  char tmp_file[100];
  sprintf(tmp_file, "%s", "/tmp/al/clipboard/last_clip");

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


  char* xclip = whereis("xclip");

  // If xclip is not found just using last_clip file.
  if (xclip == NULL) {
    return false;
  }

  char x_clip_command[200];
  sprintf(x_clip_command, "xclip -selection clipboard < %s ", tmp_file);
  int result_xlip = system(x_clip_command);

  if (result_xlip != 0) {
    return false;
  }

  char rm_tmp_file_command[200];
  sprintf(rm_tmp_file_command, "rm %s", tmp_file);

  free(xclip);

  return true;
}

Cursor loadFromClipBoard(Cursor cursor) {
  char* xclip = whereis("xclip");

  FILE* f;
  if (xclip == NULL) {
    // If xclip is not found just using last_clip file.
    f = fopen("/tmp/al/clipboard/last_clip", "r");
  }
  else {
    f = popen("xclip -selection clipboard -out", "r");
  }

  if (f == NULL) {
    return cursor;
  }

  // Duplicated search in project DUP_SCAN.
  char c;
  while (fscanf(f, "%c", &c) != EOF) {
#ifdef LOGS
    // assert(checkFileIntegrity(root) == true);
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
        if (TAB_CHAR_USE) {
          ch.t[0] = '\t';
          cursor = insertCharInLineC(cursor, ch);
        }
        else {
          ch.t[0] = ' ';
          for (int i = 0; i < TAB_SIZE; i++) {
            cursor = insertCharInLineC(cursor, ch);
          }
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

  free(xclip);

  return cursor;
}
