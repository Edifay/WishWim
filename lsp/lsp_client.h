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
  NOTIFICATION
} PACKET_TYPE;

typedef struct {
  char name[100];
  pid_t pid;
  int inpipefd[2];
  int outpipefd[2];
  cJSON* init_result;
} LSP_Server;


//////// ---------------- JSON TOOLS ---------------------

void printToStdioJSON(cJSON *json);


//////// --------------- Low Layer JSON-RPC --------------------------

bool openLSPServer(char* name, LSP_Server* server);

void closeLSPServer(LSP_Server* server);

char* readPacket(LSP_Server* server);
cJSON* readPacketAsJSON(LSP_Server* server);

int sendPacket(LSP_Server* server, char* method, char* params, PACKET_TYPE type);
int sendPacketWithJSON(LSP_Server* server, char* method, cJSON* content, PACKET_TYPE type);

//////// ----------------- Created Functions ---------------


void initializeLspServer(LSP_Server* lsp, char* client_name, char* client_version, char* current_workspace);




cJSON* extractPacketResult(cJSON* response_obj);

int getRequestID(cJSON *request_body);

PACKET_TYPE getPacketType(cJSON *content);

#endif //CLIENT_H
