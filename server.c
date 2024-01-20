#include "parameters.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>

struct Client {
    int id;
    char username[USERNAME_LEN];
    struct Client* prev;
    struct Client* next;
};

struct ClientLinkedList {
    struct Client* head;
    struct Client* tail;
};

struct Sub {
    int cnt;
    struct Client* client;
    struct Sub* prev;
    struct Sub* next;
};

struct SubsLinkedList {
    struct Sub* head;
    struct Sub* tail;
};

struct Topic {
    int id;
    char name[TOPIC_LEN];
    struct SubsLinkedList* subscribers;
    struct Topic* prev;
    struct Topic* next;
};

struct TopicLinkedList {
    struct Topic* head;
    struct Topic* tail;
};

struct Client* create_client(int id, const char* username);
struct ClientLinkedList* client_linked_list();
struct Client* find_client_by_username(struct ClientLinkedList* list, const char* username);
struct Client* find_client_by_id(struct ClientLinkedList* list, int id);
int add_client(struct ClientLinkedList* list, int id, const char* username);
void delete_client(struct ClientLinkedList* list, int id);
void print_linked_list(struct ClientLinkedList* list);
void free_client_linked_list(struct ClientLinkedList* list);

struct Sub* create_sub(int cnt, int id, struct ClientLinkedList* list);
struct SubsLinkedList* subs_linked_list();
struct Sub* find_sub_by_client_id(struct SubsLinkedList* list, struct ClientLinkedList* client_list, int id);
int add_modify_sub(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int cnt, int id);
void delete_sub(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int id);
void free_subs_linked_list(struct SubsLinkedList* list);

struct Topic* create_topic(int id, const char* name, struct TopicLinkedList* topic_list, struct ClientLinkedList* client_list);
struct TopicLinkedList* topic_linked_list();
struct Topic* find_topic_by_name(struct TopicLinkedList* list, const char* name);
struct Topic* find_topic_by_id(struct TopicLinkedList* list, int id);
int add_topic(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int client_id); 
void delete_topic(struct TopicLinkedList* list, const char* name);
void free_topic_linked_list(struct TopicLinkedList* list);

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

struct Client* create_client(int id, const char* username) {
    struct Client* new_client = (struct Client*)malloc(sizeof(struct Client));
    new_client->id = id;
    snprintf(new_client->username, sizeof(new_client->username), "%s", username); //?
    new_client->prev = NULL;
    new_client->next = NULL;
    return new_client;
}

struct ClientLinkedList* client_linked_list() {
    struct ClientLinkedList* new_list = (struct ClientLinkedList*)malloc(sizeof(struct ClientLinkedList));
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}

struct Client* find_client_by_username(struct ClientLinkedList* list, const char* username) {
    struct Client* current = list->head;
    
