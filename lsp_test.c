#include <assert.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "../../.config/tree-sitter/tree-sitter-c/src/tree_sitter/parser.h"
#include "lsp/lsp_client.h"
#include "utils/tools.h"
#include "tree-sitter/lib/include/tree_sitter/api.h"

#include <regex.h>

//     if (poll(&(struct pollfd){.fd = lsp.inpipefd[0], .events = POLLIN}, 1, 0) == 1) {

typedef enum {
  SYMBOL,
  FIELD,
  REGEX,
  GROUP
} PATH_TYPE;

struct TreePath_ {
  PATH_TYPE type;
  const regex_t* reg;
  const char* name;
  const char* color_group;
  const struct TreePath_* next;
};

typedef struct TreePath_ TreePath;

struct TreePathSeq_ {
  TreePath* value;
  struct TreePathSeq_* next;
};

typedef struct TreePathSeq_ TreePathSeq;

void explore(TSNode root_node, TreePath* path_symbol, int offset) {
  const char* name = ts_node_type(root_node);
  // printf("Before %p %ld \n", name, (long)name);
  // exit(0);
  path_symbol[offset].type = SYMBOL;
  path_symbol[offset].name = name;
  path_symbol[offset].color_group = NULL;
  path_symbol[offset].next = NULL;
  path_symbol[offset].reg = NULL;

  for (int i = 0; i < offset; i++) {
    // printf(" ");
  }
  for (int i = 0; i < offset + 1; i++) {
    if (path_symbol[i].type == SYMBOL || path_symbol[i].type == FIELD)
      printf("%s", path_symbol[i].name);
    if (i != offset) {
      if (path_symbol[i].type == SYMBOL)
        printf(".");
      else
        printf(":");
    }
  }
  printf("\n");

  // if (ts_node_named_child_count(root_node) == ts_node_child_count(root_node)) {
  // printf("CHILD NODE WITH NON NAMED CHILD.\n");
  // exit(0);
  // }

  for (int i = 0; i < ts_node_child_count(root_node); i++) {
    TSNode current_child = ts_node_child(root_node, i);


    if (ts_node_is_named(current_child)) {
      if (ts_node_field_name_for_child(root_node, i) != NULL) {
        offset++;
        path_symbol[offset].type = FIELD;
        path_symbol[offset].name = name;
        path_symbol[offset].color_group = NULL;
        path_symbol[offset].next = NULL;
        path_symbol[offset].reg = NULL;
      }
      explore(current_child, path_symbol, offset + 1);
      if (ts_node_field_name_for_child(root_node, i) != NULL) {
        offset--;
      }
    }
    else {
      // If is not named. Like keywords.
    }
  }
}


// Declare the `tree_sitter_json` function, which is
// implemented by the `tree-sitter-json` library.
const TSLanguage* tree_sitter_c(void);

const TSLanguage* tree_sitter_python(void);


int main(int argc, char** args) {
  TSParser* parser = ts_parser_new();

  // Set the parser's language (JSON in this case).
  const TSLanguage* lang = tree_sitter_c();
  ts_parser_set_language(parser, lang);


  // exit(0);

  FILE* f = fopen("main.c", "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET); /* same as rewind(f); */

  char* file_content = malloc(fsize + 1);
  fread(file_content, fsize, 1, f);
  fclose(f);

  file_content[fsize] = 0;


  TSTree* tree = ts_parser_parse_string(
    parser,
    NULL,
    file_content,
    strlen(file_content)
  );


  TSNode root_node = ts_tree_root_node(tree);

  TreePath* path = malloc(100 * sizeof(TreePath));
  assert(path != NULL);
  explore(root_node, path, 0);
  free(path);

  // printf("Context [%d, %d] - [%d, %d]\n", ts_node_start_point(root_node).row, ts_node_start_point(root_node).column, ts_node_end_point(root_node).row, ts_node_end_point(root_node).column);

  // char* string = ts_node_string(root_node);

  // printf("%s\n", string);

  // free(string);
  free(file_content);

  ts_tree_delete(tree);
  ts_parser_delete(parser);

  if (1)
    return 0;

  printf("Opening lsp-server.\n");

  LSP_Server lsp;

  // bool result = openLSPServer("pyright-langserver", "--stdio", &lsp);
  bool result = openLSPServer("pylsp", "-v", &lsp);
  // bool result = openLSPServer("clangd", "", &lsp);
  if (result == false) {
    printf("Failed to open lsp-server.\n");
    return 1;
  }
  printf("Server started !\n");

  initializeLspServer(&lsp, "al", "1.0", getenv("PWD"));

  // char* init_result_str = cJSON_Print(lsp.init_result);
  // printf("%s\n", init_result_str);
  // free(init_result_str);

  printf("HandShake Success.\n");

  int count = 0;
  while (count < 100) {
    if (poll(&(struct pollfd){.fd = lsp.inpipefd[0], .events = POLLIN}, 1, 0) == 1) {
      cJSON* request = readPacketAsJSON(&lsp);
      printToStdioJSON(request);
      cJSON_Delete(request);
    }

    char* file_name = "main.c";

    switch (count) {
      case 10:

        FILE* f = fopen(file_name, "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET); /* same as rewind(f); */

        char* file_content = malloc(fsize + 1);
        fread(file_content, fsize, 1, f);
        fclose(f);

        file_content[fsize] = 0;


        notifyLspFileDidOpen(lsp, file_name, file_content);

        free(file_content);
        break;

      case 50:

        cJSON* tokens_req = cJSON_CreateObject();
        cJSON* text_document = cJSON_AddObjectToObject(tokens_req, "textDocument");
        char uri[PATH_MAX];
        getLocalURI(file_name, uri);
        cJSON_AddStringToObject(text_document, "uri", uri);

        sendPacketWithJSON(&lsp, "textDocument/semanticTokens/full", tokens_req, REQUEST);

        cJSON_Delete(tokens_req);
        break;
      case 70:

        cJSON* tokens_req_delta = cJSON_CreateObject();
        cJSON_AddStringToObject(tokens_req_delta, "previousResultId", "1");
        cJSON* text_document_delta = cJSON_AddObjectToObject(tokens_req_delta, "textDocument");
        char uri_delta[PATH_MAX];
        getLocalURI(file_name, uri_delta);
        cJSON_AddStringToObject(text_document_delta, "uri", uri_delta);

        sendPacketWithJSON(&lsp, "textDocument/semanticTokens/full/delta", tokens_req_delta, REQUEST);

        cJSON_Delete(tokens_req_delta);

        break;
      default:
        break;
    }

    usleep(100000);
    count++;
  }

  closeLSPServer(&lsp);

  return 0;
}
