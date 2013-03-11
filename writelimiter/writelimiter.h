#pragma once


#define PATH_MAX 4096

struct request {
    char operation;
    int flags;
    mode_t mode;
    char pathname[PATH_MAX];
};
