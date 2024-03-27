#ifndef FILE_STRUCTURE_H
#define FILE_STRUCTURE_H

#include "utf_8_extractor.h"

#define MAX_ELEMENT_NODE 20
#define CACHE_SIZE 10

typedef unsigned int Size;

/**
 *  Containings char from ONE line.
 */
typedef struct LineNode_ {
  struct LineNode_* next;
  struct LineNode_* prev;
  Char_U8* ch;
  int current_max_element_number;
  int element_number;
} LineNode;

/**
 *  Containing lines.
 */
typedef struct FileNode_ {
  struct FileNode_* next;
  struct FileNode_* prev;
  LineNode* lines;
  int current_max_element_number;
  int element_number;
} FileNode;

/**
 * Identify a element in a node. With possible old relative index.
 */
typedef struct {
  int relative_index;
  LineNode* line;
  int last_shift;
} Identifier;


/**
 *  Init a new empty head of FileNode.
 */
void initEmptyFileNode(FileNode* file);

/**
 *  Init a new empty head of LineNode.
 */
void initEmptyLineNode(LineNode* line);


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
LineNode* insertNodeAfter(LineNode* node);

/**
 * Insert a char at index of the line node.
 */
Identifier insertChar(LineNode* line, Char_U8 ch, int index);

/**
 * Insert a char at index of the line node.
 */
Identifier removeChar(LineNode* line, int index);


/**
 * Destroy line free all memory.
 */
void destroyLine(LineNode* node);


#endif
