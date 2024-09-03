#include "lsp_client.h"

#include <assert.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <sys/poll.h>
#include <sys/prctl.h>
#include "../../utils/tools.h"


//////// ---------------- JSON TOOLS ---------------------

void printToStdioJSON(cJSON* json) {
  char* text = cJSON_Print(json);
  printf("%s\n", text);
  free(text);
}


//////// --------------- Low Layer JSON-RPC --------------------------


int id = 0;

// https://stackoverflow.com/questions/6171552/popen-simultaneous-read-and-write
bool LSP_openLSPServer(char* name, char* command_args, char* language, LSP_Server* server) {
  char* path = whereis(name);
  if (path == NULL) {
    return false;
  }
  char pathMemSafe[PATH_MAX];

  strcpy(server->name, name);
  strcpy(pathMemSafe, path);

  free(path);

  strncpy(server->language, language, 100);

  printf("Starting server on path : %s\n", pathMemSafe);

  // Create pipe in the 2 directions.
  pipe(server->inpipefd);
  pipe(server->outpipefd);

  server->pid = fork();
  if (server->pid == 0) {
    // Child

    // Bind Stdin/Stdout on pipes.
    dup2(server->outpipefd[0], STDIN_FILENO);
    dup2(server->inpipefd[1], STDOUT_FILENO);
    // dup2(server->inpipefd[1], STDERR_FILENO); // Do not bind the err channel it's used for lsp_server logs.


    //ask kernel to deliver SIGTERM in case the parent dies
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    //close unused pipe ends
    close(server->outpipefd[1]);
    close(server->inpipefd[0]);


    char command[strlen(name) + strlen(command_args) + strlen(" 2>> lsp_logs.txt ") + 1 /*null char*/];
    sprintf(command, "%s %s 2>> lsp_logs.txt", name, command_args);

    // system(command);
    // execl(pathMemSafe, "", (char *)NULL);

    execl("/bin/sh", "sh", "-c", command, NULL);

    exit(1);
  }
  // The code below will be executed only by parent. You can write and read
  // from the child using pipefd descriptors, and you can send signals to
  // the process using its pid by kill() function. If the child process will
  // exit unexpectedly, the parent process will obtain SIGCHLD signal that
  // can be handled (e.g. you can respawn the child process).

  //close unused pipe ends
  close(server->outpipefd[0]);
  close(server->inpipefd[1]);

  return true;
}


void LSP_closeLSPServer(LSP_Server* server) {
  kill(server->pid, SIGKILL); //send SIGKILL signal to the child process
  int status;
  waitpid(server->pid, &status, 0);
  cJSON_Delete(server->init_result);
}


void skipUntilContent(LSP_Server* server) {
}

#define BUFF_SIZE 256
#define HEADER_FIRST_FIELD "Content-Length:"

char* LSP_readPacket(LSP_Server* server) {
  char buf[BUFF_SIZE];

  // read the first head field title.
  int header_size = strlen(HEADER_FIRST_FIELD);
  int n = read(server->inpipefd[0], buf, header_size);
  buf[n] = '\0';


  // If the first header is not the expected one.
  if (n != header_size || strcmp(HEADER_FIRST_FIELD, buf) != 0) {
    fprintf(stderr, "Error with lsp read.\r\n");
    exit(0);
    return NULL;
  }

  // save the value of the first field in buf.
  int index = 0;
  while ((n = read(server->inpipefd[0], buf + index, 1)) > 0 && buf[index] != '\n') {
    index++;
    assert(index < BUFF_SIZE);
  }
  assert(index >= 1);
  buf[index - 1] = '\0';

  // Extract the content_length from buf.
  int content_length;
  sscanf(buf, " %d ", &content_length);

  // Extract the full second field. (This field is ignored).
  index = 0;
  while ((n = read(server->inpipefd[0], buf + index, 1)) > 0 && buf[index] != '\n') {
    index++;
    assert(index < BUFF_SIZE);
  }
  assert(index >= 1);
  buf[index - 1] = '\0';

#ifdef LOGS
  printf("Second header (ignored) : %s\n", buf);
#endif

  // reach first '{' some lsp servers add more break lines...  :/
  index = 0;
  while ((n = read(server->inpipefd[0], buf + index, 1)) > 0 && buf[index] != '{') {
    index++;
    assert(index < BUFF_SIZE);
  }

  // Allocate the content array
  char* content = malloc(content_length * sizeof(char) + 1 /* +1 for null char*/);
  assert(content != NULL);

  content[0] = '{'; // We already read the first '{'
  n = 0;
  while (n != content_length - 1) {
    n += read(server->inpipefd[0], content + 1 + n, content_length - 1 - n);
  }
  content[content_length] = '\0';
  assert(n == content_length - 1);

#ifdef LOGS
  printf("Readed bytes : %d\n", n);
#endif
  // If we didn't read the right number of bytes exit. May improve by the use of the buf.

  // fwrite(content, 1, content_length, stdout);
  // printf("\n");

  return content;
}

