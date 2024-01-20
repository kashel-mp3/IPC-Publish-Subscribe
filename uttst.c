#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define MAX_MESSAGES 10
#define MAX_MESSAGE_LENGTH 100
#define MAX_TOPIC_LENGTH 36

struct messageEntry
{
    char topic[MAX_TOPIC_LENGTH];
    char text[MAX_MESSAGE_LENGTH];
};

struct messageEntry* createMessageEntry(char* topic, char* text){
    struct messageEntry* newMessage = (struct messageEntry*) malloc(sizeof(struct messageEntry));
    strcpy(newMessage->topic, topic);
    strcpy(newMessage->text, text);
    return newMessage;
}

struct messageLogBuffer{
    int writePos;
    struct messageEntry* messages[MAX_MESSAGES];
};

struct messageLogBuffer* createMessageLogBuffer(){
    struct messageLogBuffer* MLB = (struct messageLogBuffer*) malloc(sizeof(struct messageLogBuffer));
    MLB->writePos = 0;
    for(int i = 0; i < MAX_MESSAGES; ++i){
        MLB->messages[i] = NULL;
    }
    return MLB;
}

void addMessageToBuffer(struct messageLogBuffer* MLB, struct messageEntry* entry){
    int writePos = MLB->writePos;
    if(MLB->messages[writePos] != NULL){
        free(MLB->messages[writePos]);
    }
    MLB->messages[writePos] = entry;
    MLB->writePos++;
    MLB->writePos %= MAX_MESSAGES;
}

void deleteMessageLogBuffer(struct messageLogBuffer* MLB){
    for(int i = 0; i < MAX_MESSAGES && MLB->messages[i] != NULL; ++i){
        free(MLB->messages[i]);
    }
    free(MLB);
}

void clearScreen() {
    printf("\033[H\033[J");
}

void moveCursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

int displayMessages(struct messageLogBuffer* MLB) {
    int nMessages = 0;
    int offset = MLB->writePos;
    struct messageEntry* entry;
    for(int i = 0; i < MAX_MESSAGES; ++i){
        entry = MLB->messages[(offset + i) % MAX_MESSAGES];
        if(entry != NULL){
            printf("%s | %s\n", entry->topic, entry->text);
            ++nMessages;
        }
    }
    return nMessages;
}

int isAlnumOnly(char* text, int maxLen){
    for(int i = 0; i < maxLen; ++i){
        if(text[i] == '\0') break;
        if(!isalnum(text[i])) return -1;
    }
    return 0;
}

struct threadData{
    char inputBuffer[MAX_MESSAGE_LENGTH + 1];
    int bufferPos;
    pthread_mutex_t inputLock;
    pthread_mutex_t bufferReadLock;
    int inputReady;
};

struct threadData* createThreadData(){
    struct threadData* newThread = (struct threadData*) malloc(sizeof(struct threadData));
    pthread_mutex_init(&(newThread->inputLock), NULL);
    pthread_mutex_init(&(newThread->bufferReadLock), NULL);
    newThread->bufferPos = 0;
    return newThread;
};

void deleteThreadData(struct threadData* data){
    pthread_mutex_destroy(&data->inputLock);
    pthread_mutex_destroy(&data->bufferReadLock);
}

void* inputThreadFunction(void* _data){
    struct threadData* data = (struct threadData*) _data;
    char inputChar;
    int byteRead;

    while(1){
        byteRead = read(STDIN_FILENO, &inputChar, 1);
        if(byteRead > 0){
            if(inputChar == '\n'){
                if(data->bufferPos > 0){
                    pthread_mutex_lock(&data->inputLock);
                    //printf("input Locked\n");
                    data->inputReady = 1;
                    pthread_mutex_lock(&data->inputLock); // to odblokowuje processing thread po przetworzeniu buffera
                    pthread_mutex_unlock(&data->inputLock);
                    pthread_mutex_lock(&data->bufferReadLock);
                    data->inputBuffer[0] = '\0';
                    data->bufferPos = 0;
                    pthread_mutex_unlock(&data->bufferReadLock);
                }
            }
            else if(inputChar == 127 && data->bufferPos > 0){
                pthread_mutex_lock(&data->bufferReadLock);
                data->bufferPos--;
                data->inputBuffer[data->bufferPos] = '\0';
                pthread_mutex_unlock(&data->bufferReadLock);
            }
            else if(' ' <= inputChar && inputChar <= '~' && data->bufferPos < MAX_MESSAGE_LENGTH - 1){
                pthread_mutex_lock(&data->bufferReadLock);
                data->inputBuffer[data->bufferPos] = inputChar;
                data->bufferPos++;
                data->inputBuffer[data->bufferPos] = '\0';
                pthread_mutex_unlock(&data->bufferReadLock);
            }
        }
        //clearScreen();
        usleep(10000);
    }
}

void setNonBlockingMode() {
    struct termios ttystate;

    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
}

void resetNonBlockingMode(){
    struct termios ttystate;

    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void displayUI(struct messageLogBuffer* MLB, struct threadData* data, char* topic){
    clearScreen();
    printf("Connected to LOCALHOST as GUEST\n");
    int offset = displayMessages(MLB);
    pthread_mutex_lock(&data->bufferReadLock);
    moveCursor(offset + 2, 0);
    printf("%s >> %s", topic, data->inputBuffer);
    fflush(stdin);
    pthread_mutex_unlock(&data->bufferReadLock);
}

int main() {
    char* topic = (char*) malloc(sizeof(char) * MAX_TOPIC_LENGTH);
    struct messageLogBuffer* messageLogBuffer = createMessageLogBuffer();

    struct threadData* data = createThreadData();
    pthread_t inputThreadId;

    setNonBlockingMode();
    setvbuf(stdout, NULL, _IONBF, 0);

    if(pthread_create(&inputThreadId, NULL, inputThreadFunction, (void*) data) != 0){
        perror("Failed to create input thread");
        exit(EXIT_FAILURE);
    }

    strcpy(topic, "RANDOM"); // default topic;

    while (1) {
        displayUI(messageLogBuffer, data, topic);
        
        //TODO incoming messages handling

        if(data->inputReady){
            if (data->inputBuffer[0] == '/') {
                if(strncmp(data->inputBuffer, "/topic ", 7) == 0){
                    int argLen = strlen(data->inputBuffer + 7);
                    if(argLen == 0){
                        //TODO missing argument
                    }
                    else if(argLen > MAX_TOPIC_LENGTH){
                        //TODO topic too long
                    }
                    else if(isAlnumOnly(data->inputBuffer + 7, argLen)){
                        //TODO invalid argument !ALNUM
                    }
                    else{
                        topic = strcpy(topic, data->inputBuffer + 7);
                    }
                    //if(topic[argLen - 1] == '\n') topic[argLen - 1] = '\0';
                }
                else{

                }
            } 
            else {
                addMessageToBuffer(messageLogBuffer, createMessageEntry(topic, data->inputBuffer));
            }
            data->inputReady = 0;
            pthread_mutex_unlock(&data->inputLock);
            //printf("input unlocked\n");
        }
        usleep(100000);
    }

    deleteThreadData(data);
    resetNonBlockingMode();

    return 0;
}