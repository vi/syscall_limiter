#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>


/* I want only O_CREAT|O_WRONLY|O_TRUNC, not queer open's or creat's signatures */
//#include <fcntl.h>
#define _FCNTL_H
#include <bits/fcntl.h>


#include "writelimiter.h"

#include "recv_fd.h"
#include "send_fd.h"

#define SOCKET 33
#define TICKET_SOCKET 34

static int absolutize_path(struct request *r, const char* pathname) {
    if(pathname[0]!='/') {
        // relative path
        char* ret = getcwd(r->pathname, sizeof(r->pathname));
        if (!ret) {
            return -1;
        }
        int l = strlen(r->pathname);
        if (l==PATH_MAX) {
            errno=ERANGE;
            return -1;
        }
        r->pathname[l]='/';
        strncpy(r->pathname+l+1, pathname, PATH_MAX-l-1);
    } else {
        // absolute path
        strncpy(r->pathname, pathname, PATH_MAX);
    }
    return 0;
}

static int receive_ticket() {
    char buf[6];
    return read(TICKET_SOCKET, buf, 6);
}

static int perform_request(const struct request* r) {
    write(SOCKET, r, sizeof(*r));
    int ret = read(SOCKET, &errno, sizeof(errno));
    if(ret==-1)return -1;
    if(errno) {
        return -1;
    } else {
        int ret = 0;
        if (r->operation=='o') {
            ret = recv_fd(SOCKET);
        }
        return ret;
    }
}

static int remote_open(const char *pathname, int flags, mode_t mode) {
    receive_ticket();

    struct request r;
    r.operation = 'o';
    r.flags = flags;
    r.mode = mode;
    
    int ret = absolutize_path(&r, pathname);    
    if (ret==-1) return -1;
        
    ret=-1;
    
    return perform_request(&r);
}


static int (*orig_open)(const char *pathname, int flags, mode_t mode) = NULL;
int open(const char *pathname, int flags, mode_t mode) {
    if(!orig_open) {
        orig_open = dlsym(RTLD_NEXT, "open");
    }
    
    int ret = (*orig_open)(pathname, flags, mode);
    if (ret==-1) {
        return remote_open(pathname, flags, mode);
    }
    return ret;
}

static int (*orig_open64)(const char *pathname, int flags, mode_t mode) = NULL;
int open64(const char *pathname, int flags, mode_t mode) {
    if(!orig_open64) {
        orig_open64 = dlsym(RTLD_NEXT, "open64");
    }
    
    int ret = (*orig_open64)(pathname, flags, mode);
    if (ret==-1) {
        return remote_open(pathname, flags, mode);
    }
    return ret;
}

static int (*orig_creat)(const char *pathname, mode_t mode) = NULL;
int creat(const char *pathname, mode_t mode) {
    if(!orig_creat) {
        orig_creat = dlsym(RTLD_NEXT, "creat");
    }
    
    int ret = (*orig_creat)(pathname, mode);
    if (ret==-1) {
        return remote_open(pathname, O_CREAT|O_WRONLY|O_TRUNC, mode);
    }
    return ret;    
}

static int (*orig_creat64)(const char *pathname, mode_t mode) = NULL;
int creat64(const char *pathname, mode_t mode) {
    if(!orig_creat64) {
        orig_creat64 = dlsym(RTLD_NEXT, "creat64");
    }
    
    int ret = (*orig_creat64)(pathname, mode);
    if (ret==-1) {
        return remote_open(pathname, O_CREAT|O_WRONLY|O_TRUNC, mode);
    }
    return ret;    
}
