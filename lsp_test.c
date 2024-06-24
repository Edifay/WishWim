#include <assert.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <poll.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "advanced/lsp/lsp_client.h"
#include "advanced/tree-sitter/scm_parser.h"
#include "utils/tools.h"
#include "lib/tree-sitter/lib/include/tree_sitter/api.h"
#include "advanced/tree-sitter/tree_manager.h"

//     if (poll(&(struct pollfd){.fd = lsp.inpipefd[0], .events = POLLIN}, 1, 0) == 1) {


// Declare the `tree_sitter_json` function, which is
// implemented by the `tree-sitter-json` library.



void checkMatchForHighlight(TSNode node, TreePath tree_path[], int tree_path_length, long* args) {
  TreePathSeq* seq = ((TreePathSeq *)args[0]);
  char* source = (char *)args[1];
  // printf("Current : ");
  // for (int i = tree_path_length; i != 0; i--) {
  // if (tree_path[i].type == SYMBOL || tree_path[i].type == FIELD) {
  // if (i != tree_path_length) {
  // if (tree_path[i].type == SYMBOL)
  // printf(".");
  // else if (tree_path[i].type == FIELD)
  // printf(":");
  // }
  // printf("%s", tree_path[i].name);
  // }
  // }
  // printf("\n");
  // printf("\n");
  // printf("\n");
  while (seq != NULL) {
    // Get litteral string for current node.
    int char_nb = ts_node_end_byte(node) - ts_node_start_byte(node);
    // printf("Begin %d -> End %d\n", ts_node_end_byte(root_node), ts_node_start_byte(root_node));
    // printf("Print size : %d\n", char_nb);
    char* litteral_text_node = malloc(char_nb + 1);
    strncpy(litteral_text_node, source + ts_node_start_byte(node), char_nb);
    litteral_text_node[char_nb] = '\0';
    // printf("Litteral : %s\n", current_text);


    char* result = isTreePathMatchingQuery(litteral_text_node, tree_path, tree_path_length, seq->value);

    // printf("\nQuery : ");
    // printTreePath(seq->value);
    // printf("\n");

    free(litteral_text_node);

    if (result != NULL) {
      printf("Found group %s\n", result);
      break;
    }
    seq = seq->next;
  }
  // printf("\n\n_____________________________\n\n");
}

int main(int argc, char** args) {
  TreePathSeq highlight_queries;
  initTreePathSeq(&highlight_queries);
  bool res_parse = parseSCMFile(&highlight_queries, "lib/tree-sitter-c/queries/highlights.scm");

  if (res_parse == false)
    return 1;
  // @function (identifier) function: (call_expression)

  // printTreePathSeq(&highlight_queries);


  printf("\n\n\n----------------------------------\n\n\n");
  sortTreePathSeqByDecreasingSize(&highlight_queries);

  // printTreePathSeq(&highlight_queries);

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

  long* args_fct = malloc(2 * sizeof(long *));
  args_fct[0] = (long)&highlight_queries;
  args_fct[1] = (long)file_content;
  // TODO implement arg function.
  treeForEachNode(root_node, path, 0, checkMatchForHighlight, args_fct);


  free(path);
  free(args_fct);

  // printf("Context [%d, %d] - [%d, %d]\n", ts_node_start_point(root_node).row, ts_node_start_point(root_node).column, ts_node_end_point(root_node).row, ts_node_end_point(root_node).column);

  // char* string = ts_node_string(root_node);

  // printf("%s\n", string);

  // free(string);
  free(file_content);

  ts_tree_delete(tree);
  ts_parser_delete(parser);
  destroyTreePathSeq(&highlight_queries);


  if (1)
    return 0;

  printf("Opening lsp-server.\n");

  LSP_Server lsp;

  // bool result = openLSPServer("pyright-langserver", "--stdio", &lsp);
  // bool result = openLSPServer("pylsp", "-v", &lsp);
  bool result = openLSPServer("clangd", "", &lsp);
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
