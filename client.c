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

int send_login(int client_q , int server_q) {
    struct message login;
    login.mtype = CR_LOGIN;
    login.id = client_q;
    bool done = 0;
    printf("Provide username:\n");
    char* unchecked_username = (char*)malloc(50); //free?
    scanf("%s", unchecked_username);
    
    if(strlen(unchecked_username) > USERNAME_LEN) {
        printf("Invalid login: too long, try again\n\n");
        return 1;
    } else {
        strcpy(login.username, unchecked_username);
    }

    msgsnd(server_q, &login, sizeof(struct message) - sizeof(long), 0);
    struct message response;

    msgrcv(client_q, &response, sizeof(struct message) - sizeof(long), 0, 0);
    if(response.mtype == SR_ERR) {
        printf("%s\n", response.text);
        return 1;
    }
    return 0;
}

int subscribe(int client_q , int server_q) {
    struct message topic_request, topic_reply;
    topic_request.mtype = CR_REQ_TOPIC;
    topic_request.id = client_q;
    msgsnd(server_q, &topic_request, sizeof(struct message) - sizeof(long), 0);
    msgrcv(client_q, &topic_reply, sizeof(struct message) - sizeof(long), 0, 0);

    printf("Current topics:\n");
    printf("%s\n", topic_reply.topic_list);

    struct message subscribtion;
    subscribtion.mtype = CR_ADD_SUB;
    subscribtion.id = client_q;
    printf("to which topic would you like to subscribe? [type its idex]\n");
    int topic_id;
    scanf("%d", &topic_id);
    if(topic_id < 0 && topic_reply.cnt <= topic_id) {
        printf("topic of this index does not exist\n\n");
        return 1;
    }
    subscribtion.topic_id = topic_id;

    int topic_cnt;
    bool done = 0;
    while(!done) {
        printf("indef (0), number of messeges (n>0)\n");
        scanf("%d", &topic_id);
        if(topic_cnt < 0) {
            printf("has to be a positive number\n\n");
            continue;
        }
        done = 1;
    }
    subscribtion.cnt = topic_cnt; 

    msgsnd(server_q, &subscribtion, sizeof(struct message) - sizeof(long), 0);
    struct message response;
    msgrcv(client_q, &response, sizeof(struct message) - sizeof(long), 0, 0);
    if(response.mtype == SR_ERR) {
        printf("%s\n", response.text);
        return 1;
    }
    return 0;
}

int modify_subscription(int client_q , int server_q) {
    return 0;
}

int create_topic(int client_q , int server_q) {
    struct message topic;
    topic.mtype = CR_CREAT_TOPIC;
    topic.id = client_q;
    printf("what is the name of the topic that you want to create?\n");
    char* unchecked_name = (char*)malloc(50); //free?
    scanf("%s", unchecked_name);
    
    if(strlen(unchecked_name) > TOPIC_LEN) {
        printf("Invalid topic name: too long, try again\n\n");
        return 1;
    } else {
        strcpy(topic.topicname, unchecked_name);
    }
    msgsnd(server_q, &topic, sizeof(struct message) - sizeof(long), 0);
    struct message response;
    msgrcv(client_q, &response, sizeof(struct message) - sizeof(long), 0, 0);
    if(response.mtype == SR_ERR) {
        printf("%s\n", response.text);
        return 1;
    }
    return 0;
}

int send_message(int server_q, int client_q , char* text) {
    struct message textMessage;
    textMessage.mtype = CR_TEXTMSG;
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
    int client_q = msgget(getpid(), IPC_CREAT | 0666);
    while(send_login(client_q, server_q)) {};

    char* topic = (char*) malloc(sizeof(char) * MAX_TOPIC_LENGTH);
    struct messageLogBuffer* messageLogBuffer = createMessageLogBuffer();
    struct threadData* data = createThreadData();
    pthread_t inputThreadId;


    setNonBlockingMode();
    setvbuf(stdout, NULL, _IONBF, 0);
    strcpy(topic, "RANDOM"); // default topic;

    if(pthread_create(&inputThreadId, NULL, inputThreadFunction, (void*) data) != 0){
        perror("Failed to create input thread");
        exit(EXIT_FAILURE);
    }

    while (1) {
        displayUI(messageLogBuffer, data, topic);

        //TODO incoming messages handling

        if(data->inputReady){
            if (data->inputBuffer[0] == '/') {
                if(strncmp(data->inputBuffer, "/topic ", 7) == 0){
                    int argLen = strlen(data->inputBuffer + 7);
                    if(argLen == 0){
                        //TODO missing argument
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        //TODO topic too long
                    }
                    else if(isAlnumOnly(data->inputBuffer + 7, argLen)){
                        //TODO invalid argument !ALNUM
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
                addMessageToBuffer(messageLogBuffer, createMessageEntry(topic, data->inputBuffer));
                // TODO send message to server (nawet zamiast tego dodania do koeljki)
            }
            data->inputReady = 0;
            pthread_mutex_unlock(&data->inputLock);
            //printf("input unlocked\n");
        }
        usleep(100000);
    }

    deleteThreadData(data);
    resetNonBlockingMode();

    return 0;
}