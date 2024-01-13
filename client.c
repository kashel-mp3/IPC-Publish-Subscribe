#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct login_msg {
    long mtype;
    char id[20], username[20];
};

int send_login(int q_name) {
    struct login_msg login;
    login.mtype = 1;
    printf("Provide ID:\n");
    scanf("%s", (login.id));
    printf("Provide username:\n");
    scanf("%s", (login.username));
    msgsnd(q_name, &login, sizeof(struct login_msg) - sizeof(long), 0);
}

int main() {
    int server_q = 0;
    send_login(server_q);

    return 0;
}