cJSON* LSP_readPacketAsJSON(LSP_Server* server, bool block) {
  if (block == false && poll(&(struct pollfd){.fd = server->inpipefd[0], .events = POLLIN}, 1, 0) != 1) {
    return NULL;
  }
  char* content_str = LSP_readPacket(server);
  cJSON* at_return = cJSON_Parse(content_str);
  if (LSP_getRequestType(at_return) != RESPONSE) {
    if (strcmp("window/logMessage", LSP_getRequestMethod(at_return)) == 0) {
      cJSON* message_obj = cJSON_GetObjectItem(LSP_getNotificationParams(at_return), "message");
      printf("Server log : %s\n", cJSON_GetStringValue(message_obj));
      cJSON_Delete(at_return);
      free(content_str);
      return LSP_readPacketAsJSON(server, block);
    }
  }
  free(content_str);
  return at_return;
}

int LSP_sendPacket(LSP_Server* server, char* method, char* params, PACKET_TYPE type) {
  if (params == NULL) {
    params = "{}";
  }
  cJSON* json_request_obj = cJSON_CreateObject();
  cJSON_AddStringToObject(json_request_obj, "jsonrpc", "2.0");
  cJSON_AddStringToObject(json_request_obj, "method", method);
  cJSON_AddRawToObject(json_request_obj, "params", params);
  if (type == REQUEST) {
    id++;
    cJSON_AddNumberToObject(json_request_obj, "id", id);
  }

  char* content_str = cJSON_PrintUnformatted(json_request_obj);
  int content_length = strlen(content_str);

  char atSend[content_length + 200];

  sprintf(atSend, "Content-Length: %d\r\n\r\n", content_length);
  char head_length = strlen(atSend);

  strcat(atSend, content_str);

  write(server->outpipefd[1], atSend, head_length + content_length);

  // fwrite(atSend, 1, head_length + content_length, stdout);
  // printf("\n");

  free(content_str);
  cJSON_Delete(json_request_obj);

  if (type == REQUEST)
    return id;
  return 0;
}

int LSP_sendPacketWithJSON(LSP_Server* server, char* method, cJSON* content, PACKET_TYPE type) {
  char* json_params = cJSON_PrintUnformatted(content);
  int temp_id = LSP_sendPacket(server, method, json_params, type);
  free(json_params);
  return temp_id;
}

PACKET_TYPE LSP_getRequestType(cJSON* content) {
  cJSON* id_obj = cJSON_GetObjectItem(content, "id");
  cJSON* method_obj = cJSON_GetObjectItem(content, "method");
  if (id_obj != NULL && method_obj == NULL) {
    return RESPONSE;
  }
  if (method_obj != NULL && id_obj == NULL) {
    return NOTIFICATION;
  }
  if (method_obj != NULL && id_obj != NULL) {
    return REQUEST;
  }

  // If there something wrong with server happened. Or something not handled.
  assert(false);
}


//////// ----------------- Created Functions ---------------


//// -------- Tools Functions --------

