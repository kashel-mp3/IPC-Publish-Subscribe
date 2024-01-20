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

struct message {
    long mtype;
    int id;
    int cnt;
    char username[USERNAME_LEN];
    char topicname[TOPIC_LEN];
    int sub_duration;
    char sub_topic[20];
    char error_text[100];
};

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
        printf("%s\n", response.error_text);
        return 1;
    }
    return 0;
}

int subscribe(int client_q , int server_q) {
    struct message subscribtion;
    subscribtion.mtype = CR_ADD_SUB;
    //geta and print all the subscriptinos from server
    printf("to which topisc would you like to subscribe?\n");
    char* unchecked_name = (char*)malloc(50); //free?
    scanf("%s", unchecked_name);
    
    if(strlen(unchecked_name) > TOPIC_LEN) {
        printf("Invalid topic name: too long, try again\n\n");
        return 1;
    } else {
        strcpy(subscribtion.topicname, unchecked_name);
    }
    msgsnd(server_q, &subscribtion, sizeof(struct message) - sizeof(long), 0);
    struct message response;
    msgrcv(client_q, &response, sizeof(struct message) - sizeof(long), 0, 0);
    if(response.mtype == SR_ERR) {
        printf("%s\n", response.error_text);
        return 1;
    }
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
        printf("%s\n", response.error_text);
        return 1;
    }
    return 0;
}

int send_message(int client_q , int server_q) {
    //
}

int block_user(int client_q , int server_q) {
    //
}

int main() {
    int server_q = msgget(SERVER_KEY, IPC_CREAT | 0666); // czy klient powinien tworzyć czy tu ma być błont, czy ma czekać czy co?
    int client_q = msgget(getpid(), IPC_CREAT | 0666);

    while(send_login(client_q, server_q)) {};
    while(create_topic(client_q, server_q)) {};
    while(subscribe(client_q, server_q)) {};
    return 0;
}