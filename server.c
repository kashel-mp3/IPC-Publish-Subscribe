#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_message {
    long mtype;
    
};

int recieve_login() {
    
    
}

int main() {
    int key = 111;
    int queue = msgid = msgget(key, IPC_CREAT | 0666);
    switch (expression) {
    case 1:
        recieve_login();
        break;
    }
    return 0;
}