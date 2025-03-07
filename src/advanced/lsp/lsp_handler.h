#ifndef LSP_HANDLER_H
#define LSP_HANDLER_H
#include <stdbool.h>

#include "lsp_client.h"
#include "../../io_management/io_manager.h"


typedef struct {
  char lang_id[100];
  bool is_enable;
} LSP_Datas;

struct _LSPServerLinkedList_Cell {
  LSP_Server lsp_server;
  struct _LSPServerLinkedList_Cell* next;
};

typedef struct _LSPServerLinkedList_Cell LSPServerLinkedList_Cell;


typedef struct {
  LSPServerLinkedList_Cell* head;
} LSPServerLinkedList;


void setLspDatas(LSP_Datas* lsp_datas, IO_FileID io_file);


//// ------------ UTILS ------------


void initLSPServerList(LSPServerLinkedList* list);

void destroyLSPServerList(LSPServerLinkedList* list);

LSP_Server* allocateNewLSPServerToLSPServerList(LSPServerLinkedList* list);

void addLSPServerCellToLSPServerList(LSPServerLinkedList* list, LSPServerLinkedList_Cell* cell);

LSP_Server* getLSPServerForLanguage(LSPServerLinkedList* list, char* language);

bool loadNewLSPServer(LSP_Server* container, char* language);


#endif //LSP_HANDLER_H
