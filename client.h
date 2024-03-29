#include "ui.h"

int messageSend(int queueId, struct message* msg);
struct message* messageReceive(int queueId, struct message* msg, long mtype);
int send_login(int client_q , int server_q, const char* username);
struct message subscribe_modify(int client_q , int server_q, const char* topicname, int duration);
int unsubscribe(int client_q , int server_q, const char* topicname);
int create_topic(int client_q , int server_q, const char* topicname);
void display_topiclist(int client_q , int server_q, struct messageLogBuffer* mlb);
int send_message(int server_q, int client_q , char* topic, char* username, char* text);
struct message block_user(int server_q, int client_q, char* username);
int checkTopic(int server_q, int client_q, char* topicname);
char* parseInput(char* input, int* parsedInt);
