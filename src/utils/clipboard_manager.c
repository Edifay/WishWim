#include "clipboard_manager.h"

#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/prctl.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include "constants.h"
#include "../data-management/file_management.h"

int xclip = -1;
int wl_copy = -1;
int wl_paste = -1;
char* xclip_path;
char* wl_copy_path;
char* wl_paste_path;


void createClipBoardTmpDir() {
  mkdir_p(CLIPBOARD_PATH, 0777);
}

void updateXClipVars() {
  if (xclip == -1) {
    xclip_path = whereis("xclip");
    xclip = xclip_path != NULL;
  }
}

void updateWlCopyVars() {
  if (wl_copy == -1) {
    wl_copy_path = whereis("wl-copy");
    wl_copy = wl_copy_path != NULL;
  }
}

void updateWlPasteVars() {
  if (wl_paste == -1) {
    wl_paste_path = whereis("wl-paste");
    wl_paste = wl_paste_path != NULL;
  }
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
  fprintf(f_out, "%c", EOF);
  fclose(f_out);


  updateWlCopyVars();
  updateXClipVars();


  // If xclip and wl-copy are not found just using last_clip file.
  if (xclip == false && wl_copy == false) {
    return false;
  }

  if (xclip == true) {
    if (fork() == 0) {
      FILE* dev_null = fopen("/dev/null", "w");
      dup2(fileno(dev_null), STDOUT_FILENO);
      fclose(dev_null);

      FILE* f_last_clip = fopen("/tmp/al/clipboard/last_clip", "r");
      dup2(fileno(f_last_clip), STDIN_FILENO);
      fclose(f_last_clip);

      prctl(PR_SET_PDEATHSIG, SIGTERM);

      execl(xclip_path, xclip_path, "-selection", "-clipboard", NULL);
      exit(1);
    }
  }

  if (wl_copy == true) {
    if (fork() == 0) {
      FILE* dev_null = fopen("/dev/null", "w");
      dup2(fileno(dev_null), STDOUT_FILENO);
      fclose(dev_null);

      FILE* f_last_clip = fopen("/tmp/al/clipboard/last_clip", "r");
      dup2(fileno(f_last_clip), STDIN_FILENO);
      fclose(f_last_clip);

      prctl(PR_SET_PDEATHSIG, SIGTERM);

      execl(wl_copy_path, wl_copy_path, NULL);
      exit(1);
    }
  }

  return true;
}

Cursor loadFromClipBoard(Cursor cursor) {
  updateWlPasteVars();
  updateXClipVars();

  int pipe_read[2];
  pipe(pipe_read);

  FILE* f;
  // prefer using wl_paste instead of xclip
  if (wl_paste == true) {
    int child_pid = fork();
    if (child_pid == 0) {
      dup2(pipe_read[1], STDOUT_FILENO);

      prctl(PR_SET_PDEATHSIG, SIGTERM);

      execl(wl_paste_path, wl_paste_path, "-n", NULL);

      close(pipe_read[0]);
      close(pipe_read[1]);
      exit(1);
    }

    f = fdopen(pipe_read[0], "r");
  }
  else {
    if (xclip == true) {
      // redondant if

      int child_pid = fork();
      if (child_pid == 0) {
        dup2(pipe_read[1], STDOUT_FILENO);

        prctl(PR_SET_PDEATHSIG, SIGTERM);

        execl(xclip_path, xclip_path, "-selection", "-out", NULL);

        close(pipe_read[0]);
        close(pipe_read[1]);
        exit(1);
      }
      f = fdopen(pipe_read[0], "r");

    }
    else {
      // If xclip and wl_paste are not found just using last_clip file.
      f = fopen("/tmp/al/clipboard/last_clip", "r");
    }
  }

  if (f == NULL) {
    return cursor;
  }

  // Duplicated search in project DUP_SCAN.
  char c;
  while (fscanf(f, "%c", &c) != EOF) {
    if (c == EOF)
      break;
#ifdef LOGS
    // assert(checkFileIntegrity(root) == true);
#endif
    if (iscntrl(c)) {
      if (c == '\n') {
#ifdef LOGS
        // printf("Enter\r\n");
#endif
        cursor = insertNewLineInLineC(cursor);
      }
      else if (c == 9) {
#ifdef LOGS
        // printf("Tab\r\n");
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
        // printf("Unsupported Char loaded from file : '%d'.\r\n", c);
#endif
        // exit(0);
      }
    }
    else {
      Char_U8 ch = readChar_U8FromFileWithFirst(f, c);
#ifdef LOGS
      printChar_U8(stdout, ch);
      // printf("\r\n");
#endif
      cursor = insertCharInLineC(cursor, ch);
    }
  }

  fclose(f);
  close(pipe_read[0]);
  close(pipe_read[1]);

  return cursor;
}
