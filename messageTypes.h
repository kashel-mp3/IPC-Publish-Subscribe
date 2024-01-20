#pragma once

#include "parameters.h"

struct message {
    long mtype;
    int id;
    int cnt;
    char username[USERNAME_LEN];
    char topicname[TOPIC_LEN];
    char topic_id;
    int sub_duration;
    char sub_topic[20];
    char text[100];
    char topic_list[TOPIC_LEN*TOPIC_CNT];
};