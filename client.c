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
    char username[USERNAME_LEN];
    char* error_text;
};

int send_login(int server_q) {
    struct message login;
    login.mtype = 1;
    login.id = getpid();
    bool done = 0;
    printf("Provide username:\n");
    char* unchecked_username = (char*)malloc(50); //free?
    scanf("%50s", unchecked_username);
    printf("6");
    
    if(strlen(unchecked_username) > USERNAME_LEN) {
        printf("Invalid login: too long, try again\n\n");
        printf("0");
        return 0;
    } else {
        strcpy(login.username, unchecked_username);
    }
    msgsnd(server_q, &login, sizeof(struct message) - sizeof(long), 0);
    printf("5");
    struct message response;
    msgrcv(server_q, &response, sizeof(struct message) - sizeof(long), 0, 0);
    if(response.mtype == 2) {
        printf("%s\n", response.error_text);
        printf("1");
        return 0;
    }
    printf("3");
    return login.id;
}

int main() {
    int server_q = msgget(SERVER_KEY, IPC_CREAT | 0666); // czy klient powinien tworzyć czy tu ma być błont, czy ma czekać czy co?
    int client_q = 0;
    while(!client_q) {
        client_q = send_login(server_q);
    }
    return 0;
}