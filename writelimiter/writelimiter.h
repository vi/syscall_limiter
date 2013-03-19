#pragma once


#define PATH_MAX 4096

struct request {
    char operation;
    int flags;
    mode_t mode;
    dev_t dev;
    char pathname[PATH_MAX];
};
