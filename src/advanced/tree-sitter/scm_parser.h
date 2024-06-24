#ifndef SCM_PARSER_H
#define SCM_PARSER_H

#include "tree_manager.h"


#define MAX_LENGTH_SYMBOL 4096


void initTreePathSeq(TreePathSeq* seq);

void destroyTreePathSeq(TreePathSeq* seq);

int lengthTreePathSeq(TreePathSeq *seq);

TreePathSeq* pushAfterNewTreePathSeqNode(TreePathSeq* seq);

void pushSymbol(TreePathSeq* seq, char* name);

void pushField(TreePathSeq* seq, char* name);

void pushMatch(TreePathSeq* seq, char* name);

void pushGroup(TreePathSeq* seq, char* name);

void pushRegex(TreePathSeq* seq, char* name);


bool parseSCMFile(TreePathSeq* seq, char* file_name);

void printTreePathSeq(TreePathSeq* seq) ;


void sortTreePathSeqByDecreasingSize(TreePathSeq* seq);

#endif //SCM_PARSER_H
