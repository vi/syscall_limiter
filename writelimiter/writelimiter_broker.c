#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//#include <regex.h>

#include "writelimiter.h"

#include "recv_fd.h"
#include "send_fd.h"
#include "popen2.h"

int safer_write(int f, char* b, int n);
int safer_read(int f, char* b, int n);


int serve(int socket, int syncsocket, int policy_in, int policy_out) {
    char additional_path[PATH_MAX];
    memset(additional_path, 0, PATH_MAX);
    for(;;) {
        write(syncsocket, "grant\n", 6); /* to prevent multiple clients messing with each other */
        
        struct request r;
        read(socket, &r, sizeof(r));
        r.pathname[PATH_MAX-1]=0;
        
        errno=0;
        
        if(strchr(r.pathname, '\n')) errno=EACCES;
            
        if (!errno) {
            safer_write(policy_in, r.pathname, strlen(r.pathname));
            safer_write(policy_in, "\n", 1);
            char reply;
            safer_read(policy_out, &reply, 1);
            if (reply!='1') {
                errno=EACCES;
            }
        }
        
        if (errno) {
            write(socket, &errno, sizeof(errno));
            continue;
        }
        
        errno=0;
        int ret;
        switch (r.operation) {
        case 'o': {        
            ret = open(r.pathname, r.flags, r.mode);
        } break;
        case 'u': {
            unlink(r.pathname);
        } break;
        case 'r': {
            rmdir(r.pathname);
        } break;
        case 'A': {
            /* Additinal path for moving and linking */
            memcpy(additional_path, r.pathname, PATH_MAX);
        } break;
        case 'l': {
            if(!additional_path[0]) errno=EINVAL;
            else {
                link(additional_path, r.pathname);
            }    
        } break;
        case 's': {
            if(!additional_path[0]) errno=EINVAL;
            else {
                symlink(additional_path, r.pathname);
            }    
        } break;
        case 'n': {
            if(!additional_path[0]) errno=EINVAL;
            else {
                rename(additional_path, r.pathname);
            }  
        } break;
        case 'm': {
            mkdir(r.pathname, r.mode);
        } break;
        case 'c': {
            chmod(r.pathname, r.mode);
        } break;
        default: {
            errno = ENOSYS;      
        }
        } // switch
    
        write(socket, &errno, sizeof(errno));    
        
        if(r.operation == 'o' && !errno) {
            send_fd(socket, ret);
            close(ret);
        }
    }

    return 0;
}


int main(int argc, char* argv[], char* envp[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: writelimiter_broker policy_commandline ...(argv)\n");
        return 1;
    }
    
    int sockets[2] = {-1, -1};
    int syncsockets[2] = {-1,-1};
    socketpair(AF_UNIX, SOCK_DGRAM, 0, syncsockets);
    socketpair(AF_UNIX, SOCK_DGRAM, 0, sockets);
    if (sockets[0]==-1) {
        perror("socketpair");
        return 1;
    }
    
    struct popen2 policyprog;
    int ret = popen2(argv[1], &policyprog);
    if (ret) {
        perror("popen2");
        return 1;
    }    
    
    int childpid = fork();
    
    if (!childpid) {
        close(sockets[0]);
        close(syncsockets[0]);
        return serve(sockets[1], syncsockets[1], policyprog.to_child, policyprog.from_child); 
    }
    close(sockets[1]);
    close(syncsockets[1]);
    dup2(sockets[0], 33);
    dup2(syncsockets[0], 34);
    close(sockets[0]);
    close(syncsockets[0]);
    close(policyprog.from_child);
    close(policyprog.to_child);
    
    execve(argv[2], argv+2, envp);
    perror("execve");
    return 127;
}