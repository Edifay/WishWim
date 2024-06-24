#include "file_structure.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "../utils/tools.h"

/**
 *  Init a new empty head of LineNode.
 */
void initEmptyLineNode(LineNode* line) {
  assert(line != NULL);
  line->fixed = false;
  line->next = NULL;
  line->prev = NULL;
  line->ch = NULL;
  line->current_max_element_number = 0;
  line->element_number = 0;
}


/**
 *  Init a new head of LineNode.
 */
void initLineNode(LineNode* line, Size size);

/**
 *  Return the number of char in the LineNode.
 */
int sizeLineNode(LineNode* line) {
  int count = 0;
  while (line != NULL) {
    count += line->element_number;
    line = line->next;
  }
  return count;
}

/**
 * Insert an EMPTY node after the node given.
 */
LineNode* insertLineNodeBefore(LineNode* node) {
  LineNode* newNode = malloc(sizeof(LineNode));
  initEmptyLineNode(newNode);

  newNode->prev = node->prev;
  node->prev = newNode;
  if (newNode->prev != NULL) {
    newNode->prev->next = newNode;
  }
  newNode->next = node;

  return newNode;
}


/**
 * Insert an EMPTY node after the node given.
 */
LineNode* insertLineNodeAfter(LineNode* node) {
  LineNode* newNode = malloc(sizeof(LineNode));
  initEmptyLineNode(newNode);

  newNode->next = node->next;
  newNode->prev = node;
  if (newNode->next != NULL) {
    newNode->next->prev = newNode;
  }
  node->next = newNode;

  return newNode;
}

/**
 * Free the current Node. And return the prev node if not NULL or the next node if not NULL.
 */
LineNode* destroyCurrentLineNode(LineNode* node) {
  LineNode* newNode = NULL;

  if (node->prev != NULL) {
    newNode = node->prev;
  }
  else if (node->next != NULL) {
    newNode = node->next;
  }


  if (node->next != NULL) {
    node->next->prev = node->prev;
  }

  if (node->prev != NULL) {
    node->prev->next = node->next;
  }

  assert(node->prev == NULL || node->prev->next == node->next);
  assert(node->next == NULL || node->next->prev == node->prev);

  free(node->ch);
  assert(node->fixed == false);
  free(node);
  return newNode;
}


/**
 * -1     => failed.
 * 0      => access to index 0 from the next node
 * other  => Number of Char_U8 moved to previous node.
 */
