#include "parameters.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "messageTypes.h"
#include "ui.h"

int messageSend(int queueId, struct message* msg){
    return msgsnd(queueId, msg, sizeof(struct message) - sizeof(long), 0);
}

struct message* messageReceive(int queueId, struct message* msg, long mtype){
    msgrcv(queueId, msg, sizeof(struct message) - sizeof(long), 0, 0);
    return msg;
}

int send_login(int client_q , int server_q, const char* username) {
    struct message loginMessage;
    loginMessage.mtype = CR_LOGIN;
    loginMessage.id = client_q;
    strcpy(loginMessage.username, username);
    msgsnd(server_q, &loginMessage, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &loginMessage, sizeof(struct message) - sizeof(long), CR_LOGIN, 0);
    return loginMessage.id == SR_ERR;
}

struct message subscribe_modify(int client_q , int server_q, const char* topicname, int duration) {
    struct message subMessage;
    subMessage.mtype = CR_ADD_SUB;
    subMessage.id = client_q;
    subMessage.sub_duration = duration;
    strcpy(subMessage.topicname, topicname);
    msgsnd(server_q, &subMessage, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &subMessage, sizeof(struct message) - sizeof(long), CR_ADD_SUB, 0);
    return subMessage;
}

int unsubscribe(int client_q , int server_q, const char* topicname) {
    struct message unsubMessage;
    unsubMessage.mtype = CR_UNSUB;
    unsubMessage.id = client_q;
    strcpy(unsubMessage.topicname, topicname);
    msgsnd(server_q, &unsubMessage, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &unsubMessage, sizeof(struct message) - sizeof(long), CR_UNSUB, 0);
    return unsubMessage.id == SR_ERR;
}

int create_topic(int client_q , int server_q, const char* topicname) {
    struct message topicMessage;
    topicMessage.mtype = CR_CREAT_TOPIC;
    topicMessage.id = client_q;
    strcpy(topicMessage.topicname, topicname);
    msgsnd(server_q, &topicMessage, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &topicMessage, sizeof(struct message) - sizeof(long), CR_CREAT_TOPIC, 0);
    return topicMessage.id == SR_ERR;
}

void display_topiclist(int client_q , int server_q, struct messageLogBuffer* mlb) {
    struct message topicListMessege;
    topicListMessege.mtype = CR_REQ_TOPIC;
    topicListMessege.id = client_q;
    msgsnd(server_q, &topicListMessege, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &topicListMessege, sizeof(struct message) - sizeof(long), CR_REQ_TOPIC, 0);
    struct messageEntry* entry = (struct messageEntry*) malloc(sizeof(struct messageEntry));
    strcpy(entry->topic, "TOPIC LIST");
    strcpy(entry->text, topicListMessege.topic_list);
    addMessageToBuffer(mlb, entry);
}

int send_message(int server_q, int client_q , char* topic, char* username, char* text) {
    struct message textMessage;
    textMessage.mtype = CR_TEXTMSG;
    textMessage.id = client_q;
    strcpy(textMessage.topicname, topic);
    strcpy(textMessage.username, username);
    strcpy(textMessage.text, text);
    return messageSend(server_q, &textMessage);
}

struct message block_user(int server_q, int client_q, char* username) {
    struct message muteMessage;
    muteMessage.mtype = CR_MUTE;
    muteMessage.id = client_q;
    strcpy(muteMessage.username, username);
    messageSend(server_q, &muteMessage);
    messageReceive(client_q, &muteMessage, CR_MUTE);
    return muteMessage;
}

int checkTopic(int server_q, int client_q, char* topicname){
    struct message topicMessage;
    topicMessage.mtype = CR_TOPIC;
    topicMessage.id = client_q;
    strcpy(topicMessage.topicname, topicname);
    messageSend(server_q, &topicMessage);
    messageReceive(client_q, &topicMessage, CR_TOPIC);
    return topicMessage.id == SR_ERR;
}

char* parseInput(char* input, int* parsedInt) {
    char* spacePosition = strchr(input, ' ');
    char* output;
    if(spacePosition != NULL) {
        size_t wordLength = spacePosition - input;
        output = (char*)malloc(wordLength + 1);
        strncpy(output, input, wordLength);
        (output)[wordLength] = '\0'; 
        if(*(spacePosition + 1) != '\0') {
            *parsedInt = atoi(spacePosition + 1);
        } else {
            *parsedInt = -1;
        }
    } else {
        output = input;
        *parsedInt = -1;
    }
    return output;
}