// TODO Implement send of Capabality
// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#initialize
void LSP_initializeServer(LSP_Server* lsp, char* client_name, char* client_version, char* current_workspace_path) {
  cJSON* init_params = cJSON_CreateObject();
  cJSON_AddNumberToObject(init_params, "processId", lsp->pid);

  cJSON* client_info = cJSON_AddObjectToObject(init_params, "clientInfo");
  cJSON_AddStringToObject(client_info, "name", client_name);
  cJSON_AddStringToObject(client_info, "version", client_version);

  cJSON* workspace_array = cJSON_AddArrayToObject(init_params, "workspaceFolders");

  cJSON* workspace = cJSON_CreateObject();
  char workspaceURI[PATH_MAX];
  getLocalURI(current_workspace_path, workspaceURI);
  cJSON_AddStringToObject(workspace, "uri", workspaceURI);
  cJSON_AddStringToObject(workspace, "name", basename(current_workspace_path));
  cJSON_AddItemToArray(workspace_array, workspace);

  cJSON* general = cJSON_AddObjectToObject(init_params, "general");
  cJSON* positionEncodings = cJSON_AddArrayToObject(general, "positionEncodings");
  cJSON_AddItemToArray(positionEncodings, cJSON_CreateString("utf-8"));

  int tmp_id = LSP_sendPacketWithJSON(lsp, "initialize", init_params, REQUEST);

  char* init_params_text = cJSON_Print(init_params);
  fprintf(stderr, "INIT_SEND:\n%s\n", init_params_text);
  free(init_params_text);

  cJSON_Delete(init_params);

  cJSON* content = LSP_readPacketAsJSON(lsp, true);
  // this assert will also check if the packet is a request.
  assert(tmp_id == LSP_getRequestID(content));
  lsp->init_result = LSP_extractPacketResult(content);
  char* init_text = cJSON_Print(lsp->init_result);
  fprintf(stderr, "INIT: \n%s\n", init_text);
  free(init_text);
  cJSON_Delete(content);
}


cJSON* LSP_extractPacketResult(cJSON* response_obj) {
  cJSON* at_return = cJSON_GetObjectItem(response_obj, "result");
  cJSON_DetachItemFromObject(response_obj, "result");
  return at_return;
}

int LSP_getRequestID(cJSON* request_body) {
  assert(LSP_getRequestType(request_body) == REQUEST || LSP_getRequestType(request_body) == RESPONSE);
  cJSON* id_obj = cJSON_GetObjectItem(request_body, "id");
  assert(id_obj != NULL);
  int current_id = cJSON_GetNumberValue(id_obj);
  return current_id;
}

char* LSP_getRequestMethod(cJSON* request_body) {
  assert(LSP_getRequestType(request_body) == REQUEST ||LSP_getRequestType(request_body) == NOTIFICATION);
  cJSON* method_obj = cJSON_GetObjectItem(request_body, "method");
  assert(method_obj != NULL);
  char* method_name = cJSON_GetStringValue(method_obj);
  return method_name;
}


cJSON* LSP_getNotificationParams(cJSON* notification_body) {
  assert(LSP_getRequestType(notification_body) == NOTIFICATION);
  cJSON* param_obj = cJSON_GetObjectItem(notification_body, "params");
  return param_obj;
}

cJSON* LSP_getJSONPosition(int cursor_row, int cursor_column) {
  cJSON* position = cJSON_CreateObject();
  cJSON_AddNumberToObject(position, "line", cursor_row - 1);
  cJSON_AddNumberToObject(position, "character", cursor_column);

  return position;
}

cJSON* LSP_getJSONRange(int cur1_row, int cur1_column, int cur2_row, int cur2_column) {
  cJSON* range = cJSON_CreateObject();
  cJSON_AddItemToObject(range, "start", LSP_getJSONPosition(cur1_row, cur1_column));
  cJSON_AddItemToObject(range, "end", LSP_getJSONPosition(cur2_row, cur2_column));

  return range;
}