int slideFromLineNodeToNextLineNodeAfterIndex(LineNode* node, int index) {
  if (node->next == NULL)
    return -1;

  if (node->next->element_number == MAX_ELEMENT_NODE)
    return -1;

  if (node->next->current_max_element_number != MAX_ELEMENT_NODE && node->element_number - index + 1 > node->next->
      current_max_element_number - node->next->element_number) {
    // If previous is not already full allocated && need more space to store
    node->next->current_max_element_number = min(
      node->next->current_max_element_number + node->element_number - index + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
    assert(node->next->current_max_element_number <= MAX_ELEMENT_NODE);
    node->next->ch = realloc(node->next->ch, node->next->current_max_element_number * sizeof(Char_U8));
  }

  if (index == MAX_ELEMENT_NODE) {
    return 0;
  }

  int moved = min(node->element_number - index, node->next->current_max_element_number - node->next->element_number);
#ifdef LOGS
  printf("SLIDING RIGHT NUMBER %d\r\n", moved);
#endif
  assert(node->next->element_number + moved <= MAX_ELEMENT_NODE);

  memmove(node->next->ch + moved, node->next->ch, node->next->element_number * sizeof(Char_U8));
  memcpy(node->next->ch, node->ch + node->element_number - moved, moved * sizeof(Char_U8));

  node->element_number -= moved;
  node->next->element_number += moved;
  return moved;
}


/**
 * -1     => failed.
 * 0      => access to last item from the previous node
 * other  => Number of Char_U8 moved to previous node.
 */
int slideFromLineNodeToPreviousLineNodeBeforeIndex(LineNode* node, int index) {
  if (node->prev == NULL)
    return -1;

  if (node->prev->element_number == MAX_ELEMENT_NODE)
    return -1;


  if (node->prev->current_max_element_number != MAX_ELEMENT_NODE && index + 1 > node->prev->current_max_element_number -
      node->prev->element_number) {
    // If previous is not already full allocated && need more space to store
#ifdef LOGS
    printf("Realloc previous ");
#endif
    node->prev->current_max_element_number = min(node->prev->current_max_element_number + index + 1 + CACHE_SIZE,
                                                 MAX_ELEMENT_NODE);
    assert(node->prev->current_max_element_number <= MAX_ELEMENT_NODE);
#ifdef LOGS
    printf(" new size %d\n\r", node->prev->current_max_element_number);
#endif
    node->prev->ch = realloc(node->prev->ch, node->prev->current_max_element_number * sizeof(Char_U8));
  }
#ifdef LOGS
  printf("Min Of Index : %d and %d - %d  = %d  \r\n", index, node->prev->current_max_element_number,
         node->prev->element_number, node->prev->current_max_element_number - node->prev->element_number);
#endif

  if (index == 0)
    return 0;

  int moved = min(index, node->prev->current_max_element_number - node->prev->element_number);
  // printf("Moved :
  assert(node->prev->element_number + moved <= MAX_ELEMENT_NODE);

  memcpy(node->prev->ch + node->prev->element_number, node->ch, moved * sizeof(Char_U8));
  memmove(node->ch, node->ch + moved, (node->element_number - moved) * sizeof(Char_U8));

  node->element_number -= moved;
  node->prev->element_number += moved;
  return moved;
}


/**
 * Get 1 case open in line
 * Return the new relative index for line cell.
 * Shift cannot be positive.
 *
 * If index == 0 focus on previous.
 *
 * If the current node has place focus on it.
 *
 * Else focus on previous.
 *
 * Else focus on next.
 *
 *
 */
int allocateOneCharAtIndex(LineNode* line, int index) {
  assert(index >= 0);
  assert(index <= MAX_ELEMENT_NODE);

  int available_here = MAX_ELEMENT_NODE - line->element_number;

  if (index != 0 || line->prev == NULL || line->prev->element_number == MAX_ELEMENT_NODE) {
    if (available_here >= 1) {
      // Is size available in current cell
#ifdef LOGS
      printf("Simple case !\n\r");
#endif

      if (line->current_max_element_number - line->element_number < 1) {
#ifdef LOGS
        printf("Realloc mem\n\r");
#endif
        // Need to realloc memory ?
        line->current_max_element_number = min(line->current_max_element_number + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
        assert(line->current_max_element_number <= MAX_ELEMENT_NODE);
        line->ch = realloc(line->ch, line->current_max_element_number * sizeof(Char_U8));
      }

      assert(line->current_max_element_number - line->element_number >= 1);
      return 0;
    }

    assert(available_here == 0);
  }

#ifdef LOGS
  printf("Hard case !\n\r");
#endif

  // Current cell cannot contain the asked size.
  int available_prev = line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line->prev->element_number;
  int available_next = line->next == NULL ? 0 : MAX_ELEMENT_NODE - line->next->element_number;
#ifdef LOGS
  printf("AVAILABLE PREVIOUS %d | NEXT %d | CURRENT %d \r\n", available_prev, available_next, available_here);
#endif
  if (available_prev + available_next + available_here < 1) {
    // Need to insert node
#ifdef LOGS
    printf("INSERTING A NODE\r\n");
#endif

    if (index == 0 && !line->fixed) {
      // Due to the fact that we can just deallocate instant from right of an array only more power full for index == 0.
      insertLineNodeBefore(line);
      available_prev = MAX_ELEMENT_NODE;
    }
    else {
      insertLineNodeAfter(line);
      available_next = MAX_ELEMENT_NODE;
    }
  }

  assert(available_prev + available_next >= 1);

  int shift = 0;
  if (available_prev != 0 && !line->fixed) {
#ifdef LOGS
    printf("SLIDING LEFT\r\n");
#endif
    int moved = slideFromLineNodeToPreviousLineNodeBeforeIndex(line, index);
#ifdef LOGS
    printf("Moved : %d Shift %d\r\n", moved, shift);
#endif
    assert(moved != -1);
    shift -= moved;
  }
  else if (MAX_ELEMENT_NODE - line->element_number < 1) {
    // assert(index != MAX_ELEMENT_NODE);
#ifdef LOGS
    printf("SLIDING RIGHT\r\n");
#endif
    slideFromLineNodeToNextLineNodeAfterIndex(line, index);
  }
#ifdef LOGS
  printf("When return %d\n\r", shift);
#endif
  return shift;
}

/**
 * Return idenfier for the node containing current relative index.
 * Return an index as 0 < index <= line->element_number
 * Index == 0 is possible if index = 0 and previous node doesn't exist.
 */
LineIdentifier moduloLineIdentifierR(LineNode* line, int column) {
  int abs_column = column;
  while (column < 0 || (column == 0 && line->prev != NULL)) {
    assert(line->prev != NULL); // Index out of range
    column += line->prev->element_number;
    line = line->prev;
  }

  while (column > line->element_number) {
    column -= line->element_number;
    assert(line->next != NULL); // Index out of range
    line = line->next;
  }

  LineIdentifier line_id;
  line_id.line = line;
  line_id.relative_column = column;
  line_id.absolute_column = abs_column;

  return line_id;
}

LineIdentifier moduloLineIdentifier(LineIdentifier line_id) {
  LineIdentifier tmp_line_id = moduloLineIdentifierR(line_id.line, line_id.relative_column);
  tmp_line_id.absolute_column = line_id.absolute_column;
  return tmp_line_id;
}

/**
 * Insert a char at index of the line node.
 * Return the new relative index for line cell.
 */
LineIdentifier insertCharInLine(LineIdentifier line_id, Char_U8 ch) {
  line_id = moduloLineIdentifier(line_id);

  LineNode* line = line_id.line;
  int column = line_id.relative_column;

  int relative_shift = allocateOneCharAtIndex(line, column);
#ifdef LOGS
  printf("Shift : %d & Index : %d\n\r", relative_shift, column);
#endif
  column = column + relative_shift;

  assert(relative_shift <= 0);
  assert(column <= MAX_ELEMENT_NODE);
  assert(column >= 0);

  if (column == 0) {
    if (line->prev != NULL && line->prev->element_number != MAX_ELEMENT_NODE) {
      // can add previous.
#ifdef LOGS
      printf("SHIFT PREVIOUS DETECTED\n\r");
#endif
      assert(line->prev != NULL);
      line = line->prev;
      column = line->element_number;
    }
    else {
      // cannot add previous assert place here.
      assert(line->element_number != MAX_ELEMENT_NODE);
    }
  }
  else if (column == MAX_ELEMENT_NODE) {
    assert(line->next != NULL);
#ifdef LOGS
    printf("SHIFT NEXT DETECTED\n\r");
#endif
    line = line->next;
    column = 0;
  }

  if (column == line->element_number) {
    // Simple happend
    line->ch[column] = ch;
    line->element_number++;
#ifdef LOGS
    printf("ADD AT THE END\r\n");
#endif
  }
  else {
    // Insert in array
#ifdef LOGS
    printf("INSERT IN MIDDLE\r\n");
#endif
    assert(line->element_number - column > 0);
    memmove(line->ch + column + 1, line->ch + column, (line->element_number - column) * sizeof(Char_U8));
    line->ch[column] = ch;
    line->element_number++;
  }

  line_id.line = line;
  line_id.relative_column = column + 1;
  line_id.absolute_column++;

  return line_id;
}


/**
 * Insert a char at index of the line node.
 */
LineIdentifier removeCharInLine(LineIdentifier line_id) {
  line_id = moduloLineIdentifier(line_id);
  line_id.relative_column--;
  LineNode* line = line_id.line;
  int cursorPos = line_id.relative_column;

  if (cursorPos == -1) {
    assert(line->prev != NULL);
    line = line->prev;
    cursorPos = line->element_number;
    line_id.line = line;
    line_id.relative_column = cursorPos;
  }

#ifdef LOGS
  printf("REMOVE CHAR RELATIVE %d \r\n", cursorPos);
  printf("CURRENT LINE ELEMENT NUMBER %d\r\n", line->element_number);
#endif
  assert(cursorPos >= 0);
  assert(cursorPos < line->element_number);

  if (cursorPos != line->element_number - 1) {
#ifdef LOGS
    printf("REMOVE IN MIDDLE. Table Size %d. Element Number %d\r\n", line->current_max_element_number,
           line->element_number);
#endif
    // printf("At move : %d, first %d, ")
    memmove(line->ch + cursorPos, line->ch + cursorPos + 1,
            (line->element_number - cursorPos - 1) * sizeof(Char_U8));
  }
  else {
#ifdef LOGS
    printf("REMOVE ATT END\r\n");
#endif
  }

  line->element_number--;

  if (line->element_number == 0) {
    // the current node is empty.
    int available_prev = line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line->prev->element_number;
    int available_next = line->next == NULL ? 0 : MAX_ELEMENT_NODE - line->next->element_number;

    if (available_prev != 0 || available_next != 0) {
      assert(line->prev != NULL || line->next != NULL);
#ifdef LOGS
      printf("FREE NODE\r\n");
#endif
      if (line->next != NULL && line->next->element_number == 0) {
        assert(line->next->fixed == false);
        destroyCurrentLineNode(line->next);
      }
      else {
        if (line->fixed == true) {
          assert(line->next->fixed == false);
          slideFromLineNodeToPreviousLineNodeBeforeIndex(line->next, line->next->element_number);
          assert(line->next->element_number == 0);
          destroyCurrentLineNode(line->next);
          line_id.line = line;
          line_id.relative_column = 0;
        }
        else {
          line = destroyCurrentLineNode(line);
          line_id.line = line;
          line_id.relative_column = line->element_number;
        }
      }
    }
    else {
      if (line->current_max_element_number > CACHE_SIZE + 1) {
#ifdef LOGS
        printf("REALLOC ONLY CACHE\r\n");
#endif
        line->current_max_element_number = min(MAX_ELEMENT_NODE, CACHE_SIZE);
        assert(line->current_max_element_number <= MAX_ELEMENT_NODE);
        line->ch = realloc(line->ch, line->current_max_element_number * sizeof(Char_U8));
      }
    }
  }

#ifdef LOGS
  printf("REMOVE ENDED\r\n");
#endif
  line_id.absolute_column--;
  return line_id;
}


Char_U8 getCharForLineIdentifier(LineIdentifier line_id) {
  line_id = moduloLineIdentifier(line_id);
  if (line_id.relative_column == 0) {
    assert(line_id.line->prev != NULL); // OUT OF RANGE.
    return line_id.line->prev->ch[line_id.line->prev->element_number - 1];
  }
  return line_id.line->ch[line_id.relative_column - 1];
}


LineIdentifier getLastLineNode(LineNode* line) {
  assert(line != NULL);
  int abs_column = 0;
  while (line->next != NULL) {
    abs_column += line->element_number;
    line = line->next;
  }

  LineIdentifier line_id;
  line_id.relative_column = line->element_number;
  line_id.line = line;
  line_id.absolute_column = abs_column + line->element_number;

  return line_id;
}

int getAbsoluteLineIndex(LineIdentifier id) {
  assert(id.line != NULL);

  int abs = id.relative_column;
  id.line = id.line->prev;

  while (id.line != NULL) {
    abs += id.line->element_number;
    id.line = id.line->prev;
  }

  return abs;
}


bool isEmptyLine(LineNode* line) {
  while (line != NULL) {
    if (line->element_number != 0)
      return false;
    line = line->next;
  }
  return true;
}

bool hasElementAfterLine(LineIdentifier line_id) {
  return line_id.relative_column != line_id.line->element_number || isEmptyLine(line_id.line->next) == false;
}

bool hasElementBeforeLine(LineIdentifier line_id) {
  return line_id.absolute_column != 0;
}


void printLineNode(LineNode* line) {
  for (int i = 0; i < line->element_number; i++) {
    printChar_U8(stdout, line->ch[i]);
  }
}

LineIdentifier tryToReachAbsColumn(LineIdentifier line_id, int abs_column) {
  if (line_id.absolute_column == abs_column)
    return line_id;

  line_id = moduloLineIdentifier(line_id);

  line_id.absolute_column -= line_id.relative_column;
  line_id.relative_column = 0;

  bool first = false;

  while (line_id.absolute_column + line_id.line->element_number < abs_column) {
    first = true;
    if (line_id.line->next == NULL)
      break;
    line_id.absolute_column += line_id.line->element_number - line_id.relative_column;
    line_id.line = line_id.line->next;
    line_id.relative_column = 0;
  }

  while (abs_column < line_id.absolute_column) {
    assert(first == false);
    assert(line_id.relative_column == 0);
    if (line_id.line->prev == NULL)
      break;
    line_id.absolute_column -= line_id.line->prev->element_number;
    line_id.line = line_id.line->prev;
    line_id.relative_column = 0; // no needed.
  }

  if (line_id.absolute_column < abs_column) {
    int toAdd = min(line_id.line->element_number - line_id.relative_column, abs_column - line_id.absolute_column);
    line_id.absolute_column += toAdd;
    line_id.relative_column += toAdd;
    assert(line_id.relative_column == line_id.line->element_number || abs_column == line_id.absolute_column);
  }

  return moduloLineIdentifier(line_id);
}


void deleteLinePart(LineIdentifier line_id, int length) {
  line_id = moduloLineIdentifier(line_id);

  int current_removed = 0;
  while (current_removed < length) {
    // If it's the line end go to next.
    if (line_id.relative_column == line_id.line->element_number) {
      line_id.line = line_id.line->next;
      assert(line_id.line != NULL);
      line_id.relative_column = 0;
    }

    // If the id is the begin of a node and If the node can be completely removed. We can improve perf.
    if (line_id.relative_column == 0 && line_id.line->element_number <= length - current_removed) {
      current_removed += line_id.line->element_number;
      if (line_id.line->fixed == false) {
        // If the node can be cleared.
        line_id.line = destroyCurrentLineNode(line_id.line);
        line_id.relative_column = line_id.line->element_number;
      }
      else {
        // If the node is fixed.
        // assert(false);
        line_id.line->element_number = 0;
        line_id.line->current_max_element_number = 0;
        free(line_id.line->ch);
        line_id.line->ch = NULL;
      }
    }
    else {
      // Need to delete a part of the node.
      int atDelete = min(length - current_removed, line_id.line->element_number - line_id.relative_column);
      memmove(line_id.line->ch + line_id.relative_column, line_id.line->ch + line_id.relative_column + atDelete,
              (line_id.line->element_number - line_id.relative_column - atDelete) * sizeof(Char_U8));
      line_id.line->element_number -= atDelete;
      current_removed += atDelete;

      // Realloc line.ch if too much unused mem.
      if (line_id.line->current_max_element_number > line_id.line->element_number + CACHE_SIZE) {
        line_id.line->ch = realloc(line_id.line->ch, (line_id.line->element_number + CACHE_SIZE) * sizeof(Char_U8));
        line_id.line->current_max_element_number = line_id.line->element_number + CACHE_SIZE;
        assert(line_id.line->current_max_element_number <= MAX_ELEMENT_NODE);
      }
    }
  }
}


/**
 * Destroy line free all memory.
 * Will check Node before.
 */
void destroyFullLine(LineNode* node) {
  while (node != NULL && node->prev != NULL) {
    node = node->prev;
  }

  while (node != NULL) {
    LineNode* tmp = node;
    node = node->next;

    free(tmp->ch);
    if (tmp->fixed == false)
      free(tmp);
    else {
      initEmptyLineNode(tmp);
      tmp->fixed = true;
    }
  }
}

/**
 *
 * Destroy line letting this current node.
 */
void destroyChildLine(LineNode* node) {
  assert(node != NULL);
  free(node->ch);
  node->ch = NULL;
  node->current_max_element_number = 0;
  node->element_number = 0;
  node = node->next;

  while (node != NULL) {
    LineNode* tmp = node;
    node = node->next;

    free(tmp->ch);
    if (tmp->fixed == false)
      free(tmp);
    else {
      initEmptyLineNode(tmp);
      tmp->fixed = true;
    }
  }
}


///// ----------------------- FILE ---------------------------------

/**
 *  Init a new empty head of FileNode.
 */
void initEmptyFileNode(FileNode* file) {
  assert(file != NULL);
  file->next = NULL;
  file->prev = NULL;
  file->lines = NULL;
  file->current_max_element_number = 0;
  file->element_number = 0;
}

Cursor initNewWrittableFile() {
  FileNode* file = malloc(sizeof(FileNode));
  initEmptyFileNode(file);

  FileIdentifier file_id = insertEmptyLineInFile(moduloFileIdentifierR(file, 0));
  assert(file_id.file == file);
  assert(file_id.relative_row == 1);

  return moduloCursorR(file, 1, 0);
}

/**
 * length == -1 => mean that length is
 */
void rebindFileNode(FileNode* file, int start, int length) {
  if (length == -1)
    length = file->element_number - start;
  assert(start + length <= file->element_number);
  assert(start >= 0);
  for (int i = start; i < start + length; i++) {
    if (file->lines[i].next != NULL) {
      file->lines[i].next->prev = file->lines + i;
    }
  }
}

void rebindFullFileNode(FileNode* file) {
  rebindFileNode(file, 0, -1);
}


/**
 * Insert an EMPTY node after the node given.
 */
FileNode* insertFileNodeBefore(FileNode* file) {
  FileNode* newNode = malloc(sizeof(FileNode));
  initEmptyFileNode(newNode);

  newNode->prev = file->prev;
  file->prev = newNode;
  if (newNode->prev != NULL) {
    newNode->prev->next = newNode;
  }
  newNode->next = file;

  return newNode;
}


/**
 * Insert an EMPTY node after the node given.
 */
FileNode* insertFileNodeAfter(FileNode* file) {
  FileNode* newNode = malloc(sizeof(FileNode));
  initEmptyFileNode(newNode);

  newNode->next = file->next;
  newNode->prev = file;
  if (newNode->next != NULL) {
    newNode->next->prev = newNode;
  }
  file->next = newNode;

  return newNode;
}

/**
 * Free the current Node. And return the prev node if not NULL or the next node if not NULL.
 */
FileNode* destroyCurrentFileNode(FileNode* file) {
  FileNode* newNode = NULL;

  if (file->prev != NULL) {
    newNode = file->prev;
  }
  else if (file->next != NULL) {
    newNode = file->next;
  }


  if (file->next != NULL) {
    file->next->prev = file->prev;
  }

  if (file->prev != NULL) {
    file->prev->next = file->next;
  }

  assert(file->prev == NULL || file->prev->next == file->next);
  assert(file->next == NULL || file->next->prev == file->prev);

  for (int i = 0; i < file->element_number; i++) {
    destroyFullLine(file->lines + i);
  }
  free(file->lines);
  free(file);
  return newNode;
}


/**
 * -1     => failed.
 * 0      => access to index 0 from the next node
 * other  => Number of Char_U8 moved to previous node.
 */
int slideFromFileNodeToNextFileNodeAfterIndex(FileNode* file, int row) {
  if (file->next == NULL)
    return -1;

  if (file->next->element_number == MAX_ELEMENT_NODE)
    return -1;

  if (file->next->current_max_element_number != MAX_ELEMENT_NODE && file->element_number - row + 1 > file->next->
      current_max_element_number - file->next->element_number) {
    // If previous is not already full allocated && need more space to store
    file->next->current_max_element_number = min(
      file->next->current_max_element_number + file->element_number - row + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
    assert(file->next->current_max_element_number <= MAX_ELEMENT_NODE);
    LineNode* old_tab = file->next->lines;
    file->next->lines = realloc(file->next->lines, file->next->current_max_element_number * sizeof(LineNode));
    if (file->next->lines != old_tab) {
      rebindFullFileNode(file->next);
    }
  }

  if (row == MAX_ELEMENT_NODE) {
    return 0;
  }

  int moved = min(file->element_number - row, file->next->current_max_element_number - file->next->element_number);
#ifdef LOGS
  printf("SLIDING RIGHT NUMBER %d\r\n", moved);
#endif
  assert(file->next->element_number + moved <= MAX_ELEMENT_NODE);

  memmove(file->next->lines + moved, file->next->lines, file->next->element_number * sizeof(LineNode));
  memcpy(file->next->lines, file->lines + file->element_number - moved, moved * sizeof(LineNode));

  file->element_number -= moved;
  file->next->element_number += moved;

  rebindFullFileNode(file->next);
  return moved;
}


/**
 * -1     => failed.
 * 0      => access to last item from the previous node
 * other  => Number of Char_U8 moved to previous node.
 */
int slideFromFileNodeToPreviousFileNodeBeforeIndex(FileNode* file, int row) {
  if (file->prev == NULL)
    return -1;

  if (file->prev->element_number == MAX_ELEMENT_NODE)
    return -1;


  if (file->prev->current_max_element_number != MAX_ELEMENT_NODE && row + 1 > file->prev->current_max_element_number -
      file->prev->element_number) {
    // If previous is not already full allocated && need more space to store
#ifdef LOGS
    printf("Realloc previous ");
#endif
    file->prev->current_max_element_number = min(file->prev->current_max_element_number + row + 1 + CACHE_SIZE,
                                                 MAX_ELEMENT_NODE);
    assert(file->prev->current_max_element_number <= MAX_ELEMENT_NODE);
#ifdef LOGS
    printf(" new size %d\n\r", file->prev->current_max_element_number);
#endif
    LineNode* old_tab = file->prev->lines;
    file->prev->lines = realloc(file->prev->lines, file->prev->current_max_element_number * sizeof(LineNode));
    if (file->prev->lines != old_tab) {
      rebindFullFileNode(file->prev);
    }
  }
#ifdef LOGS
  printf("Min Of Index : %d and %d - %d  = %d  \r\n", row, file->prev->current_max_element_number,
         file->prev->element_number, file->prev->current_max_element_number - file->prev->element_number);
#endif

  if (row == 0)
    return 0;

  int moved = min(row, file->prev->current_max_element_number - file->prev->element_number);
  assert(file->prev->element_number + moved <= MAX_ELEMENT_NODE);

  memcpy(file->prev->lines + file->prev->element_number, file->lines, moved * sizeof(LineNode));
  memmove(file->lines, file->lines + moved, (file->element_number - moved) * sizeof(LineNode));


  file->element_number -= moved;
  file->prev->element_number += moved;
  rebindFullFileNode(file);
  rebindFileNode(file->prev, file->prev->element_number - moved, moved);

  return moved;
}


/**
 * Get 1 case open in line
 * Return the new relative index for line cell.
 * Shift cannot be positive.
 *
 * If the current node has place focus on it.
 *
 * Else focus on previous.
 *
 * Else focus on next.
 *
 *
 */
int allocateOneRowInFile(FileNode* file, int row) {
  assert(row >= 0);
  assert(row <= MAX_ELEMENT_NODE);

  int available_here = MAX_ELEMENT_NODE - file->element_number;

  if (available_here >= 1) {
    // Is size available in current cell
#ifdef LOGS
    printf("Simple case !\n\r");
#endif

    if (file->current_max_element_number - file->element_number < 1) {
#ifdef LOGS
      printf("Realloc mem\n\r");
#endif
      // Need to realloc memory ?
      file->current_max_element_number = min(file->current_max_element_number + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
      assert(file->current_max_element_number <= MAX_ELEMENT_NODE);
      LineNode* old_tab = file->lines;
      file->lines = realloc(file->lines, file->current_max_element_number * sizeof(LineNode));
      if (file->lines != old_tab) {
        rebindFullFileNode(file);
      }
    }

    assert(file->current_max_element_number - file->element_number >= 1);
    return 0;
  }

  assert(available_here == 0);
#ifdef LOGS
  printf("Hard case !\n\r");
#endif

  // Current cell cannot contain the asked size.
  int available_prev = file->prev == NULL ? 0 : MAX_ELEMENT_NODE - file->prev->element_number;
  int available_next = file->next == NULL ? 0 : MAX_ELEMENT_NODE - file->next->element_number;
#ifdef LOGS
  printf("AVAILABLE PREVIOUS %d | NEXT %d | CURRENT %d \r\n", available_prev, available_next, available_here);
#endif

  if (available_prev + available_next + available_here < 1) {
    // Need to insert node
#ifdef LOGS
    printf("INSERTING A NODE\r\n");
#endif

    if (row == 0 && file->prev != NULL /*AVOID move previous for root FileNode*/) {
      // Due to the fact that we can just deallocate instant from right of an array only more power full for index == 0.
      insertFileNodeBefore(file);
      available_prev = MAX_ELEMENT_NODE;
    }
    else {
      insertFileNodeAfter(file);
      available_next = MAX_ELEMENT_NODE;
    }
  }

  assert(available_prev + available_next >= 1);

  int shift = 0;
  if (available_prev != 0) {
#ifdef LOGS
    printf("SLIDING LEFT\r\n");
#endif
    int moved = slideFromFileNodeToPreviousFileNodeBeforeIndex(file, row);
#ifdef LOGS
    printf("Moved : %d Shift %d\r\n", moved, shift);
#endif
    assert(moved != -1);
    shift -= moved;
  }
  else if (MAX_ELEMENT_NODE - file->element_number < 1) {
    // assert(index != MAX_ELEMENT_NODE);
#ifdef LOGS
    printf("SLIDING RIGHT\r\n");
#endif
    slideFromFileNodeToNextFileNodeAfterIndex(file, row);
  }
#ifdef LOGS
  printf("When return %d\n\r", shift);
#endif
  return shift;
}

/**
 * Return idenfier for the node containing current relative index.
 * Return an index as 0 <= index <= line->element_number
 */
FileIdentifier moduloFileIdentifierR(FileNode* file, int row) {
  int abs_row = row;
  while (row < 0 || (row == 0 && file->prev != NULL)) {
    assert(file->prev != NULL); // Index out of range
    row += file->prev->element_number;
    file = file->prev;
  }

  while (row > file->element_number) {
    row -= file->element_number;
    // assert(file->next != NULL); // Index out of range
    file = file->next;
  }

  FileIdentifier line_id;
  line_id.file = file;
  line_id.relative_row = row;
  line_id.absolute_row = abs_row;

  return line_id;
}

FileIdentifier moduloFileIdentifier(FileIdentifier file_id) {
  FileIdentifier tmp_file_id = moduloFileIdentifierR(file_id.file, file_id.relative_row);
  tmp_file_id.absolute_row = file_id.absolute_row;
  return tmp_file_id;
}


/**
 * Insert a char at index of the line node.
 * Return the new relative index for line cell.
 */
FileIdentifier insertEmptyLineInFile(FileIdentifier file_id) {
  FileIdentifier line_id = moduloFileIdentifier(file_id);

  FileNode* file = line_id.file;
  int row = line_id.relative_row;

  int relative_shift = allocateOneRowInFile(file, row);

#ifdef LOGS
  printf("Shift : %d & Index : %d\n\r", relative_shift, row);
#endif
  row = row + relative_shift;

  assert(relative_shift <= 0);
  assert(row <= MAX_ELEMENT_NODE);
  assert(row >= 0);

  if (row == 0) {
    if (file->prev != NULL && file->prev->element_number != MAX_ELEMENT_NODE) {
      // can add previous.
#ifdef LOGS
      printf("SHIFT PREVIOUS DETECTED\n\r");
#endif
      assert(file->prev != NULL);
      file = file->prev;
      row = file->element_number;
    }
    else {
      // cannot add previous assert place here.
      assert(file->element_number != MAX_ELEMENT_NODE);
    }
  }
  else if (row == MAX_ELEMENT_NODE) {
    assert(file->next != NULL);
#ifdef LOGS
    printf("SHIFT NEXT DETECTED\n\r");
#endif
    file = file->next;
    row = 0;
  }

  if (row == file->element_number) {
    // Simple happend
    initEmptyLineNode(file->lines + row);
    file->lines[row].fixed = true;
    file->element_number++;

#ifdef LOGS
    printf("ADD AT THE END\r\n");
#endif
  }
  else {
    // Insert in array
#ifdef LOGS
    printf("INSERT IN MIDDLE\r\n");
#endif
    assert(file->element_number - row > 0);
    memmove(file->lines + row + 1, file->lines + row, (file->element_number - row) * sizeof(LineNode));
    initEmptyLineNode(file->lines + row);
    file->lines[row].fixed = true;
    file->element_number++;
    rebindFileNode(file, row + 1, -1);
  }

  line_id.file = file;
  line_id.relative_row = row + 1;
  line_id.absolute_row++;

  return line_id;
}


/**
 * Insert a line at index of the line node.
 */
FileIdentifier removeLineInFile(FileIdentifier file_id) {
  file_id = moduloFileIdentifier(file_id);
  file_id.relative_row--;
  file_id.absolute_row--;
  FileNode* file = file_id.file;
  int row = file_id.relative_row;

  if (row == -1) {
    assert(file->prev != NULL);
    file = file->prev;
    row = file->element_number;
    file_id.file = file;
    file_id.relative_row = row;
  }

#ifdef LOGS
  printf("REMOVE LINE RELATIVE %d \r\n", row);
  printf("CURRENT LINE ELEMENT NUMBER %d\r\n", file->element_number);
#endif
  assert(row >= 0);
  assert(row < file->element_number);

  if (row != file->element_number - 1) {
#ifdef LOGS
    printf("REMOVE IN MIDDLE. Table Size %d. Element Number %d\r\n", file->current_max_element_number,
           file->element_number);
#endif
    // printf("At move : %d, first %d, ")
    memmove(file->lines + row, file->lines + row + 1, (file->element_number - row - 1) * sizeof(LineNode));
    rebindFileNode(file, row, file->element_number - 1 - row);
  }
  else {
#ifdef LOGS
    printf("REMOVE ATT END\r\n");
#endif
  }

  file->element_number--;

  if (file->element_number == 0) {
    // the current node is empty.
    int available_prev = file->prev == NULL ? 0 : MAX_ELEMENT_NODE - file->prev->element_number;
    int available_next = file->next == NULL ? 0 : MAX_ELEMENT_NODE - file->next->element_number;

    if (available_prev != 0 || available_next != 0) {
      assert(file->prev != NULL || file->next != NULL);
#ifdef LOGS
      printf("FREE NODE\r\n");
#endif
      file = destroyCurrentFileNode(file);
      file_id.file = file;
      file_id.relative_row = file_id.file->element_number;
    }
    else {
      if (file->current_max_element_number > CACHE_SIZE + 1) {
#ifdef LOGS
        printf("REALLOC ONLY CACHE\r\n");
#endif
        file->current_max_element_number = min(MAX_ELEMENT_NODE, CACHE_SIZE);
        assert(file->current_max_element_number <= MAX_ELEMENT_NODE);
        LineNode* old_tab = file->lines;
        file->lines = realloc(file->lines, file->current_max_element_number * sizeof(LineNode));
        if (file->lines != old_tab) {
          rebindFullFileNode(file);
        }
      }
    }
  }
#ifdef LOGS
  printf("REMOVE ENDED\r\n");
#endif
  return file_id;
}

int getAbsoluteFileIndex(FileIdentifier id) {
  int abs = id.relative_row;
  id.file = id.file->prev;

  while (id.file != NULL) {
    abs += id.file->element_number;
    id.file = id.file->prev;
  }

  return abs;
}

LineNode* getLineForFileIdentifier(FileIdentifier file_id) {
  file_id = moduloFileIdentifier(file_id);
  if (file_id.relative_row == 0) {
    assert(file_id.file->prev != NULL); // OUT OF RANGE.
    return file_id.file->prev->lines + file_id.file->prev->element_number - 1;
  }
  return file_id.file->lines + file_id.relative_row - 1;
}


bool checkFileIntegrity(FileNode* file) {
  while (file != NULL) {
    for (int i = 0; i < file->element_number; i++) {
      LineNode* line = file->lines + i;
      if (line != NULL) {
        // assert(line->prev == NULL);
        if (line->prev != NULL)
          return false;
      }
      while (line != NULL) {
        if (line->next != NULL) {
          // assert(line->next->prev == line);
          if (line->next->prev != line) {
            printf("INTEGRITY WRONG \r\n");
            if (i != 0) {
              printLineNode(file->lines + i - 1);
              printf("\r\n");
            }
            printf("=> %d : ", line->element_number);
            printLineNode(line);
            printf(" -> ");
            if (line->next != NULL)
              printLineNode(line->next);
            else
              printf("None");
            printf("\r\n");
            // assert(line->next->prev == line);
            return false;
          }
        }
        line = line->next;
      }
    }
    if (file->next != NULL) {
      // assert(file->next->prev == file);
      if (file->next->prev != file)
        return false;
    }
    file = file->next;
  }
  return true;
}

bool isEmptyFile(FileNode* file) {
  while (file != NULL) {
    if (file->element_number != 0)
      return false;
    file = file->next;
  }
  return true;
}

bool hasElementAfterFile(FileIdentifier file_id) {
  return file_id.relative_row != file_id.file->element_number || isEmptyFile(file_id.file->next) == false;
}

bool hasElementBeforeFile(FileIdentifier file_id) {
  return file_id.absolute_row != 1;
}

FileIdentifier tryToReachAbsRow(FileIdentifier file_id, int abs_row) {
  if (file_id.absolute_row == abs_row) {
    return file_id;
  }

  file_id = moduloFileIdentifier(file_id);

  file_id.absolute_row -= file_id.relative_row;
  file_id.relative_row = 0;
  assert(file_id.relative_row == 0);


  bool first = false;

  while (file_id.absolute_row + file_id.file->element_number < abs_row) {
    first = true;
    if (file_id.file->next == NULL)
      break;
    file_id.absolute_row += file_id.file->element_number;
    file_id.file = file_id.file->next;
  }

  while (abs_row < file_id.absolute_row) {
    assert(first == false);
    if (file_id.file->prev == NULL)
      break;
    file_id.absolute_row -= file_id.file->prev->element_number;
    file_id.file = file_id.file->prev;
  }

  if (file_id.absolute_row < abs_row) {
    int toAdd = min(file_id.file->element_number - file_id.relative_row, abs_row - file_id.absolute_row);
    file_id.absolute_row += toAdd;
    file_id.relative_row += toAdd;
    assert(file_id.relative_row == file_id.file->element_number || abs_row == file_id.absolute_row);
  }

  return moduloFileIdentifier(file_id);
}


void deleteFilePart(FileIdentifier file_id, int length) {
  file_id = moduloFileIdentifier(file_id);

  int current_removed = 0;
  while (current_removed < length) {
    // If it's the line end go to next.
    if (file_id.relative_row == file_id.file->element_number) {
      file_id.file = file_id.file->next;
      file_id.relative_row = 0;
      assert(file_id.file != NULL);
    }

    // If the id is the begin of a node and If the node can be completely removed. We can improve perf.
    if (file_id.relative_row == 0 && file_id.file->element_number <= length - current_removed) {
      current_removed += file_id.file->element_number;
      if (file_id.file->prev != NULL) {
        // If the node can be cleared.
        file_id.file = destroyCurrentFileNode(file_id.file);
        file_id.relative_row = file_id.file->element_number;
      }
      else {
        // If the node is fixed.
        // assert(false);
        file_id.file->element_number = 0;
        file_id.file->current_max_element_number = 0;
        for (int i = 0; i < file_id.file->element_number; i++) {
          destroyFullLine(file_id.file->lines + i);
        }
        free(file_id.file->lines);
        file_id.file->lines = NULL;
      }
    }
    else {
      // Need to delete a part of the node.
      int atDelete = min(length - current_removed, file_id.file->element_number - file_id.relative_row);
      for (int i = file_id.relative_row; i < file_id.relative_row + atDelete; i++) {
        destroyFullLine(file_id.file->lines + i);
      }
      memmove(file_id.file->lines + file_id.relative_row, file_id.file->lines + file_id.relative_row + atDelete,
              (file_id.file->element_number - file_id.relative_row - atDelete) * sizeof(LineNode));
      file_id.file->element_number -= atDelete;
      current_removed += atDelete;


      // Realloc file.lines if too much unused mem.
      if (file_id.file->current_max_element_number > file_id.file->element_number + CACHE_SIZE) {
        file_id.file->lines = realloc(file_id.file->lines, (file_id.file->element_number + CACHE_SIZE) * sizeof(LineNode));
        file_id.file->current_max_element_number = file_id.file->element_number + CACHE_SIZE;
        assert(file_id.file->current_max_element_number <= MAX_ELEMENT_NODE);
        assert(file_id.file->element_number <= file_id.file->current_max_element_number);
      }

      rebindFileNode(file_id.file, 0, file_id.file->element_number);
    }
  }
}

/**
 * Destroy line free all memory.
 * Will check Node before.
 */
void destroyFullFile(FileNode* node) {
  while (node->prev != NULL) {
    node = node->prev;
  }

  while (node != NULL) {
    FileNode* tmp = node;
    node = node->next;

    for (int i = 0; i < tmp->element_number; i++) {
      destroyFullLine(tmp->lines + i);
    }

    free(tmp->lines);
    free(tmp);
  }
}


////// -------------- COMBO LINE & FILE --------------


Cursor moduloCursorR(FileNode* file, int row, int column) {
  FileIdentifier file_id = moduloFileIdentifierR(file, row);
  LineIdentifier line_id = moduloLineIdentifierR(getLineForFileIdentifier(file_id), column);

  return cursorOf(file_id, line_id);
}


/**
 *  Init a new head of FileNode.
 */
void initFileNode(FileNode* file, Size size);


/**
 *  Return the number of line in the FileNode.
 */
int sizeFileNode(FileNode* file) {
  int count = 0;
  while (file != NULL) {
    count += file->element_number;
    file = file->next;
  }
  return count;
}


/**
 * Return the char from the FileNode at index line, column. If file[line][column] don't exist return null.
 */
Char_U8 getCharAt(FileNode* file, int row, int column) {
  return getCharForLineIdentifier(moduloCursorR(file, row, column).line_id);
}

Cursor cursorOf(FileIdentifier file_id, LineIdentifier line_id) {
  Cursor cursor = {file_id, line_id};
  return moduloCursor(cursor);
}

Cursor moduloCursor(Cursor cursor) {
  cursor.file_id = moduloFileIdentifier(cursor.file_id);
  if (cursor.line_id.absolute_column <= MAX_ELEMENT_NODE) {
    // The node have a big chance to be fixed
    // So the risk that the line_id could have been reallocated is high.
    cursor.line_id = moduloLineIdentifierR(getLineForFileIdentifier(cursor.file_id), cursor.line_id.absolute_column);
  }
  else {
    // If the node is not fixed use classic modulo.
    // Less risk better performance.
    cursor.line_id = moduloLineIdentifier(cursor.line_id);
  }

  return cursor;
}

Cursor insertCharInLineC(Cursor cursor, Char_U8 ch) {
  cursor.line_id = insertCharInLine(cursor.line_id, ch);
  return cursor;
}

Cursor removeCharInLineC(Cursor cursor) {
  cursor.line_id = removeCharInLine(cursor.line_id);
  return cursor;
}

Cursor removeLineInFileC(Cursor cursor) {
  destroyFullLine(getLineForFileIdentifier(cursor.file_id));
  FileIdentifier file_id_after_del = removeLineInFile(cursor.file_id);
  cursor.file_id = tryToReachAbsRow(file_id_after_del, file_id_after_del.absolute_row + 1);
  cursor.line_id = getLastLineNode(getLineForFileIdentifier(cursor.file_id));
  return cursor;
}


Cursor insertNewLineInLineC(Cursor cursor) {
  cursor = moduloCursor(cursor);

  FileIdentifier file_id = cursor.file_id;
  LineIdentifier line_id = cursor.line_id;
  const bool line_was_fixed = line_id.line->fixed;

  FileIdentifier newFileIdForNewLine = insertEmptyLineInFile(file_id);
  LineNode* newLine = getLineForFileIdentifier(newFileIdForNewLine);
  assert(newLine != NULL);
  assert(newLine->prev == NULL);
  assert(newLine->next == NULL);
  assert(newLine->fixed == true);
  assert(newLine->element_number == 0);
  assert(newLine->current_max_element_number == 0);
  assert(newLine->ch == NULL);

  // file_id may have been reallocated.
  file_id = tryToReachAbsRow(newFileIdForNewLine, file_id.absolute_row);
  if (line_was_fixed) {
    // If the line was fixed the line may have been reallocated. So we need to re-use modulo. We admit that fixed are the src of the line.
    line_id = moduloCursorR(file_id.file, file_id.relative_row, line_id.relative_column).line_id;
  }

  if (line_id.relative_column == 0) {
#ifdef LOGS
    printf("Current cursor pos is 0 check to move current node.\r\n");
#endif

    // We are not moving the node for now but juste moving the Char_U8 array.
    newLine->ch = line_id.line->ch;
    newLine->next = line_id.line->next;
    newLine->current_max_element_number = line_id.line->current_max_element_number;
    newLine->element_number = line_id.line->element_number;

    if (line_id.line->fixed == false) {
      assert(false);
      // This situation is not supposed to happend, by the definition of modulo ! But it's implemented if needed
      if (line_id.line->prev != NULL) {
        line_id.line->prev->next = NULL;
        free(line_id.line);
      }
    }
    else {
      initEmptyLineNode(line_id.line);
      line_id.line->fixed = true;
    }
  }
  else if (line_id.relative_column == line_id.line->element_number) {
#ifdef LOGS
    printf("Current cursor pos is max_element check to move next node.\r\n");
#endif
    // We are not moving the node for now but juste moving the Char_U8 array.
    if (line_id.line->next != NULL) {
      newLine->ch = line_id.line->next->ch;
      newLine->next = line_id.line->next->next;
      newLine->current_max_element_number = line_id.line->next->current_max_element_number;
      newLine->element_number = line_id.line->next->element_number;

      if (line_id.line->next->fixed == false) {
        free(line_id.line->next);
        line_id.line->next = NULL;
      }
      else {
        initEmptyLineNode(line_id.line);
        line_id.line->fixed = true;
      }
    }
    else {
      // Currently at the end of the line.
      // DO NOTHING !
#ifdef LOGS
      printf("Nothing to do ?\r\n");
#endif
    }
  }
  else {
    // In the middle of a line node. Need to copy data to a new linenode.
#ifdef LOGS
    printf("Current cursor pos is in mid line dissociating line.\r\n");
#endif

    newLine->next = line_id.line->next;
    newLine->element_number = line_id.line->element_number - line_id.relative_column;

    newLine->current_max_element_number = min(newLine->element_number + CACHE_SIZE, MAX_ELEMENT_NODE);
    assert(newLine->current_max_element_number <= MAX_ELEMENT_NODE);

    assert(newLine->ch == NULL);
    newLine->ch = malloc(newLine->current_max_element_number * sizeof(Char_U8));
    memcpy(newLine->ch, line_id.line->ch + line_id.relative_column, newLine->element_number * sizeof(Char_U8));

    line_id.line->next = NULL;
    line_id.line->element_number = line_id.relative_column;

    const int old_max_element_number = line_id.line->current_max_element_number;
    line_id.line->current_max_element_number = min(min(MAX_ELEMENT_NODE, old_max_element_number),
                                                   line_id.line->element_number + CACHE_SIZE);
    if (old_max_element_number != line_id.line->current_max_element_number) {
      line_id.line->ch = realloc(line_id.line->ch, line_id.line->current_max_element_number * sizeof(Char_U8));
    }
  }

  if (newLine->next != NULL) // Really important because newLine may have changed.
    newLine->next->prev = newLine;

  LineIdentifier newLineId = moduloLineIdentifierR(newLine, 0);
  return cursorOf(newFileIdForNewLine, newLineId);
}


/**
 *
 * Dumb implementation of concat.
 * */
Cursor concatNeighbordsLinesC(Cursor cursor) {
  cursor = moduloCursor(cursor);

  FileIdentifier file_id = cursor.file_id;
  LineIdentifier line_id = cursor.line_id;

  assert(line_id.line->fixed == true);
  // We assume that in the file we remove a line when we are at the column 0 so the lineNode is fixed.

  if (isEmptyLine(line_id.line)) {
#ifdef LOGS
    printf("Deleting without moving empty line\r\n");
#endif
    destroyFullLine(getLineForFileIdentifier(file_id));

    FileIdentifier newLineId = removeLineInFile(file_id);
    LineIdentifier lastNode = getLastLineNode(getLineForFileIdentifier(newLineId));

    if (getLineForFileIdentifier(newLineId)->next != NULL)
      assert(lastNode.line != NULL);

    return cursorOf(newLineId, lastNode);
  }
#ifdef LOGS
  printf("Deleting with moving line\r\n");
#endif

  LineNode* newNode = malloc(sizeof(LineNode));
  initEmptyLineNode(newNode);
  newNode->ch = line_id.line->ch;
  newNode->next = line_id.line->next;
  if (newNode->next != NULL)
    newNode->next->prev = newNode;
  newNode->fixed = false;
  newNode->element_number = line_id.line->element_number;
  newNode->current_max_element_number = line_id.line->current_max_element_number;

  FileIdentifier newLineId = removeLineInFile(file_id);
  LineIdentifier lastNode = getLastLineNode(getLineForFileIdentifier(newLineId));

  if (lastNode.line->element_number == 0) {
    // Just copy the data of newNode to this node.
    // printf("First case\r\n");
    assert(lastNode.line->element_number == 0);
    free(lastNode.line->ch);
    lastNode.line->ch = newNode->ch;
    lastNode.line->next = newNode->next;
    lastNode.line->element_number = newNode->element_number;
    lastNode.line->current_max_element_number = newNode->current_max_element_number;
    if (lastNode.line->next != NULL)
      lastNode.line->next->prev = lastNode.line;
    free(newNode);
  }
  else {
    lastNode.line->next = newNode;
    newNode->prev = lastNode.line;
  }


  return cursorOf(newLineId, lastNode);
}


Cursor tryToReachAbsPosition(Cursor cursor, int row, int column) {
  FileIdentifier new_file_id = tryToReachAbsRow(cursor.file_id, row);
  LineIdentifier new_line_id = tryToReachAbsColumn(moduloLineIdentifierR(getLineForFileIdentifier(new_file_id), 0), column);
  return cursorOf(new_file_id, new_line_id);
}

Char_U8 getCharAtCursor(Cursor cursor) {
  return getCharForLineIdentifier(cursor.line_id);
}
