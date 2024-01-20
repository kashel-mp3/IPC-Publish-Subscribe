#pragma once
#include <pthread.h>

#define MAX_MESSAGES 10
#define MAX_MESSAGE_LENGTH 100
#define MAX_TOPIC_LENGTH 36

struct messageEntry
{
    char topic[MAX_TOPIC_LENGTH];
    char text[MAX_MESSAGE_LENGTH];
};

struct messageEntry* createMessageEntry(char* topic, char* text);
void deleteMessageEntry(struct messageEntry* entry);

struct messageLogBuffer{
    int writePos;
    struct messageEntry* messages[MAX_MESSAGES];
};

struct messageLogBuffer* createMessageLogBuffer();
void deleteMessageLogBuffer(struct messageLogBuffer* MLB);
void addMessageToBuffer(struct messageLogBuffer* MLB, struct messageEntry* entry);

void clearScreen();
void clearLines(int numLines);

void moveCursor(int row, int col);
int displayMessages(struct messageLogBuffer* MLB);
int isAlnumOnly(char* text, int maxLen);

struct threadData{
    char inputBuffer[MAX_MESSAGE_LENGTH + 1];
    int bufferPos;
    pthread_mutex_t inputLock;
    pthread_mutex_t bufferReadLock;
    int inputReady;
};

struct threadData* createThreadData();
void deleteThreadData(struct threadData* data);
void* inputThreadFunction(void* _data);
void setNonBlockingMode();
void resetNonBlockingMode();
void displayUI(struct messageLogBuffer* MLB, struct threadData* data, char* topic);