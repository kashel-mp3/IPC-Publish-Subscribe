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

int subscribe_modify(int client_q , int server_q, const char* topicname, int duration) {
    struct message subMessage;
    subMessage.mtype = CR_ADD_SUB;
    subMessage.id = client_q;
    subMessage.cnt = duration;
    strcpy(subMessage.topicname, topicname);
    msgsnd(server_q, &subMessage, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &subMessage, sizeof(struct message) - sizeof(long), CR_ADD_SUB, 0);
    return subMessage.id == SR_ERR;
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
    strcpy(entry->text, topicListMessege.topic_list); // to się może nie zmieścić, należałoby to ciąć do 100 znaków, znajdując pierwszy przecinek na indeksie mniejszym od 100 (maksymalna długość wiadomości)
    addMessageToBuffer(mlb, entry);
}

int send_message(int server_q, int client_q , char* topic, char* username, char* text) {
    struct message textMessage;
    textMessage.mtype = CR_TEXTMSG;
    strcpy(textMessage.topicname, topic);
    strcpy(textMessage.username, username);
    strcpy(textMessage.text, text);
    return messageSend(server_q, &textMessage);
}

int block_user(int server_q, int client_q, char* username) {
    struct message muteMessage;
    muteMessage.mtype = CR_MUTE;
    strcpy(muteMessage.username, username);
    messageSend(server_q, &muteMessage);
    messageReceive(client_q, &muteMessage, CR_MUTE);
    return muteMessage.id == SR_ERR;
}

int main() {
    int server_q = msgget(SERVER_KEY, IPC_CREAT | 0666); // czy klient powinien tworzyć czy tu ma być błont, czy ma czekać czy co?
    int client_q = msgget(getpid() + 10000, IPC_CREAT | 0666);
    char username[USERNAME_LEN];
    do{
        printf("Provide username: ");
        scanf("%s", username);
        
    }while(send_login(client_q, server_q, username));

    char* topic = (char*) malloc(sizeof(char) * MAX_TOPIC_LENGTH);
    struct messageLogBuffer* messageLogBuffer = createMessageLogBuffer();
    struct threadData* data = createThreadData();
    pthread_t inputThreadId;


    setNonBlockingMode();
    setvbuf(stdout, NULL, _IONBF, 0);
    strcpy(topic, DEFAULT_TOPIC); // default topic;

    if(pthread_create(&inputThreadId, NULL, inputThreadFunction, (void*) data) != 0){
        perror("Failed to create input thread");
        exit(EXIT_FAILURE);
    }

    int duration;
    struct message msg;
    while (1) {
        displayUI(messageLogBuffer, data, topic);

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
                }
                else if(strncmp(data->inputBuffer, "/help", 5) == 0) {
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/topic [topic name]        switches the topic"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/newtopic [topic name]     creates new topic"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/topiclist                 displays list of currenty topics"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/sub [topic name] [type]   creates an subscription"));
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("HELP", "/unsub [topic name]        deletes subscription if one exists"));
                }
                else if(strncmp(data->inputBuffer, "/sub ", 5) == 0) {
                    int argLen = strlen(data->inputBuffer + 5);
                    if(argLen == 0){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "missing argument: topic"));
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "too long"));
                    }
                    else if(isAlnumOnly(data->inputBuffer + 5, argLen)){
                        addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "invalid arguent"));
                    }
                    else{
                        struct message msg;
                        msg.mtype = CR_ADD_SUB;
                        msg.id = client_q;
                        strcpy(msg.topicname, data->inputBuffer + 5);
                        argLen = strlen(data->inputBuffer + 5 + argLen);
                        int arg = atoi(data->inputBuffer + 5 + argLen);
                        if(argLen == 0){
                            duration = -1;
                        } else if(arg <= 0) {
                            addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "invalid argument: duration has to be a positive number"));
                        } else {
                            duration = arg;
                        }
                        
                        msg.cnt = duration;
                        msgsnd(server_q, &msg, sizeof(struct message) - sizeof(long), 0);
                        msgrcv(client_q, &msg, sizeof(struct message) - sizeof(long), CR_ADD_SUB, 0);
                        if(msg.id == SR_ERR) {
                            addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", msg.text));
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
                        topic = strcpy(topic, data->inputBuffer + 7);
                    }
                }
                else{
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "Invalid command "));
                }
            } 
            else {
                //addMessageToBuffer(messageLogBuffer, createMessageEntry(topic, data->inputBuffer));
                // TODO send message to server (nawet zamiast tego dodania do koeljki)
                if(send_message(server_q, client_q, topic, username, data->inputBuffer)){
                    addMessageToBuffer(messageLogBuffer, createMessageEntry("ERROR", "message went bad :("));
                }
            }
            data->inputReady = 0;
            pthread_mutex_unlock(&data->inputLock);
            //printf("input unlocked\n");
        }
        usleep(100000);
    }

    deleteMessageLogBuffer(messageLogBuffer);
    deleteThreadData(data);
    resetNonBlockingMode();

    return 0;
}