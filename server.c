#include "parameters.h"
#include "server.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <string.h>
#include <sys/msg.h>

struct Client* create_client(int id, const char* username) {
    struct Client* new_client = (struct Client*)malloc(sizeof(struct Client));
    new_client->id = id;
    new_client->blocked = blocked_linked_list();
    snprintf(new_client->username, sizeof(new_client->username), "%s", username);
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
    
    while(current != NULL) {
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
        free_blocked_linked_list(current->blocked);
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
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}

struct Sub* find_sub_by_client_id(struct SubsLinkedList* list, int id) {
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
    struct Sub* new_sub = find_sub_by_client_id(subs_list, id); 
    if(new_sub == NULL) {
        new_sub = create_sub(cnt, id, client_list);
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

void delete_topic(struct Topic* topic, struct TopicLinkedList* list) {
    if(topic) {
        if(topic->next != NULL) {
            topic->next->prev = topic->prev;
        } else {
            topic->prev->next = NULL;
            list->tail = topic->prev;

        }
        if(topic->prev != NULL) {
            topic->prev->next = topic->next;
        } else {
            topic->next->prev = NULL;
            list->head = topic->next;
        }
        free(topic);
    }
}

void delete_sub(struct TopicLinkedList* topic_list, struct Topic* topic, struct Sub* sub) {
    if(sub) {
        if(sub->next != NULL) {
            sub->next->prev = sub->prev;
        } else {
            sub->prev->next = NULL;
            topic->subscribers->tail = sub->prev;

        }
        if(sub->prev != NULL) {
            sub->prev->next = sub->next;
        } else {
            sub->next->prev = NULL;
            topic->subscribers->head = sub->next;
        }
        free(sub);
    }
    if(topic->subscribers->head == NULL && topic->subscribers->tail == NULL) {
        delete_topic(topic, topic_list);
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

struct BlockedUser* create_blocked(struct Client* blocked) {
    struct BlockedUser* new_blocked =  (struct BlockedUser*)malloc(sizeof(struct BlockedUser));
    new_blocked->user = blocked;
    new_blocked->prev = NULL;
    new_blocked->next = NULL;
    return new_blocked;
}
struct BlockedLinkedList* blocked_linked_list() {
    struct BlockedLinkedList* new_list = (struct BlockedLinkedList*)malloc(sizeof(struct BlockedLinkedList));
    new_list->head = NULL;
    new_list->tail = NULL;
    return new_list;
}
void free_blocked_linked_list(struct BlockedLinkedList* list) {
    struct BlockedUser* current = list->head;
    struct BlockedUser* next;
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    free(list);
}
struct BlockedUser* find_blocked_by_id(struct BlockedLinkedList* list, int id) {
    struct BlockedUser* current = list->head;
    while(current) {
        if(current->user->id == id) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void add_blocked(struct BlockedLinkedList* list, struct Client* blocked) {
    struct BlockedUser* new_blocked = create_blocked(blocked);
    if(list->head == NULL) {
        list->head = new_blocked;
        list->tail = new_blocked;
    } else {
        new_blocked->prev = list->tail;
        list->tail->next = new_blocked;
        list->tail = new_blocked;
    }
}

void delete_blocked(struct BlockedLinkedList* list, int id) {
    struct BlockedUser* blocked = find_blocked_by_id(list, id);
    if(blocked) {
        if(blocked->next) {
            blocked->next->prev = blocked->prev;
        } else {
            blocked->prev->next = NULL;
            list->tail = blocked->prev;
        }
        if(blocked->prev) {
            blocked->prev->next = blocked->next;
        } else {
            blocked->next->prev = NULL;
            list->head = blocked->next;
        }
        free(blocked);
    }
}

void displayBlockedList(struct Client* client){
    struct BlockedUser* curr = client->blocked->head;
    printf("Blocked user list of %d %s: ", client->id, client->username);
    while(curr != NULL){
        printf("%d %s, ", curr->user->id, curr->user->username);
        curr = curr->next;
    }
    printf("\n");
}

struct Topic* create_topic(int id, const char* name, struct TopicLinkedList* topic_list, struct ClientLinkedList* client_list) {
    struct Topic* new_topic = (struct Topic*)malloc(sizeof(struct Topic));
    struct SubsLinkedList* subs_list = subs_linked_list(id, client_list);
    strcpy(new_topic->name, name);
    new_topic->subscribers = subs_list;
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
    if(client_id != -1) { //zakladam -1 = server
        add_modify_sub(topic_list, name, client_list, UNLIMITED, client_id);
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
        total_length += snprintf(NULL, 0, "%s%s", current_topic->name, (current_topic->next != NULL) ? ", " : "");
        current_topic = current_topic->next;
    }
    char* result = (char*)malloc(total_length + 1);
    char* current_position = result;
    current_topic = topics->head;

    while (current_topic != NULL) {
        current_position += snprintf(current_position, total_length + 1, "%s%s", current_topic->name, (current_topic->next != NULL) ? ", " : "");
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

int send_info_about_new_topic(struct ClientLinkedList* client_list, const char* topicname, int creator_id) {
    struct Client* current = client_list->head;
    if(!current) {
        return 0;
    }

    struct message author_feedback, client_info;
    strcpy(author_feedback.username, "SERVER");
    strcpy(author_feedback.topicname, "INFO");
    author_feedback.mtype = SR_INFO;
    strcpy(author_feedback.text, "your topic has been sucessfully addeed!!! ;^D");
    strcpy(client_info.username, "SERVER");
    strcpy(client_info.topicname, "INFO");
    client_info.mtype = SR_INFO;
    strcpy(client_info.text, "somebody just created a new topic (");
    strcat(client_info.text, topicname);
    strcat(client_info.text, ")");
    while(current) {
        if(current->id == creator_id) {
            msgsnd(current->id, &author_feedback, sizeof(struct message) - sizeof(long), 0);
        } else {
            msgsnd(current->id, &client_info, sizeof(struct message) - sizeof(long), 0);
        }
        current = current->next;
    }
    return 0;
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
        strcpy(response.text, "sth wronk :^(");
        switch (msg.mtype) {
            case CR_LOGIN:
                response.mtype = CR_LOGIN;
                if(find_client_by_username(active_clients, msg.username) != NULL) {
                    strcpy(response.text, "username taken lul");
                    printf("DBG | Client rejected: duplicate username. %s\n", msg.username);
                    response.id = SR_ERR;
                } else {
                    if(find_topic_by_name(active_topics, DEFAULT_TOPIC) == NULL) {
                        add_topic(active_topics, DEFAULT_TOPIC, active_clients, -1);
                        printTopicsAndSubscribers(active_topics);
                    }
                    add_client(active_clients, msg.id, msg.username);
                    add_modify_sub(active_topics, DEFAULT_TOPIC, active_clients, -1, msg.id);
                    printTopicsAndSubscribers(active_topics);
                    response.id = SR_OK;
                    printf("DBG | Client added: %s\n", msg.username);
                }
                break;

            case CR_CREAT_TOPIC:
                response.mtype = CR_CREAT_TOPIC;
                if(find_topic_by_name(active_topics, msg.topicname) != NULL) {
                    strcpy(response.text, "topicname taken lul");
                    printf("DBG | Topic request rejected: duplicate topic. %s\n", msg.topicname);
                } else {
                    add_topic(active_topics, msg.topicname, active_clients, msg.id);
                    no_of_topics++;
                    response.id = SR_OK;
                    send_info_about_new_topic(active_clients, msg.topicname, msg.id);
                    printf("DBG | Topic request accepted: topic created %s\n", msg.topicname);
                }
                break;

            case CR_REQ_TOPIC:
                response.mtype = CR_REQ_TOPIC;
                response.topic_list[0] = '\0';
                response.cnt = no_of_topics;
                char* topics_string = topics_to_string(active_topics);
                if (topics_string != NULL) {
                    response.id = SR_OK;
                    snprintf(response.topic_list, sizeof(response.topic_list), "%s", topics_string);
                    free(topics_string);
                    printf("DBG | Topic list successfully created for %s", msg.username);
                } else {
                    strcpy(response.text, "Memory allocation error for topic_list");
                    printf("DBG | Memory allocation error for topic_list\n");
                }
                break;

            case CR_ADD_SUB:
                response.mtype = CR_ADD_SUB;
                printf("SUB %s %d\n", msg.topicname, msg.sub_duration);
                struct Topic* topic = find_topic_by_name(active_topics, msg.topicname);
                if(topic == NULL) {
                    response.id = SR_ERR;
                    strcpy(response.text, "no such topic");
                } else {
                    add_modify_sub(active_topics, msg.topicname, active_clients, msg.sub_duration, msg.id);
                    response.id = SR_OK; 
                    printTopicsAndSubscribers(active_topics);
                }
                break;

            case CR_UNSUB:
                response.mtype = CR_UNSUB;
                
                struct Topic* topic2 = find_topic_by_name(active_topics, msg.topicname);
                if(topic2 == NULL) {
                    response.id = SR_ERR;
                    strcpy(response.text, "this topic does not exist");
                    break;
                } 
                struct Sub* sub2 = find_sub_by_client_id(topic2->subscribers, msg.id);
                if(sub2 == NULL) {
                    response.id = SR_ERR;
                    strcpy(response.text, "you werent subscribed in the first place");
                    break;
                }
                struct Client* clientData = sub2->client;
                delete_sub(active_topics, topic2, sub2);
                printf("DBG | %s successfully unsubscribed from %s\n", clientData->username, msg.topicname);
                response.id = SR_OK;
                break;

            case CR_MUTE:
                response.mtype = CR_MUTE;
                struct Client* client = find_client_by_id(active_clients, msg.id);
                struct Client* to_mute = find_client_by_username(active_clients, msg.username);
                if(to_mute == NULL) {
                    strcpy(response.text, "no such user");
                } else {
                    add_blocked(client->blocked, to_mute);
                    displayBlockedList(client);
                    response.id = SR_OK;
                }
                break;

            case CR_TEXTMSG:
                struct Topic* topic1 = find_topic_by_name(active_topics, msg.topicname);
                struct Sub* sub = topic1->subscribers->head;
                strcpy(response.topicname, msg.topicname);
                strcpy(response.username, msg.username);
                strcpy(response.text, msg.text);
                response.mtype = SR_TEXTMSG;
                response.id = SR_OK;
                struct message sub_ended;
                sub_ended.mtype = SR_INFO;
                sub_ended.id = SR_OK;
                strcpy(sub_ended.username, "SERVER");
                strcpy(sub_ended.topicname, "INFO");
                strcpy(sub_ended.text, "Your subscription to topic ");
                strcat(sub_ended.text, msg.topicname);
                strcat(sub_ended.text, " ended. You will not recieve further messeges from this topic.");
                while(sub != NULL){
                    if(find_blocked_by_id(sub->client->blocked, msg.id) == NULL){ // można zmienić na find blocked by username, żeby jak ktoś przeloguje to dalej był blokowany
                        msgsnd(sub->client->id, &response, sizeof(struct message) - sizeof(long), 0);
                        if(sub->client->id != msg.id && sub->cnt > 0) {
                            sub->cnt--;
                            printf("DBG: user %s has %d msg left from topic %s\n",sub->client->username, sub->cnt, topic1->name);
                            if(sub->cnt == 0) {
                                msgsnd(sub->client->id, &sub_ended, sizeof(struct message) - sizeof(long), 0);
                                delete_sub(active_topics, topic1, sub);
                                printTopicsAndSubscribers(active_topics);
                            }
                        }
                    }
                    sub = sub->next;
                } // zakładamy, że osoba pisząca wiadomość odbiera ją w sposób synchroniczny (czeka na odpowiedź od serwera zanim dalej klient działa) oraz typ tej wiadomości jest taki sam jak zapytania CR_TEXTMSG, podczas gdy wiadomości od innych użytkowników są wysyłane z typem SR_TEXTMSG i klienci odbierają je w sposób asynchroniczny
                continue;
                break;

            case CR_TOPIC:
                if(find_topic_by_name(active_topics, msg.topicname) != NULL){
                    response.id = SR_OK;
                }
                else{
                    response.id = SR_ERR;
                }
                break;
        }
        msgsnd(msg.id, &response, sizeof(struct message) - sizeof(long), 0);
    }

    free_client_linked_list(active_clients);
    free_topic_linked_list(active_topics);
    return 0;
}