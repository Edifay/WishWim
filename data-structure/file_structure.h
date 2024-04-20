#ifndef FILE_STRUCTURE_H
#define FILE_STRUCTURE_H

#include "utf_8_extractor.h"
#include <stdbool.h>

#define MAX_ELEMENT_NODE 1000
#define CACHE_SIZE 120

// #define LOGS

typedef unsigned int Size;

/**
 *  Containings char from ONE line.
 */
typedef struct LineNode_ {
  bool fixed;
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
 * Represent a cursor in the line.
 */
typedef struct {
  int relative_column;
  LineNode* line;
} LineIdentifier;

/**
 * Represent a cursor in lines contained in file.
 */
typedef struct {
  int relative_row;
  FileNode* file;
} FileIdentifier;


typedef struct {
  FileIdentifier file_id;
  LineIdentifier line_id;
} Cursor;


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
 * Insert an EMPTY node before the node given.
 */
// LineNode* insertNodeBefore(LineNode* node);


/**
 * Insert an EMPTY node after the node given.
 */
// LineNode* insertNodeAfter(LineNode* node);


/**
 * -1     => failed.
 * 0      => access to index 0 from the next node
 * other  => Number of Char_U8 moved to previous node.
 */
// int slideFromNodeToNextNodeAfterIndex(LineNode* node, int index);

/**
 * -1     => failed.
 * 0      => access to last item from the previous node
 * other  => Number of Char_U8 moved to previous node.
 */
// int slideFromNodeToPreviousNodeBeforeIndex(LineNode* node, int index);

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
// int allocateOneCharAtIndex(LineNode* line, int index);

/**
 * Return idenfier for the node containing current relative index.
 */
LineIdentifier moduloLineIdentifierR(LineNode* line, int column);

LineIdentifier moduloLineIdentifier(LineIdentifier file_id);

/**
 * Insert a char at index of the line node.
 */
LineIdentifier insertCharInLine(LineIdentifier line_id, Char_U8 ch);

/**
 * Insert a char at index of the line node.
 */
LineIdentifier removeCharInLine(LineIdentifier line_id);

Char_U8* getCharForLineIdentifier(LineIdentifier id);

LineIdentifier getLastLineNode(LineNode* line);

int getAbsoluteLineIndex(LineIdentifier id);

bool isEmptyLine(LineNode* line);

void printLineNode(LineNode* line);


/**
 * Destroy line free all memory.
 */
void destroyFullLine(LineNode* node);

////// --------------------FILE-----------------------------

/**
 *  Init a new empty head of FileNode.
 */
void initEmptyFileNode(FileNode* file);

FileIdentifier moduloFileIdentifierR(FileNode* file, int row);

FileIdentifier moduloFileIdentifier(FileIdentifier file_id);

FileIdentifier insertEmptyLineInFile(FileIdentifier file_id);

FileIdentifier removeLineInFile(FileIdentifier file_id);

int getAbsoluteFileIndex(FileIdentifier id);

LineNode* getLineForFileIdentifier(FileIdentifier id);

bool checkFileIntegrity(FileNode* file);

bool isEmptyFile(FileNode* file);

void destroyFullFile(FileNode* node);


////// -------------- COMBO LINE & FILE --------------

Cursor initNewWrittableFile();

Cursor moduloCursorR(FileNode* file, int row, int column);

Cursor moduloCursor(Cursor cursor);

Cursor cursorOf(FileIdentifier file_id, LineIdentifier line_id);

Cursor insertCharInLineC(Cursor cursor, Char_U8 ch);

Cursor removeCharInLineC(Cursor cursor);

Cursor insertNewLineInLineC(Cursor cursor);

Cursor concatNeighbordsLinesC(Cursor cursor);

#endif
