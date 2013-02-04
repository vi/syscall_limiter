#include <stdio.h>
#include <seccomp.h>

// gcc limit_syscalls.c -lseccomp -o limit_syscalls

// https://github.com/vi/syscall_limiter

// Created by Vitaly "_Vi" Shukela; 2013; MIT

int main(int argc, char* argv[], char* envp[]) {
    
    if (argc<3 || !strcmp(argv[1], "-?") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "--version")) {
        fprintf(stderr, "Usage: limit_syscalls syscall1 syscall2 ... syscallN -- program [arguments]\n");
        fprintf(stderr, "Example:\n"
            "   limit_syscalls execve exit write read open close mmap2  fstat64 access  mprotect set_thread_area -- /bin/echo qqq\n");
        return 126;
    }
    
    // ECHRNG, just to provide more or less noticable message when we block a syscall
    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ERRNO(44));
    if (!ctx) {
        perror("seccomp_init");
        return 126;
    }
    
    int i;
    
    for (i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "--")) break;
            
        int syscall = seccomp_syscall_resolve_name(argv[i]);
        
        if (syscall == __NR_SCMP_ERROR) {
            fprintf(stderr, "Failed to resolve syscall %s\n", argv[i]);
            return 125;
        }
        
        int ret = seccomp_rule_add(ctx, SCMP_ACT_ALLOW, syscall, 0);
        
        if (ret!=0) {
            fprintf(stderr, "seccomp_rule_add returned %d\n", ret);
            return 124;
        }
    }
    
    int ret = seccomp_load(ctx);
    if (ret!=0) {
        fprintf(stderr, "seccomp_load returned %d\n", ret);        
    }
    
    execve(argv[i+1], argv+i+1, envp);
    
    perror("execve");    
    return 123;
}