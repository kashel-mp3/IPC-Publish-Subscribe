#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>

#define MAX_CLIENTS 100

struct message {
    long mtype;
    int id;
    char username[50];
};

struct Client {
    int id;
    char username[50];
};

int calculate_index(int clientID) {
    return clientID % MAX_CLIENTS;
}

void print_client(struct Client* clients, int id) {
    int index = calculate_index(id);
    printf("ID: %d, Username: %s\n", clients[index].id, clients[index].username);
}

void print_clients(struct Client* clients) {
    // Iterate through the user array and print each user
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].id != 0) {  // Check if the slot is occupied
            printf("ID: %d, Username: %s\n", clients[i].id, clients[i].username);
        }
    }
}

void add_client(struct Client* clients, int id, const char* username) {
    int index = calculate_index(id);
    clients[index].id = id;
    strncpy(clients[index].username, username, sizeof(clients[index].username));
}

int main() {
    struct Client clients[MAX_CLIENTS];

    int key = 111;
    int q_id = msgget(key, IPC_CREAT | 0666); 
    while(1) {
        struct message msg;
        msgrcv(q_id, &msg, sizeof(struct message) - sizeof(long), 0, 0);
        switch (msg.mtype) {
        case 1:
            add_client(clients, msg.id, msg.username);
            print_client(clients, msg.id);
            break;
        }
    }
    return 0;
}