// http://media.unpythonic.net/emergent-files/01108826729/popen2.c

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "popen2.h"

int popen2(const char *cmdline, struct popen2 *childinfo) {
    pid_t p;
    int pipe_stdin[2], pipe_stdout[2];

    if(pipe(pipe_stdin)) return -1;
    if(pipe(pipe_stdout)) return -1;

    //printf("pipe_stdin[0] = %d, pipe_stdin[1] = %d\n", pipe_stdin[0], pipe_stdin[1]);
    //printf("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

    p = fork();
    if(p < 0) return p; /* Fork failed */
    if(p == 0) { /* child */
        close(pipe_stdin[1]);
        dup2(pipe_stdin[0], 0);
        close(pipe_stdout[0]);
        dup2(pipe_stdout[1], 1);
        execl("/bin/sh", "sh", "-c", cmdline, NULL);
        perror("execl"); exit(99);
    }
    childinfo->child_pid = p;
    childinfo->to_child = pipe_stdin[1];
    childinfo->from_child = pipe_stdout[0];
    close(pipe_stdin[0]);
    close(pipe_stdout[1]);
    return 0; 
}

//#define TESTING
#ifdef TESTING
int main(void) {
    char buf[1000];
    struct popen2 kid;
    popen2("tr a-z A-Z", &kid);
    write(kid.to_child, "testing\n", 8);
    close(kid.to_child);
    memset(buf, 0, 1000);
    read(kid.from_child, buf, 1000);
    printf("kill(%d, 0) -> %d\n", kid.child_pid, kill(kid.child_pid, 0)); 
    printf("from child: %s", buf); 
    printf("waitpid() -> %d\n", waitpid(kid.child_pid, NULL, 0));
    printf("kill(%d, 0) -> %d\n", kid.child_pid, kill(kid.child_pid, 0)); 
    return 0;
}
#endif