int main() {
    int server_q = msgget(SERVER_KEY, IPC_CREAT | 0666);
    int client_q = msgget(getpid() + 10000, IPC_CREAT | 0666);
    char username[USERNAME_LEN];
    do{
        printf("Provide username: ");
        scanf("%s", username);
        
    }while(send_login(client_q, server_q, username));

    char* topic = (char*) malloc(sizeof(char) * MAX_TOPIC_LENGTH);
    struct messageLogBuffer* messageLogBuffer = createMessageLogBuffer();
    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "Type /help to get some help."));
    struct threadData* data = createThreadData();
    pthread_t inputThreadId;


    setNonBlockingMode();
    setvbuf(stdout, NULL, _IONBF, 0);
    strcpy(topic, DEFAULT_TOPIC);

    if(pthread_create(&inputThreadId, NULL, inputThreadFunction, (void*) data) != 0){
        perror("Failed to create input thread");
        exit(EXIT_FAILURE);
    }

    struct message msg;
    while (1) {
        displayUI(messageLogBuffer, data, topic, username);

        if (msgrcv(client_q, &msg, sizeof(struct message), -CR_TEXTMSG, IPC_NOWAIT) != -1) {
            char* header = (char*) malloc(sizeof(char) * (USERNAME_LEN + 1 + TOPIC_LEN));
            strcpy(header, msg.username);
            strcat(header, "@");
            strcat(header, msg.topicname);
            addMessageToBuffer(messageLogBuffer, createMessageEntry(header, msg.text));
            free(header);
        }

        if(data->inputReady){
            if (data->inputBuffer[0] == '/') {
                if(strncmp(data->inputBuffer, "/topic ", 7) == 0){
                    int argLen = strlen(data->inputBuffer + 7);
                    if(argLen == 0){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "missing argument: topic"));
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "too long"));
                    }
                    else if(isAlnumOnly(data->inputBuffer + 7, argLen)){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "invalid arguent"));
                    }
                    else if(checkTopic(server_q, client_q, data->inputBuffer + 7)){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "this topic does not exist"));
                    }
                    else{
                        strcpy(topic, data->inputBuffer + 7);
                    }
                } else if(strncmp(data->inputBuffer, "/newtopic ", 10) == 0) {
                    int argLen = strlen(data->inputBuffer + 10);
                    if(argLen == 0){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "missing argument: topic"));
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "too long"));
                    }
                    else if(isAlnumOnly(data->inputBuffer + 10, argLen)){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "invalid arguent"));
                    }
                    else{
                        strcpy(topic, data->inputBuffer + 10);
                        if(create_topic(client_q, server_q, data->inputBuffer + 10)){
                            addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "topic with this name already exists"));
                        }
                    }
                } else if(strncmp(data->inputBuffer, "/topiclist", 10) == 0) {
                    display_topiclist(client_q, server_q, messageLogBuffer);
                }
                else if(strncmp(data->inputBuffer, "/help", 5) == 0) {
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/topic [topic name]        switches the topic"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/newtopic [topic name]     creates new topic"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/topiclist                 displays list of currenty topics"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/sub [topic name] [type]   creates an subscription"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/unsub [topic name]        deletes subscription if one exists"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/mute [username]           mutes user based on username"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/quit                      exits the program"));
                }
                else if(strncmp(data->inputBuffer, "/sub ", 5) == 0) {
                    int argLen = strlen(data->inputBuffer + 5);
                    if(argLen == 0){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "missing argument: topic"));
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "too long"));
                    }
                    else{
                        int duration = -1;
                        char* topicName = parseInput(data->inputBuffer + 5, &duration);
                        if(duration <= 0 && duration != -1){
                            addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "sub duration has to be a positive number"));
                        }
                        else{
                            struct message msg = subscribe_modify(client_q, server_q, topicName, duration);
                            if(msg.id == SR_ERR) {
                                addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", msg.text));
                            } else {
                                addMessageToBuffer(messageLogBuffer, createMessageEntry("INFO", "subscription sucessfully added"));
                            }
                        }
                    }
                }
                else if(strncmp(data->inputBuffer, "/unsub ", 7) == 0) {
                    int argLen = strlen(data->inputBuffer + 7);
                    if(argLen == 0){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "missing argument: topic"));
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "too long"));
                    }
                    else if(isAlnumOnly(data->inputBuffer + 7, argLen)){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "invalid arguent"));
                    }
                    else{
                        if(unsubscribe(client_q, server_q, data->inputBuffer + 7) == SR_ERR) {
                            addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "error cancelling subscription"));
                        } else {
                            addMessageToBuffer(messageLogBuffer, createMessageEntry("INFO", "topic subscribtion cancelled"));
                        }
                    }
                }
                else if(strncmp(data->inputBuffer, "/mute ", 6) == 0){
                    char* muteUsername = (char*) malloc(sizeof(char) * USERNAME_LEN);
                    strcpy(muteUsername, data->inputBuffer + 6);
                    struct message msg = block_user(server_q, client_q, muteUsername);
                    if(msg.id == SR_ERR){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "Something went wrong with blocking."));
                    }
                    else{
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("INFO", msg.text));
                    }
                    free(muteUsername);
                }
                else if(strncmp(data->inputBuffer, "/quit", 5) == 0){
                    break;
                }
                else{
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "Invalid command "));
                }
            } 
            else {
                if(send_message(server_q, client_q, topic, username, data->inputBuffer)){
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "message went bad :("));
                }
            }
            data->inputReady = 0;
            pthread_mutex_unlock(&data->inputLock);
        }
        usleep(100000);
    }
    
    pthread_cancel(inputThreadId);
    clearScreen();
    deleteMessageLogBuffer(messageLogBuffer);
    deleteThreadData(data);
    resetNonBlockingMode();

    return 0;
}