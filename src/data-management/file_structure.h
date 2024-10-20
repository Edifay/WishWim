#ifndef FILE_STRUCTURE_H
#define FILE_STRUCTURE_H

#include "utf_8_extractor.h"
#include <stdbool.h>


/*
 *  First this implementation is not the best, I know, but it's what I have imagined first.
 *  And wanted to try with this. The best implementation of file structure is 'ropes'.
 *
 *  My implementation of the data structure is based on the 'unrolled linked list'.
 *  But I added somes features :
 *    - Both side linked list.
 *    - Resizable arrays.
 *    - Cache features.
 *
 *  The implementation for the need of tree_sitter and lsp is support
 *
 *   - UTF8 index (row_index, column_index)
 *  and
 *   - byte_index.
 *
 *  FileNode contains lines of a file.
 *  LineNode contains Char of a line.
 *
 */

#define MAX_ELEMENT_NODE 5
#define CACHE_SIZE 5

#define LOGS

typedef unsigned int Size;

/**
 *  Containings chars from ONE line.
 */
typedef struct LineNode_ {
  bool fixed;
  struct LineNode_* next;
  struct LineNode_* prev;
  Char_U8* ch;
  int current_max_element_number;
  int element_number;
  int byte_count;
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
  int lines_byte_count[MAX_ELEMENT_NODE];
  int byte_count;
} FileNode;

/**
 * Represent a cursor in the line.
 */
typedef struct {
  int relative_column;
  int absolute_column;
  LineNode* line;
} LineIdentifier;

/**
 * Represent a cursor in lines contained in file.
 */
typedef struct {
  int relative_row;
  int absolute_row;
  FileNode* file;
} FileIdentifier;


typedef struct {
  FileIdentifier file_id;
  LineIdentifier line_id;
} Cursor;


////// --------------------UTILS-----------------------------

void memcpy_CharU8Array(Char_U8* dest, const Char_U8* src, int length);

void memcpy_LineNodeArray(LineNode* dest, const LineNode* src, int length);

void memcpy_FileNodeArray(FileNode* dest, const FileNode* src, int length);

void memcpy_IntArray(int* dest, const int* src, int length);

void memmove_CharU8Array(Char_U8* dest, const Char_U8* src, int length);

void memmove_LineNodeArray(LineNode* dest, const LineNode* src, int length);

void memmove_FileNodeArray(FileNode* dest, const FileNode* src, int length);

void memmove_IntArray(int* dest, const int* src, int length);

Char_U8* malloc_CharU8Array(int length);

LineNode* malloc_LineNodeArray(int length);

FileNode* malloc_FileNodeArray(int length);

int* malloc_IntArray(int length);

Char_U8* realloc_CharU8Array(Char_U8* array, int length);

LineNode* realloc_LineNodeArray(LineNode* array, int length);

FileNode* realloc_FileNodeArray(FileNode* array, int length);

int* realloc_IntArray(int* array, int length);


////// --------------------LINE-----------------------------

int byteCountForLineNode(LineNode* line, int index_start, int length);

int byteCountForCurrentLineToEnd(LineNode* line, int index_start);

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
Char_U8 getCharAt(FileNode* file, int row, int column);


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

LineIdentifier moduloLineIdentifier(LineIdentifier line_id);

/**
 * Insert a char at index of the line node.
 */
// LineIdentifier insertCharInLine(LineIdentifier line_id, Char_U8 ch);

/**
 * Insert a char at index of the line node.
 */
// LineIdentifier removeCharInLine(LineIdentifier line_id);

Char_U8 getCharForLineIdentifier(LineIdentifier id);

LineIdentifier getLastLineNode(LineNode* line);

int getAbsoluteLineIndex(LineIdentifier id);

bool isEmptyLine(LineNode* line);

bool hasElementAfterLine(LineIdentifier line_id);

bool hasElementBeforeLine(LineIdentifier line_id);

void printLineNode(LineNode* line);

LineIdentifier tryToReachAbsColumn(LineIdentifier line_id, int abs_column);

// void deleteLinePart(LineIdentifier line_id, int length);

/**
 * Destroy line free all memory.
 */
void destroyFullLine(LineNode* node);

////// --------------------FILE-----------------------------


int sumLinesBytes(int* array, int length);

/**
 *  Init a new empty head of FileNode.
 */
void initEmptyFileNode(FileNode* file);

FileIdentifier moduloFileIdentifierR(FileNode* file, int row);

FileIdentifier moduloFileIdentifier(FileIdentifier file_id);

// FileIdentifier insertEmptyLineInFile(FileIdentifier file_id);

// FileIdentifier removeLineInFile(FileIdentifier file_id);

int getAbsoluteFileIndex(FileIdentifier id);

LineNode* getLineForFileIdentifier(FileIdentifier id);

bool checkFileIntegrity(FileNode* file);

bool printByteCount(FileNode* file);

bool checkByteCountIntegrity(FileNode* file);

bool isEmptyFile(FileNode* file);

bool hasElementAfterFile(FileIdentifier file_id);

bool hasElementBeforeFile(FileIdentifier file_id);

FileIdentifier tryToReachAbsRow(FileIdentifier file_id, int row);

// void deleteFilePart(FileIdentifier file_id, int length);

void destroyFullFile(FileNode* node);


////// -------------- COMBO LINE & FILE --------------

Cursor initNewWrittableFile();

Cursor moduloCursorR(FileNode* file, int row, int column);

Cursor moduloCursor(Cursor cursor);

Cursor cursorOf(FileIdentifier file_id, LineIdentifier line_id);

Cursor insertCharInLineC(Cursor cursor, Char_U8 ch);

Cursor removeCharInLineC(Cursor cursor);

Cursor removeLineInFileC(Cursor cursor);

Cursor insertNewLineInLineC(Cursor cursor);

Cursor concatNeighbordsLinesC(Cursor cursor);

Cursor bulkDelete(Cursor cursor, Cursor select_cursor);

Cursor tryToReachAbsPosition(Cursor cursor, int row, int column);

Char_U8 getCharAtCursor(Cursor cursor);

bool isCursorPreviousThanOther(Cursor cursor, Cursor other);

bool isCursorStrictPreviousThanOther(Cursor cursor, Cursor other);

bool isCursorBetweenOthers(Cursor cursor, Cursor cur1, Cursor cur2);

bool areCursorEqual(Cursor cur1, Cursor cur2);

#endif
