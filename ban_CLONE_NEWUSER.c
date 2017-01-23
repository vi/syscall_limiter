#include <linux/sched.h>                // for CLONE_NEWUSER
#include <seccomp.h>                    // for seccomp_rule_add, etc
#include <stdint.h>                     // for uint32_t
#include <stdio.h>                      // for fprintf, perror, stderr
#include <unistd.h>                     // for execve

// gcc ban_CLONE_NEWUSER.c -lseccomp -o ban_CLONE_NEWUSER

int main(int argc, char* argv[], char* envp[]) {
    
    if (argc<2) {
        fprintf(stderr, "Usage: ban_CLONE_NEWUSER program [arguments]\n");
        fprintf(stderr, "Bans unshare(2) with any flags and clone(2) with CLONE_NEWUSER flag\n");
        return 126;
    }
    
    uint32_t default_action = SCMP_ACT_ALLOW;
    
    scmp_filter_ctx ctx = seccomp_init(default_action);
    if (!ctx) {
        perror("seccomp_init");
        return 126;
    }
    
    int ret = 0;
    ret |= seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1),  seccomp_syscall_resolve_name("unshare"), 0);
    ret |= seccomp_rule_add(ctx, SCMP_ACT_ERRNO(1),  seccomp_syscall_resolve_name("clone"), 1, SCMP_CMP(0, SCMP_CMP_MASKED_EQ, CLONE_NEWUSER, CLONE_NEWUSER));
        
    if (ret!=0) {
        fprintf(stderr, "seccomp_rule_add returned %d\n", ret);
        return 124;
    }
    
    ret = seccomp_load(ctx);
    if (ret!=0) {
        fprintf(stderr, "seccomp_load returned %d\n", ret);        
        return 124;
    }
    
    execve(argv[1], argv+1, envp);
    
    perror("execve");
    return 127;
}
