/*
 * Partial implementation of : https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/
 */
#ifndef CLIENT_H
#define CLIENT_H
#include <stdbool.h>
#include <cjson/cJSON.h>
#include <sys/types.h>


typedef enum {
  REQUEST,
  NOTIFICATION,
  RESPONSE
} PACKET_TYPE;

typedef struct {
  char name[100];
  char language[100];
  pid_t pid;
  int inpipefd[2];
  int outpipefd[2];
  cJSON* init_result;
} LSP_Server;


//////// ---------------- JSON TOOLS ---------------------

void printToStdioJSON(cJSON* json);


//////// --------------- Low Layer JSON-RPC --------------------------

bool LSP_openLSPServer(char* name, char* command_args, char* language, LSP_Server* server);

void LSP_closeLSPServer(LSP_Server* server);

char* LSP_readPacket(LSP_Server* server);

cJSON* LSP_readPacketAsJSON(LSP_Server* server, bool block);

int LSP_sendPacket(LSP_Server* server, char* method, char* params, PACKET_TYPE type);

int LSP_sendPacketWithJSON(LSP_Server* server, char* method, cJSON* content, PACKET_TYPE type);

PACKET_TYPE LSP_getRequestType(cJSON* content);

//////// ----------------- Created Functions ---------------

//// -------- Tools Functions --------

void LSP_initializeServer(LSP_Server* lsp, char* client_name, char* client_version, char* current_workspace);

cJSON* LSP_extractPacketResult(cJSON* response_obj);

int LSP_getRequestID(cJSON* request_body);

char* LSP_getRequestMethod(cJSON* request_body);

cJSON* LSP_getNotificationParams(cJSON* notification_body);

cJSON* LSP_getJSONPosition(int cursor_row, int cursor_column);

cJSON* LSP_getJSONRange(int cur1_row, int cur1_column, int cur2_row, int cur2_column);

cJSON* LSP_getJSONTextDocumentItem(char* file_name, char* languageId, int version, char* text);

cJSON* LSP_getJSONTextDocumentIdentifier(char* file_name);

cJSON* LSP_getJSONTextDocumentPositionParams(char* file_name, int cur_row, int cur_column);

cJSON* LSP_getJSONTextEdit(int cur1_row, int cur1_column, int cur2_row, int cur2_column, char* new_text);

cJSON* LSP_getJSONTextDocumentEdit(char* file_name, int cur1_row, int cur1_column, int cur2_row, int cur2_column, char* new_text);

cJSON *LSP_getJSONLocation(char *file_name, int cur1_row, int cur1_column, int cur2_row, int cur2_column);

//// -------- Receive Functions --------

bool LSP_dispatchOnReceive(LSP_Server* lsp, void (*dispatcher)(cJSON* packet, long* payload), long* payload);


//// -------- Send Functions --------

void LSP_notifyLspFileDidOpen(LSP_Server lsp, char* file_name, char* file_content);


#endif //CLIENT_H