    while(current) {
        if(!strcmp(current->username, username)) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

struct Client* find_client_by_id(struct ClientLinkedList* list, int id) {
    struct Client* current = list->head;
    
    while(current) {
        if(current->id == id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

//returns: 1 - client not added username taken/ 0 - client added
int add_client(struct ClientLinkedList* list, int id, const char* username) {
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

void delete_client(struct ClientLinkedList* list, int id) {
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

void print_linked_list(struct ClientLinkedList* list) {
    struct Client* current = list->head;

    while (current != NULL) {
        printf("ID: %d, Username: %s\n", current->id, current->username);
        current = current->next;
    }
}

void free_client_linked_list(struct ClientLinkedList* list) {
    struct Client* current = list->head;
    struct Client* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

struct Sub* create_sub(int cnt, int id, struct ClientLinkedList* list) {
    struct Sub* new_sub = (struct Sub*)malloc(sizeof(struct Sub));
    struct Client* client = find_client_by_id(list, id);
    new_sub->cnt = cnt;
    new_sub->client = client;
    new_sub->prev = NULL;
    new_sub->next = NULL;
    return new_sub;
}

struct SubsLinkedList* subs_linked_list(int id, struct ClientLinkedList* client_list) {
    struct SubsLinkedList* new_list = (struct SubsLinkedList*)malloc(sizeof(struct SubsLinkedList));
    struct Sub* new_sub = create_sub(0, id, client_list);
    new_list->head = new_sub;
    new_list->tail = new_sub;
    return new_list;
}

struct Sub* find_sub_by_client_id(struct SubsLinkedList* list, struct ClientLinkedList* client_list, int id) {
    struct Sub* current = list->head;
    while(current) {
        if(current->client->id == id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int add_modify_sub(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int cnt, int id) {
    struct Topic* topic = find_topic_by_name(topic_list, name);
    struct SubsLinkedList* subs_list =  topic->subscribers;
    struct Sub* new_sub = find_sub_by_client_id(subs_list, client_list, id); // tak mogę? 
    if(new_sub == NULL) {
        new_sub = create_sub(cnt, id, client_list);
        if(new_sub) {
            return 1;
        }
        if(subs_list->head == NULL) {
        subs_list->head = new_sub;
        subs_list->tail = new_sub;
        } else {
            new_sub->prev = subs_list->tail;
            subs_list->tail->next = new_sub;
            subs_list->tail = new_sub;
        }
        return 0;
    }
    new_sub->cnt = cnt;
    return 0;
}

void delete_topic(struct TopicLinkedList* list, const char* name) {
    struct Topic* topic = find_topic_by_name(list, name);
    if(topic) {
        if(topic->next) {
            topic->next->prev = topic->prev;
        } else {
            topic->prev->next = NULL;
            list->tail = topic->prev;

        }
        if(topic->prev) {
            topic->prev->next = topic->next;
        } else {
            topic->next->prev = NULL;
            list->head = topic->next;
        }
        free(topic);
    }
}

void delete_sub(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int id) {
    struct Topic* topic = find_topic_by_name(topic_list, name);
    struct SubsLinkedList* subs_list = topic->subscribers;
    struct Sub* sub = find_sub_by_client_id(subs_list, client_list, id);
    if(sub) {
        if(sub->next) {
            sub->next->prev = sub->prev;
        } else {
            sub->prev->next = NULL;
            subs_list->tail = sub->prev;

        }
        if(sub->prev) {
            sub->prev->next = sub->next;
        } else {
            sub->next->prev = NULL;
            subs_list->head = sub->next;
        }
        free(sub);
    }
    if(topic->subscribers->head == NULL && topic->subscribers->tail == NULL) {
        delete_topic(topic_list, name);
    }
}

void free_subs_linked_list(struct SubsLinkedList* list) {
    struct Sub* current = list->head;
    struct Sub* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

struct Topic* create_topic(int id, const char* name, struct TopicLinkedList* topic_list, struct ClientLinkedList* client_list) {
    struct Topic* new_topic = (struct Topic*)malloc(sizeof(struct Topic));
    struct SubsLinkedList* subs_list = subs_linked_list(id, client_list);
    if(!topic_list->tail) {
        new_topic->id = 0;
    } else {
        new_topic->id = topic_list->tail->id + 1;
    }
    snprintf(new_topic->name, sizeof(new_topic->name), "%s", name); //?
    new_topic->subscribers = subs_list;
    //printf("%s\n", subs_list->head->client->username);
    new_topic->prev = NULL;
    new_topic->next = NULL;
    return new_topic;
}

struct TopicLinkedList* topic_linked_list() {
    struct TopicLinkedList* new_list = (struct TopicLinkedList*)malloc(sizeof(struct TopicLinkedList));
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}

struct Topic* find_topic_by_name(struct TopicLinkedList* list, const char* name) {
    struct Topic* current = list->head;
    
    while(current) {
        if(!strcmp(current->name, name)) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}


struct Topic* find_topic_by_id(struct TopicLinkedList* list, int id) {
    struct Topic* current = list->head;
    
    while(current) {
        if(current->id == id) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}


int add_topic(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int client_id) {
    if(find_topic_by_name(topic_list, name) != NULL) {
        return 1;
    }
    struct Topic* new_topic = create_topic(client_id, name, topic_list, client_list);
    if(topic_list->head == NULL) {
        topic_list->head = new_topic;
        topic_list->tail = new_topic;
    } else {
        new_topic->prev = topic_list->tail;
        topic_list->tail->next = new_topic;
        topic_list->tail = new_topic;
    }
    return 0;
}

void free_topic_linked_list(struct TopicLinkedList* list) {
    struct Topic* current = list->head;
    struct Topic* next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    free(list);
}

char* topics_to_string(struct TopicLinkedList* topics) {
    int total_length = 0;
    struct Topic* current_topic = topics->head;

    while (current_topic != NULL) {
        total_length += snprintf(NULL, 0, "%d. %s\n", current_topic->id, current_topic->name);
        current_topic = current_topic->next;
    }

    char* result = (char*)malloc(total_length + 1);

    char* current_position = result;
    current_topic = topics->head;

    while (current_topic != NULL) {
        current_position += snprintf(current_position, total_length + 1, "%d. %s\n", current_topic->id, current_topic->name);
        current_topic = current_topic->next;
    }

    return result;
}


void printTopicsAndSubscribers(struct TopicLinkedList* topics) {
    struct Topic* currentTopic = topics->head;

    while (currentTopic != NULL) {
        printf("%s:", currentTopic->name);
        struct Sub* currentSubscriber = currentTopic->subscribers->head;

        while (currentSubscriber != NULL) {
            printf(" %s", currentSubscriber->client->username);
            currentSubscriber = currentSubscriber->next;
        }

        printf("\n");

        currentTopic = currentTopic->next;
    }
}

int main() {
    struct ClientLinkedList* active_clients = client_linked_list();
    struct TopicLinkedList* active_topics = topic_linked_list();

    int no_of_topics = 0;

    int server_q = msgget(SERVER_KEY, IPC_CREAT | 0666); 
    while(1) {
        struct message msg, response;
        msgrcv(server_q, &msg, sizeof(struct message) - sizeof(long), 0, 0);
        response.mtype = SR_ERR;
        strcpy(response.error_text, "sth wronk :^(");
        switch (msg.mtype) {
            case CR_LOGIN:
                if(!add_client(active_clients, msg.id, msg.username)) {
                    //printf("%d, %s\n", msg.id, msg.username);
                    response.mtype = SR_OK;
                } else {
                    strcpy(response.error_text, "username taken lul");
                }
                break;
            case CR_CREAT_TOPIC:
                if(!add_topic(active_topics, msg.topicname, active_clients, msg.id)) {
                    no_of_topics++;
                    response.mtype = SR_OK;
                } else {
                    strcpy(response.error_text, "topicname taken lul");
                }
                printTopicsAndSubscribers(active_topics);
                break;
            case CR_REQ_TOPIC:
                response.topic_list[0] = '\0';
                response.cnt = no_of_topics;
                char* topics_string = topics_to_string(active_topics);
                if (topics_string != NULL) {
                    response.mtype = SR_OK;
                    snprintf(response.topic_list, sizeof(response.topic_list), "%s", topics_string);\
                    free(topics_string);
                } else {
                    strcpy(response.error_text, "Memory allocation error for topic_list");
                    response.mtype = SR_ERR;
                }
                break;
            case CR_ADD_SUB:
                struct Topic* topic = find_topic_by_id(active_topics, msg.topic_id);
                if(!add_modify_sub(active_topics, topic->name, active_clients, msg.cnt, msg.id)) {
                    response.mtype = SR_OK;
                } else {
                    strcpy(response.error_text, "nwm co poszło nie tak lul");
                }
                printTopicsAndSubscribers(active_topics);
                break;
        }
        msgsnd(msg.id, &response, sizeof(struct message) - sizeof(long), 0);
    }
    return 0;
}