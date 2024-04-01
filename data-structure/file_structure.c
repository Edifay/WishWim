#include "file_structure.h"
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


  if (node->prev->current_max_element_number != MAX_ELEMENT_NODE && index + 1 > node->prev->current_max_element_number
      -
      node->prev->element_number) {
    // If previous is not already full allocated && need more space to store
#ifdef LOGS
    printf("Realloc previous ");
#endif
    node->prev->current_max_element_number = min(node->prev->current_max_element_number + index + 1 + CACHE_SIZE,
                                                 MAX_ELEMENT_NODE);
    assert(node->prev->current_max_element_number <=MAX_ELEMENT_NODE);
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
 * Return an index as 0 <= index <= line->element_number
 */
LineIdentifier moduloLineIdentifier(LineNode* line, int column) {
  while (column < 0) {
    column += line->element_number;
    assert(line->prev != NULL); // Index out of range
    line = line->prev;
  }

  while (column > line->element_number) {
    column -= line->element_number;
    assert(line->next != NULL); // Index out of range
    line = line->next;
  }
  LineIdentifier id;
  id.last_shift = 0;
  id.line = line;
  id.relative_column = column;

  return id;
}


/**
 * Insert a char at index of the line node.
 * Return the new relative index for line cell.
 */
LineIdentifier insertCharInLine(LineNode* line, Char_U8 ch, int column) {
  LineIdentifier id = moduloLineIdentifier(line, column);
  line = id.line;
  column = id.relative_column;

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

  id.last_shift = relative_shift;
  id.line = line;
  id.relative_column = column + 1;

  return id;
}


/**
 * Insert a char at index of the line node.
 */
LineIdentifier removeCharInLine(LineNode* line, int cursorPos) {
#ifdef LOGS
  printf("ABSOLUTE INDEX %d\r\n", cursorPos);
  printf("ABSOLUTE LINE ELEMENT NUMBER %d\n\r", line->element_number);
#endif


  LineIdentifier id = moduloLineIdentifier(line, cursorPos);
  id.relative_column--;
  line = id.line;
  cursorPos = id.relative_column;

  if (cursorPos == -1) {
    assert(line->prev != NULL);
    line = line->prev;
    cursorPos = line->element_number;
    id.line = line;
    id.relative_column = cursorPos;
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
        assert(line->fixed == false);
        line = destroyCurrentLineNode(line);
        id.line = line;
        id.relative_column = id.line->element_number;
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
  return id;
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
    free(tmp);
  }
}

/**
 * Destroy line letting this current node.
 */
void destroyChildLine(LineNode* node) {
  assert(node != NULL);
  free(node->ch);
  node = node->next;

  while (node != NULL) {
    LineNode* tmp = node;
    node = node->next;

    free(tmp->ch);
    free(tmp);
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
    destroyChildLine(file->lines + i);
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
    file->next->lines = realloc(file->next->lines, file->next->current_max_element_number * sizeof(LineNode));
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
    assert(file->prev->current_max_element_number <=MAX_ELEMENT_NODE);
#ifdef LOGS
    printf(" new size %d\n\r", file->prev->current_max_element_number);
#endif
    file->prev->lines = realloc(file->prev->lines, file->prev->current_max_element_number * sizeof(LineNode));
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
      file->lines = realloc(file->lines, file->current_max_element_number * sizeof(LineNode));
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

    if (row == 0) {
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
FileIdentifier moduloFileIdentifier(FileNode* file, int row) {
  while (row < 0) {
    row += file->element_number;
    assert(file->prev != NULL); // Index out of range
    file = file->prev;
  }

  while (row > file->element_number) {
    row -= file->element_number;
    assert(file->next != NULL); // Index out of range
    file = file->next;
  }

  FileIdentifier id;
  id.last_shift = 0;
  id.file = file;
  id.relative_row = row;

  return id;
}


/**
 * Insert a char at index of the line node.
 * Return the new relative index for line cell.
 */
FileIdentifier insertEmptyLineInFile(FileNode* file, int row) {
  FileIdentifier id = moduloFileIdentifier(file, row);
  file = id.file;
  row = id.relative_row;

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
  }

  id.last_shift = relative_shift;
  id.file = file;
  id.relative_row = row + 1;

  return id;
}


/**
 * Insert a char at index of the line node.
 */
FileIdentifier removeLineInFile(FileNode* file, int row) {
#ifdef LOGS
  printf("ABSOLUTE INDEX %d\r\n", row);
  printf("ABSOLUTE LINE ELEMENT NUMBER %d\n\r", file->element_number);
#endif


  FileIdentifier id = moduloFileIdentifier(file, row);
  id.relative_row--;
  file = id.file;
  row = id.relative_row;

  if (row == -1) {
    assert(file->prev != NULL);
    file = file->prev;
    row = file->element_number;
    id.file = file;
    id.relative_row = row;
  }

#ifdef LOGS
  printf("REMOVE CHAR RELATIVE %d \r\n", row);
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
      id.file = file;
      id.relative_row = id.file->element_number;
    }
    else {
      if (file->current_max_element_number > CACHE_SIZE + 1) {
#ifdef LOGS
        printf("REALLOC ONLY CACHE\r\n");
#endif
        file->current_max_element_number = min(MAX_ELEMENT_NODE, CACHE_SIZE);
        assert(file->current_max_element_number <= MAX_ELEMENT_NODE);
        file->lines = realloc(file->lines, file->current_max_element_number * sizeof(LineNode));
      }
    }
  }
#ifdef LOGS
  printf("REMOVE ENDED\r\n");
#endif
  return id;
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
      destroyChildLine(tmp->lines + i);
    }

    free(tmp->lines);
    free(tmp);
  }
}


LineIdentifier identifierForCursor(FileNode* file, int row, int column) {
  FileIdentifier file_id = moduloFileIdentifier(file, row);
  LineIdentifier line_id = moduloLineIdentifier(file_id.file->lines + file_id.relative_row - 1, column);

  return line_id;
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
Char_U8 getCharAt(FileNode* file, int line, int column);
