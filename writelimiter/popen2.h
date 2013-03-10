#pragma once

struct popen2 {
    pid_t child_pid;
    int   from_child, to_child;
};

int popen2(const char *cmdline, struct popen2 *childinfo);