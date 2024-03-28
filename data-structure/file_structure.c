#include "file_structure.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>


int min(int a, int b) {
  if (a < b)
    return a;
  return b;
}

/**
 *  Init a new empty head of FileNode.
 */
void initEmptyFileNode(FileNode* file) {
  file->next = NULL;
  file->prev = NULL;
  file->lines = NULL;
  file->current_max_element_number = 0;
  file->element_number = 0;
}

/**
 *  Init a new empty head of LineNode.
 */
void initEmptyLineNode(LineNode* line) {
  assert(line != NULL);
  line->next = NULL;
  line->prev = NULL;
  line->ch = NULL;
  line->current_max_element_number = 0;
  line->element_number = 0;
}


/**
 *  Init a new head of FileNode.
 */
void initFileNode(FileNode* file, Size size);

/**
 *  Init a new head of LineNode.
 */
void initLineNode(LineNode* line, Size size);

/**
 *  Return the number of line in the FileNode.
 */
int sizeFileNode(FileNode* file);

/**
 *  Return the number of char in the LineNode.
 */
int sizeLineNode(LineNode* line);

/**
 * Return the char from the FileNode at index line, column. If file[line][column] don't exist return null.
 */
Char_U8 getCharAt(FileNode* file, int line, int column);


/**
 * Insert an EMPTY node after the node given.
 */
