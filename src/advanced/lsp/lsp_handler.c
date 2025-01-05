#include "lsp_handler.h"

#include <stdlib.h>
#include <string.h>

#include "../../utils/constants.h"
#include "../../utils/tools.h"


void setLspDatas(LSP_Datas* lsp_datas, IO_FileID io_file) {
  bool did_lang_was_found = getLanguageForFile(lsp_datas->name, io_file);

  LSP_Server* lsp_server = NULL;
  if (did_lang_was_found == true) {
    lsp_server = getLSPServerForLanguage(&lsp_servers, lsp_datas->name);
  }

  if (lsp_server != NULL) {
    lsp_datas->is_enable = true;
  }
}


void initLSPServerList(LSPServerLinkedList* list) {
  list->head = NULL;
}

void destroyLSPServerList(LSPServerLinkedList* list) {
  LSPServerLinkedList_Cell* cell = list->head;
  while (cell != NULL) {
    LSPServerLinkedList_Cell* tmp = cell;
    cell = cell->next;

    LSP_closeLSPServer(&tmp->lsp_server);
    free(tmp);
  }
  list->head = NULL;
}

LSP_Server* allocateNewLSPServerToLSPServerList(LSPServerLinkedList* list) {
  LSPServerLinkedList_Cell* tmp = list->head;
  list->head = malloc(sizeof(LSPServerLinkedList_Cell));
  list->head->next = tmp;
  return &list->head->lsp_server;
}


void addLSPServerCellToLSPServerList(LSPServerLinkedList* list, LSPServerLinkedList_Cell* cell) {
  LSPServerLinkedList_Cell* tmp = list->head;
  list->head = cell;
  cell->next = tmp;
}

bool getProgName(char* language, char* prog_name, char* args) {
  // LSP_TOGGLE
  return false;

  if (strcmp(language, "bash") == 0) {
    strcpy(prog_name, "bash-language-server");
    strcpy(args, "start");
  }
  else if (strcmp(language, "c") == 0) {
    strcpy(prog_name, "clangd");
    strcpy(args, "");
  }
  else if (strcmp(language, "python") == 0) {
    strcpy(prog_name, "pylsp");
    strcpy(args, "-v");
  }
  else if (strcmp(language, "markdown") == 0) {
    strcpy(prog_name, "marksman");
    strcpy(args, "");
  }
  else if (strcmp(language, "cpp") == 0) {
    strcpy(prog_name, "clangd");
    strcpy(args, "");
  }
  else {
    return false;
  }
  return true;
}

LSP_Server* getLSPServerForLanguage(LSPServerLinkedList* list, char* language) {
  LSPServerLinkedList_Cell* cell = list->head;

  while (cell != NULL && strcmp(cell->lsp_server.language, language) != 0) {
    cell = cell->next;
  }

  if (cell == NULL) {
    // If the cell is null, it will try to open the lsp_server associated.
    cell = malloc(sizeof(LSPServerLinkedList_Cell));
    bool load_result = loadNewLSPServer(&cell->lsp_server, language);

    // Open failed.
    if (load_result == false) {
      free(cell);
      return NULL;
    }

    // Open success. Adding the lsp_server to the current list.
    addLSPServerCellToLSPServerList(&lsp_servers, cell);
  }

  return &cell->lsp_server;
}


bool loadNewLSPServer(LSP_Server* container, char* language) {
  char prog_name[1000];
  char args[1000];

  if (getProgName(language, prog_name, args) == false) {
    return false;
  }

  bool opening_result = LSP_openLSPServer(prog_name, args, language, container);

  LSP_initializeServer(container, "al", "0.5", loaded_settings.is_used ? loaded_settings.dir_path : getenv("PWD"));

  return opening_result;
}