cJSON* LSP_getJSONTextDocumentItem(char* file_name, char* languageId, int version, char* text) {
  cJSON* text_document = cJSON_CreateObject();

  char uri[PATH_MAX];
  getLocalURI(file_name, uri);
  cJSON_AddStringToObject(text_document, "uri", uri);
  cJSON_AddStringToObject(text_document, "languageId", languageId);
  cJSON_AddNumberToObject(text_document, "version", version);

  cJSON_AddStringToObject(text_document, "text", text);

  return text_document;
}

cJSON* LSP_getJSONTextDocumentIdentifier(char* file_name) {
  cJSON* text_document_id = cJSON_CreateObject();
  char uri[PATH_MAX];
  getLocalURI(file_name, uri);
  cJSON_AddStringToObject(text_document_id, "uri", uri);

  return text_document_id;
}

cJSON* LSP_getJSONTextDocumentPositionParams(char* file_name, int cur_row, int cur_column) {
  cJSON* text_document_position_params = cJSON_CreateObject();

  cJSON* text_document_id = LSP_getJSONTextDocumentIdentifier(file_name);
  cJSON_AddItemToObject(text_document_position_params, "textDocument", text_document_id);

  cJSON* position = LSP_getJSONPosition(cur_row, cur_column);
  cJSON_AddItemToObject(text_document_position_params, "position", position);

  return text_document_position_params;
}

cJSON* LSP_getJSONTextEdit(int cur1_row, int cur1_column, int cur2_row, int cur2_column, char* new_text) {
  cJSON* text_edit = cJSON_CreateObject();

  cJSON* range = LSP_getJSONRange(cur1_row, cur1_column, cur2_row, cur2_column);
  cJSON_AddItemToObject(text_edit, "range", range);

  cJSON_AddStringToObject(text_edit, "newText", new_text);

  return text_edit;
}

cJSON* LSP_getJSONTextDocumentEdit(char* file_name, int cur1_row, int cur1_column, int cur2_row, int cur2_column, char* new_text) {
  cJSON* text_document_edit = cJSON_CreateObject();

  cJSON* text_document_id = LSP_getJSONTextDocumentIdentifier(file_name);
  cJSON_AddItemToObject(text_document_edit, "textDocument", text_document_id);

  cJSON* edits = cJSON_AddArrayToObject(text_document_edit, "edits");
  cJSON* edit = LSP_getJSONTextEdit(cur1_row, cur1_column, cur2_row, cur2_column, new_text);
  cJSON_AddItemToArray(edits, edit);

  return text_document_edit;
}

cJSON* LSP_getJSONLocation(char* file_name, int cur1_row, int cur1_column, int cur2_row, int cur2_column) {
  cJSON* location = cJSON_CreateObject();

  char uri[PATH_MAX];
  getLocalURI(file_name, uri);
  cJSON_AddStringToObject(location, "uri", uri);

  cJSON* range = LSP_getJSONRange(cur1_row, cur1_column, cur2_row, cur2_column);
  cJSON_AddItemToObject(location, "range", range);

  return location;
}

//// -------- Receive Functions --------

bool LSP_dispatchOnReceive(LSP_Server* lsp, void (*dispatcher)(cJSON* packet, long* payload), long* payload) {
  if (lsp == NULL) {
    return false;
  }

  cJSON* packet = LSP_readPacketAsJSON(lsp, false);
  if (packet == NULL) {
    return false;
  }
  if (dispatcher != NULL) {
    dispatcher(packet, payload);
  }
  cJSON_Delete(packet);

  return true;
}


//// -------- Send Functions --------


void LSP_notifyLspFileDidOpen(LSP_Server lsp, char* file_name, char* file_content) {
  cJSON* request_content = cJSON_CreateObject();

  cJSON* text_document = LSP_getJSONTextDocumentItem(file_name, lsp.language, 1, file_content);
  cJSON_AddItemToObject(request_content, "textDocument", text_document);


  LSP_sendPacketWithJSON(&lsp, "textDocument/didOpen", request_content, NOTIFICATION);

  cJSON_Delete(request_content);
}
