#pragma once

#define USERNAME_LEN 14
#define TOPIC_LEN 30
#define TOPIC_CNT 20
#define MAX_CLIENTS 100
#define SERVER_KEY 2137

#define SR_OK 0
#define SR_ERR 1
#define SR_INFO 2
#define SR_TEXTMSG 3

#define CR_TEXTMSG 11
#define CR_LOGIN 12
#define CR_CREAT_TOPIC 13
#define CR_REQ_TOPIC 14
#define CR_ADD_SUB 15
#define CR_UNSUB 16
#define CR_MUTE 17
#define CR_TOPIC 18
#define CR_REQ_ANC 19
#define CR_ADD_ANC 20

#define DEFAULT_TOPIC "RANDOM"
#define UNLIMITED -1

#define MAX_MESSAGES 15 // liczba najnowszych wiadomości wyświetlanych w oknie klienta
#define MAX_MESSAGE_LENGTH 100
#define MAX_TOPIC_LENGTH 36