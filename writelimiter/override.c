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
    
    if (absolutize_path(&r, pathname) == -1) return -1;         
    
    return perform_request(&r);
}

static int remote_open64(const char *pathname, int flags, mode_t mode) {
    return remote_open(pathname, flags, mode); }
static int remote_creat(const char *pathname, mode_t mode) { 
    return remote_open(pathname, O_CREAT|O_WRONLY|O_TRUNC, mode); }
static int remote_creat64(const char *pathname, mode_t mode) { 
    return remote_open(pathname, O_CREAT|O_WRONLY|O_TRUNC, mode); }

static int remote_mkdir(const char *pathname, mode_t mode) { 
    receive_ticket();

    struct request r;
    r.operation = 'm';
    r.mode = mode;
    
    if (absolutize_path(&r, pathname) == -1) return -1;  
    
    return perform_request(&r);
}

static int remote_rmdir(const char *pathname) { 
    receive_ticket();

    struct request r;
    r.operation = 'r';
    
    if (absolutize_path(&r, pathname) == -1) return -1;  
    
    return perform_request(&r);
}



#define OVERIDE_TEMPLATE(name, signature, sigargs) \
    static int (*orig_##name) signature = NULL; \
    int name signature { \
        if(!orig_##name) { \
            orig_##name = dlsym(RTLD_NEXT, "open"); \
        } \
        \
        int ret = (*orig_##name) sigargs; \
        if (ret!=-1) return ret; \
        return remote_##name sigargs; \
    }


OVERIDE_TEMPLATE(open, (const char *pathname, int flags, mode_t mode), (pathname, flags, mode))
OVERIDE_TEMPLATE(open64, (const char *pathname, int flags, mode_t mode), (pathname, flags, mode))
OVERIDE_TEMPLATE(creat, (const char *pathname,  mode_t mode), (pathname, mode))
OVERIDE_TEMPLATE(creat64, (const char *pathname, mode_t mode), (pathname, mode))
OVERIDE_TEMPLATE(mkdir, (const char *pathname,  mode_t mode), (pathname, mode))
OVERIDE_TEMPLATE(rmdir, (const char *pathname), (pathname))
