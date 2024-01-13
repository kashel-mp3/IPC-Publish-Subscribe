#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define ID_LEN 3 //maximum id_len = 10
#define USERNAME_LEN 10 // maximum username_len = 50

struct login_msg {
    long mtype;
    int id;
    char username[USERNAME_LEN + 1];
};

int valid_id(char id_s[ID_LEN + 1]) {
    int id = 0;
    for(int i = 0; i < ID_LEN; i++) {
        if(!isdigit(id_s[i])) {
            printf("Invalid ID, try again\n %c \n", id_s[i]);
            return 0;
        }
        id *= 10;
        id += id_s[i] - '0';
    }
    printf("%d\n", id);
    return id;
}

int send_login(int q_name) {
    struct login_msg login;
    login.mtype = 1;
    char id_s[ID_LEN + 1];
    bool done = 0;
    int c;
    while(!done) {
        id_s[ID_LEN] = ' ';
        printf("Provide ID (%d digits):\n", ID_LEN);
        scanf("%4s", id_s);
        while ((c = getchar()) != '\n');
        if(strlen(id_s) < ID_LEN) {
            printf("Invalid ID: too short, try again\n");
            continue;
        }
        if(strlen(id_s) > ID_LEN) {
            printf("Invalid ID: too long, try again\n");
            continue;
        }
        login.id = valid_id(id_s);
        if(!login.id) {
            continue;
        }
        done = 1;
    }
    printf("Provide username:\n");
    scanf("%20s", (login.username));
    while ((c = getchar()) != '\n');
    if(strlen(login.username) > USERNAME_LEN) {
        printf("Invalid login: too long, try again\n");
        return 1;
    }
    msgsnd(q_name, &login, sizeof(struct login_msg) - sizeof(long), 0);
    return login.id;
}

int main() {
    int server_q = 0;
    int own_q = send_login(server_q);

    return 0;
}