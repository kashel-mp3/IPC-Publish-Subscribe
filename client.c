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
    char topic_id;
    int sub_duration;
    char sub_topic[20];
    char error_text[100];
    char topic_list[TOPIC_LEN*TOPIC_CNT];
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
        printf("%s\n", response.error_text);
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