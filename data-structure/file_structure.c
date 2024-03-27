#include "file_structure.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

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
    node->next->current_max_element_number += fmin(
      node->current_max_element_number + node->element_number - index + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
    node->next->ch = realloc(node->next->ch, node->next->current_max_element_number * sizeof(Char_U8));
  }

  if (index == MAX_ELEMENT_NODE) {
    return 0;
  }

  int moved = fmin(node->element_number - index, node->next->current_max_element_number - node->next->element_number);
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
    node->prev->current_max_element_number += fmin(node->current_max_element_number + index + 1 + CACHE_SIZE,
                                                   MAX_ELEMENT_NODE);
    node->prev->ch = realloc(node->prev->ch, node->prev->current_max_element_number * sizeof(Char_U8));
  }

  if (index == 0)
    return 0;

  int moved = fmin(index, node->prev->current_max_element_number - node->prev->element_number);
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
    goto previous_case;

  if (available_here >= 1) {
    // Is size available in current cell
    printf("Simple case !\n\r");


    if (line->current_max_element_number - line->element_number < 1) {
      printf("Realloc mem\n\r");
      // Need to realloc memory ?
      line->current_max_element_number = fmin(line->current_max_element_number + 1 + CACHE_SIZE, MAX_ELEMENT_NODE);
      line->ch = realloc(line->ch, line->current_max_element_number * sizeof(Char_U8));
    }

    assert(line->current_max_element_number - line->element_number >= 1);
    return 0;
  }

  printf("Hard case !\n\r");
  assert(available_here == 0);


  // Current cell cannot contain the asked size.
  int available_prev = line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line->prev->element_number;
  int available_next = line->next == NULL ? 0 : MAX_ELEMENT_NODE - line->next->element_number;

  printf("AVAILABLE PREVIOUS %d | NEXT %d | CURRENT %d \r\n", available_prev, available_next, available_here);

  if (available_prev + available_next + available_here < 1) {
    // Need to insert node
    printf("INSERTING A NODE\r\n");

    if (index == 0) {
      // Due to the fact that we can just deallocate instant from right of an array only more power full for index == 0.
      // TODO implement insertNodeBefore(line);
      assert(false);
      available_prev = line->prev == NULL ? 0 : MAX_ELEMENT_NODE - line->prev->element_number
          /*TODO replace by MAX_ELEMENT_NUMBER*/;
    }
    else {
      insertNodeAfter(line);
      available_next = line->next == NULL ? 0 : MAX_ELEMENT_NODE - line->next->element_number;
      // TODO replace by MAX_ELEMENT_NUMBER
    }
  }

  assert(available_prev + available_next >= 1);

  int shift = 0;
  if (available_prev != 0) {
  previous_case:
    printf("SLIDING LEFT\r\n");
    int moved = slideFromNodeToPreviousNodeBeforeIndex(line, index);
    assert(moved != -1);
    shift -= moved;
  }
  else if (MAX_ELEMENT_NODE - line->element_number < 1) {
    // assert(index != MAX_ELEMENT_NODE);
    printf("SLIDING RIGHT\r\n");
    slideFromNodeToNextNodeAfterIndex(line, index);
  }

  return shift;
}


/**
 * Insert a char at index of the line node.
 * Return the new relative index for line cell.
 */
Identifier insertChar(LineNode* line, Char_U8 ch, int index) {
  // assert(index >= 0);
  // assert(index < MAX_ELEMENT_NODE);

  while (index < 0) {
    // printf("MODULO NEG DETECTED\r\n");
    index += line->element_number;
    assert(line->prev != NULL);
    line = line->prev;
    // printf("NEW INDEX : %d\n\r", index);
  }

  while (index > line->element_number) {
    // printf("MODULO POS DETECTED\r\n");
    index -= line->element_number;
    assert(line->next != NULL);
    line = line->next;
    // printf("NEW INDEX : %d\n\r", index);
  }

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
      line = line->prev;
      index = line->prev->element_number;
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
    memmove(line->ch + index + 1, line->ch + index, (line->element_number - index) * sizeof(Char_U8));
    line->ch[index] = ch;
    line->element_number++;
  }

  Identifier id;
  id.last_shift = relative_shift;
  id.line = line;
  id.relative_index = index;

  return id;
}


/**
 * Insert a char at index of the line node.
 */
Identifier removeChar(LineNode* line, int index) {
  // TODO Implement

  Identifier id;
  return id;
}


/**
 * Destroy line free all memory.
 */
void destroyLine(LineNode* node) {
  while (node != NULL) {
    LineNode* tmp = node;
    node = node->next;

    free(tmp->ch);
    free(tmp);
  }
}
