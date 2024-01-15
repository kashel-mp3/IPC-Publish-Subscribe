#include "parameters.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>

struct message {
    long mtype;
    int id;
    char username[USERNAME_LEN];
    char* error_text;
};

struct Client {
    int id;
    char username[USERNAME_LEN];
    struct Client* prev;
    struct Client* next;
};

struct LinkedList {
    struct Client* head;
    struct Client* tail;
};

struct Client* create_client(int id, const char* username) {
    struct Client* new_client = (struct Client*)malloc(sizeof(struct Client));
    new_client->id = id;
    snprintf(new_client->username, sizeof(new_client->username), "%s", username); //?
    new_client->prev = NULL;
    new_client->next = NULL;
    return new_client;
}

struct LinkedList* create_linked_list() {
    struct LinkedList* new_list = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}

struct Client* find_client_by_username(struct LinkedList* list, const char* username) {
    struct Client* current = list->head;
    
    while(current) {
        if(current->username == username) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

struct Client* find_client_by_id(struct LinkedList* list, int id) {
    struct Client* current = list->head;
    
    while(current) {
        if(current->id == id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

//returns: 1 - client not added / 0 - client added
int add_client(struct LinkedList* list, int id, const char* username) {
    if(find_client_by_username(list, username) != NULL) {
        return 1;
    }
    struct Client* new_client = create_client(id, username);
    if(list->head == NULL) {
        list->head = new_client;
        list->tail = new_client;
    } else {
        new_client->prev = list->tail;
        list->tail->next = new_client;
        list->tail = new_client;
    }
    return 0;
}

void delete_client(struct LinkedList* list, int id) {
    struct Client* client = find_client_by_id(list, id);
    if(client) {
        if(client->next) {
            client->next->prev = client->prev;
        } else {
            client->prev->next = NULL;
            list->tail = client->prev;

        }
        if(client->prev) {
            client->prev->next = client->next;
        } else {
            client->next->prev = NULL;
            list->head = client->next;
        }
        free(client);
    }
}

void print_linked_list(struct LinkedList* list) {
    struct Client* current = list->head;

    while (current != NULL) {
        printf("ID: %d, Username: %s\n", current->id, current->username);
        current = current->next;
    }
}

void free_linked_list(struct LinkedList* list) {
    struct Client* current = list->head;
    struct Client* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}


int main() {
    struct LinkedList* active_clients = create_linked_list();

    int q_id = msgget(SERVER_KEY, IPC_CREAT | 0666); 
    while(1) {
        struct message msg, response;
        msgrcv(q_id, &msg, sizeof(struct message) - sizeof(long), 0, 0);
        switch (msg.mtype) {
        case 1:
            printf("%d, %s\n", msg.id, msg.username);
            if(add_client(active_clients, msg.id, msg.username)) {
                response.mtype = 0;
            } else {
                response.mtype = 2;
                response.error_text = "username taken lul";
            }
            msgsnd(msg.id, &response, sizeof(struct message) - sizeof(long), 0);
            break;
        }
    }
    return 0;
}