#pragma once

#include "parameters.h"
#include "messageTypes.h"

struct BlockedUser {
    struct Client* user;
    struct BlockedUser* prev;
    struct BlockedUser* next;
};

struct BlockedLinkedList {
    struct BlockedUser* head;
    struct BlockedUser* tail;
};

struct Client {
    int id;
    char username[USERNAME_LEN];
    struct BlockedLinkedList* blocked;
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
struct Sub* find_sub_by_client_id(struct SubsLinkedList* list, int id);
int add_modify_sub(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int cnt, int id);

void delete_sub(struct TopicLinkedList* topic_list, struct Topic* topic, struct Sub* sub);
void free_subs_linked_list(struct SubsLinkedList* list);

struct BlockedUser* create_blocked(struct Client* blocked);
struct BlockedLinkedList* blocked_linked_list();
struct BlockedUser* find_blocked_by_id(struct BlockedLinkedList* list, int id);
void add_blocked(struct BlockedLinkedList* list, struct Client* blocked);
void delete_blocked(struct BlockedLinkedList* list, int id);
void free_blocked_linked_list(struct BlockedLinkedList* list);

struct Topic* create_topic(int id, const char* name, struct TopicLinkedList* topic_list, struct ClientLinkedList* client_list);
struct TopicLinkedList* topic_linked_list();
struct Topic* find_topic_by_name(struct TopicLinkedList* list, const char* name);
struct Topic* find_topic_by_id(struct TopicLinkedList* list, int id);
int add_topic(struct TopicLinkedList* topic_list, const char* name, struct ClientLinkedList* client_list, int client_id); 
void delete_topic(struct Topic* topic, struct TopicLinkedList* list);
void free_topic_linked_list(struct TopicLinkedList* list);