LineNode* insertNodeBefore(LineNode* node) {
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
LineNode* insertNodeAfter(LineNode* node) {
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
 * -1     => failed.
 * 0      => access to index 0 from the next node
 * other  => Number of Char_U8 moved to previous node.
 */
int slideFromNodeToNextNodeAfterIndex(LineNode* node, int index) {
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
  printf("SLIDING RIGHT NUMBER %d\r\n", moved);
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
int slideFromNodeToPreviousNodeBeforeIndex(LineNode* node, int index) {
  if (node->prev == NULL)
    return -1;

  if (node->prev->element_number == MAX_ELEMENT_NODE)
    return -1;


  if (node->prev->current_max_element_number != MAX_ELEMENT_NODE && index + 1 > node->prev->current_max_element_number -
      node->prev->element_number) {
    // If previous is not already full allocated && need more space to store
    printf("Realloc previous ");
    node->prev->current_max_element_number = min(node->prev->current_max_element_number + index + 1 + CACHE_SIZE,
                                                 MAX_ELEMENT_NODE);
    assert(node->prev->current_max_element_number <=MAX_ELEMENT_NODE);
    printf(" new size %d\n\r", node->prev->current_max_element_number);
    node->prev->ch = realloc(node->prev->ch, node->prev->current_max_element_number * sizeof(Char_U8));
  }
  printf("Min Of Index : %d and %d - %d  = %d  \r\n", index, node->prev->current_max_element_number,
         node->prev->element_number, node->prev->current_max_element_number - node->prev->element_number);

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

  if (index == 0 && line->prev != NULL && line->prev->element_number != MAX_ELEMENT_NODE)
    goto skip_here;

  if (available_here >= 1) {
    // Is size available in current cell
    printf("Simple case !\n\r");


    if (line->current_max_element_number - line->element_number < 1) {
      printf("Realloc mem\n\r");
      // Need to realloc memory ?
      line->current_max_element_number = min(line->current_max_element_number + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
      assert(line->current_max_element_number <= MAX_ELEMENT_NODE);
      line->ch = realloc(line->ch, line->current_max_element_number * sizeof(Char_U8));
    }

    assert(line->current_max_element_number - line->element_number >= 1);
    return 0;
  }

  assert(available_here == 0);
skip_here:
  printf("Hard case !\n\r");

  // Current cell cannot contain the asked size.
  int available_prev = line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line->prev->element_number;
  int available_next = line->next == NULL ? 0 : MAX_ELEMENT_NODE - line->next->element_number;

  printf("AVAILABLE PREVIOUS %d | NEXT %d | CURRENT %d \r\n", available_prev, available_next, available_here);

  if (available_prev + available_next + available_here < 1) {
    // Need to insert node
    printf("INSERTING A NODE\r\n");

    if (index == 0) {
      // Due to the fact that we can just deallocate instant from right of an array only more power full for index == 0.
      insertNodeBefore(line);
      available_prev = MAX_ELEMENT_NODE;
    }
    else {
      insertNodeAfter(line);
      available_next = MAX_ELEMENT_NODE;
    }
  }

  assert(available_prev + available_next >= 1);

  int shift = 0;
  if (available_prev != 0) {
    printf("SLIDING LEFT\r\n");
    int moved = slideFromNodeToPreviousNodeBeforeIndex(line, index);
    printf("Moved : %d Shift %d\r\n", moved, shift);
    assert(moved != -1);
    shift -= moved;
  }
  else if (MAX_ELEMENT_NODE - line->element_number < 1) {
    // assert(index != MAX_ELEMENT_NODE);
    printf("SLIDING RIGHT\r\n");
    slideFromNodeToNextNodeAfterIndex(line, index);
  }

  printf("When return %d\n\r", shift);
  return shift;
}

/**
 * Return idenfier for the node containing current relative index.
 * Return an index as 0 <= index <= line->element_number
 */
Identifier moduloIdentifier(LineNode* line, int index) {
  while (index < 0) {
    index += line->element_number;
    assert(line->prev != NULL); // Index out of range
    line = line->prev;
  }

  while (index > line->element_number) {
    index -= line->element_number;
    assert(line->next != NULL); // Index out of range
    line = line->next;
  }
  Identifier id;
  id.last_shift = 0;
  id.line = line;
  id.relative_index = index;

  return id;
}


/**
 * Insert a char at index of the line node.
 * Return the new relative index for line cell.
 */
Identifier insertChar(LineNode* line, Char_U8 ch, int index) {
  Identifier id = moduloIdentifier(line, index);
  line = id.line;
  index = id.relative_index;

  int relative_shift = allocateOneCharAtIndex(line, index);
  printf("Shift : %d & Index : %d\n\r", relative_shift, index);
  index = index + relative_shift;

  assert(relative_shift <= 0);
  assert(index <= MAX_ELEMENT_NODE);
  assert(index >= 0);

  if (index == 0) {
    if (line->prev != NULL && line->prev->element_number != MAX_ELEMENT_NODE) {
      // can add previous.
      printf("SHIFT PREVIOUS DETECTED\n\r");
      assert(line->prev != NULL);
      line = line->prev;
      index = line->element_number;
    }
    else {
      // cannot add previous assert place here.
      assert(line->element_number != MAX_ELEMENT_NODE);
    }
  }
  else if (index == MAX_ELEMENT_NODE) {
    assert(line->next != NULL);
    printf("SHIFT NEXT DETECTED\n\r");
    line = line->next;
    index = 0;
  }

  if (index == line->element_number) {
    // Simple happend
    line->ch[index] = ch;
    line->element_number++;
    printf("ADD AT THE END\r\n");
  }
  else {
    // Insert in array
    printf("INSERT IN MIDDLE\r\n");
    assert(line->element_number - index > 0);
    memmove(line->ch + index + 1, line->ch + index, (line->element_number - index) * sizeof(Char_U8));
    line->ch[index] = ch;
    line->element_number++;
  }

  id.last_shift = relative_shift;
  id.line = line;
  id.relative_index = index;

  return id;
}


/**
 * Insert a char at index of the line node.
 */
Identifier removeChar(LineNode* line, int cursorPos) {
  printf("ABSOLUTE INDEX %d\r\n", cursorPos);
  printf("ABSOLUTE LINE ELEMENT NUMBER %d\n\r", line->element_number);

  Identifier id = moduloIdentifier(line, cursorPos);
  line = id.line;
  cursorPos = id.relative_index - 1;

  if (cursorPos == -1) {
    assert(line->prev != NULL);
    line = line->prev;
    cursorPos = line->element_number;
    id.line = line;
    id.relative_index = cursorPos;
  }

  printf("REMOVE CHAR RELATIVE %d \r\n", cursorPos);
  printf("CURRENT LINE ELEMENT NUMBER %d\r\n", line->element_number);

  assert(cursorPos >= 0);
  assert(cursorPos < line->element_number);

  if (cursorPos != line->element_number - 1) {
    printf("REMOVE IN MIDDLE. Table Size %d. Element Number %d\r\n", line->current_max_element_number,
           line->element_number);
    // printf("At move : %d, first %d, ")
    memmove(line->ch + cursorPos, line->ch + cursorPos + 1, (line->element_number - cursorPos - 1) * sizeof(Char_U8));
  }
  else {
    printf("REMOVE ATT END\r\n");
  }

  line->element_number--;

  printf("REMOVE ENDED\r\n");
  return id;
}


/**
 * Destroy line free all memory.
 * Will check Node before.
 */
void destroyFullLine(LineNode* node) {
  while (node->prev != NULL) {
    node = node->prev;
  }

  while (node != NULL) {
    LineNode* tmp = node;
    node = node->next;

    free(tmp->ch);
    free(tmp);
  }
}
