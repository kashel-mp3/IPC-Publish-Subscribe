#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct message {
    long mtype;
    char id[20], username[20];
};

int recieve_login(struct message* msg) {
    printf("%s %s\n", msg->id, msg->username);
}

int main() {
    int key = 111;
    int q_id = msgget(key, IPC_CREAT | 0666); 
    while(1) {
        struct message msg;
        msgrcv(q_id, &msg, sizeof(struct message) - sizeof(long), 0, 0);
        switch (msg.mtype) {
        case 1:
            recieve_login(&msg);
            break;
        }
    }
    return 0;
}