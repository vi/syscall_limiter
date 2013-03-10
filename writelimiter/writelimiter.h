#pragma once

#define PATH_MAX 4096

struct request {
    char operation;
    char pathname[PATH_MAX];
    union {
        struct {
            int flags;
            mode_t mode;
        } open;
    };
};
