#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <cjson/cJSON.h>
#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <linux/limits.h>

#include "lsp/lsp_client.h"
#include "utils/tools.h"

//     if (poll(&(struct pollfd){.fd = lsp.inpipefd[0], .events = POLLIN}, 1, 0) == 1) {

int main(int argc, char** args) {
  printf("Opening lsp-server.\n");

  LSP_Server lsp;

  // bool result = openLSPServer("pylsp", &lsp);
  bool result = openLSPServer("clangd", &lsp);
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
      switch (getPacketType(request)) {
        case NOTIFICATION:
          printf("It's a notification\n");
          break;
        case REQUEST:
          printf("It's a request.\n");
          break;
      }
      cJSON_Delete(request);
    }

    switch (count) {
      case 10:
        cJSON* request_content = cJSON_CreateObject();
        cJSON* text_document = cJSON_AddObjectToObject(request_content, "textDocument");

        char uri[PATH_MAX];
        getLocalURI("main.c", uri);
        cJSON_AddStringToObject(text_document, "uri", uri);
        cJSON_AddStringToObject(text_document, "languageId", "c");
        cJSON_AddNumberToObject(text_document, "version", 1);

        FILE* f = fopen("main.c", "rb");
        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET); /* same as rewind(f); */

        char* string = malloc(fsize + 1);
        fread(string, fsize, 1, f);
        fclose(f);

        string[fsize] = 0;

        cJSON_AddStringToObject(text_document, "text", string);

        sendPacketWithJSON(&lsp, "textDocument/didOpen", request_content, NOTIFICATION);
        cJSON_Delete(request_content);
        free(string);
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
