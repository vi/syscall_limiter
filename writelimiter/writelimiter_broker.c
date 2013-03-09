#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "writelimiter.h"

#include "recv_fd.h"
#include "send_fd.h"


int serve(int socket, int syncsocket) {
    for(;;) {
        write(syncsocket, "grant\n", 6); /* to prevent multiple clients messing with each other */
        
        struct request r;
        read(socket, &r, sizeof(r));
        r.pathname[PATH_MAX-1]=0;
        
        errno=0;
        if(strncmp(r.pathname, "/tmp", 4)) errno=EACCES;
        if(strstr(r.pathname, "..")) errno=EACCES;
        
        int ret;
        if (!errno) {
            ret = open(r.pathname, r.flags, r.mode);
        } else {
            ret = -1;
        }
        if(ret!=-1) {
            send_fd(socket, ret);
            close(ret);
        } else {
            write(socket, "X", 1);
            write(socket, &errno, sizeof(errno));
        }
    }
    return 0;
}


int main(int argc, char* argv[], char* envp[]) {
    int sockets[2] = {-1, -1};
    int syncsockets[2] = {-1,-1};
    socketpair(AF_UNIX, SOCK_DGRAM, 0, syncsockets);
    socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets);
    if (sockets[0]==-1) {
        perror("socketpair");
        return 1;
    }
    
    int childpid = fork();
    
    if (!childpid) {
        close(sockets[0]);
        close(syncsockets[0]);
        return serve(sockets[1], syncsockets[1]); 
    }
    close(sockets[1]);
    close(syncsockets[1]);
    dup2(sockets[0], 33);
    dup2(syncsockets[0], 34);
    close(sockets[0]);
    close(syncsockets[0]);
    
    execve(argv[1], argv+1, envp);
    perror("execve");
    return 127;
}