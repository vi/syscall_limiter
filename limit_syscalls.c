#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <seccomp.h>

// gcc limit_syscalls.c -lseccomp -o limit_syscalls

// https://github.com/vi/syscall_limiter

// Created by Vitaly "_Vi" Shukela; 2013; License=MIT


/* Workaround missing va_list version of seccomp_rule_add */
static int seccomp_rule_add_hack(scmp_filter_ctx ctx, uint32_t action, 
    int syscall, unsigned int arg_cnt, struct scmp_arg_cmp *args) {
        if(arg_cnt==0) return seccomp_rule_add(ctx, action, syscall, arg_cnt);
        if(arg_cnt==1) return seccomp_rule_add(ctx, action, syscall, arg_cnt,
            args[0]);
        if(arg_cnt==2) return seccomp_rule_add(ctx, action, syscall, arg_cnt,
            args[0], args[1]);
        if(arg_cnt==3) return seccomp_rule_add(ctx, action, syscall, arg_cnt,
            args[0], args[1], args[2]);
        if(arg_cnt==4) return seccomp_rule_add(ctx, action, syscall, arg_cnt,
            args[0], args[1], args[2], args[3]);
        if(arg_cnt==5) return seccomp_rule_add(ctx, action, syscall, arg_cnt,
            args[0], args[1], args[2], args[3], args[4]);
        if(arg_cnt==6) return seccomp_rule_add(ctx, action, syscall, arg_cnt,
            args[0], args[1], args[2], args[3], args[4], args[5]);
    return -1;
}

int main(int argc, char* argv[], char* envp[]) {
    
    if (argc<3 || !strcmp(argv[1], "-?") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "--version")) {
        fprintf(stderr, "Usage: limit_syscalls syscall1 syscall2 ... syscallN -- program [arguments]\n");
        fprintf(stderr, "Example:\n"
            "   limit_syscalls execve exit write read open close mmap2  fstat64 access  mprotect set_thread_area -- /bin/echo qqq\n");
        fprintf(stderr, "Advanced:\n");
        fprintf(stderr, "   LIMIT_SYSCALLS_DEFAULT_ACTION={a,k,eN} - by default allow, kill or return error N\n");
        fprintf(stderr, "   some_syscall,A0>=3,A4<<1000,A1!=4,A2&&0x0F==0x0F - compare arguments\n");
        fprintf(stderr, "   some_syscall,{a,k,eN} - allow, kill or return error N for this rule\n");
        fprintf(stderr, "Some more example:\n");
        fprintf(stderr,"    LIMIT_SYSCALLS_DEFAULT_ACTION=a  limit_syscalls  'write,A0==1,e0' -- /usr/bin/printf --help\n");
        fprintf(stderr,"     (this makes write to stdout in /usr/bin/printf silently fail, looping it)\n");
        fprintf(stderr, "Restrict user namespace:\n");
        fprintf(stderr, "   LIMIT_SYSCALLS_DEFAULT_ACTION=a limit_syscalls clone,A0\\&\\&0x10000000==0x10000000,e1 unshare,e1  -- /bin/bash\n");
        return 126;
    }
    
    // ECHRNG, just to provide more or less noticable message when we block a syscall
    uint32_t default_action = SCMP_ACT_ERRNO(44);
    
    if (getenv("LIMIT_SYSCALLS_DEFAULT_ACTION")) {
        const char* e = getenv("LIMIT_SYSCALLS_DEFAULT_ACTION");
        if(!strcmp(e, "a")) default_action=SCMP_ACT_ALLOW;
        else 
        if(!strcmp(e, "k")) default_action=SCMP_ACT_KILL;
        else
        if(e[0] == 'e') {
            int errno_;
            if (sscanf(e+1,"%i", &errno_)!=1) {
                fprintf(stderr, "LIMIT_SYSCALLS_DEFAULT_ACTION=e<number> expected\n");
                return 109;
            }
            default_action=SCMP_ACT_ERRNO(errno_);
        }
        else
        {
            fprintf(stderr, "LIMIT_SYSCALLS_DEFAULT_ACTION should be a, k or e<number>\n");
            return 110;
        } 
    }
    
    scmp_filter_ctx ctx = seccomp_init(default_action);
    if (!ctx) {
        perror("seccomp_init");
        return 126;
    }
    
    int i;
    
    for (i=1; i<argc; ++i) {
        if (!strcmp(argv[i], "--")) break;
            
        const char* syscall_name = strtok(argv[i], ",");
                
        int syscall = seccomp_syscall_resolve_name(argv[i]);
        
        if (syscall == __NR_SCMP_ERROR) {
            fprintf(stderr, "Failed to resolve syscall %s\n", syscall_name);
            return 125;
        }
        
        int nargs = 0;
        struct scmp_arg_cmp args[6];
        
        uint32_t action = SCMP_ACT_ALLOW;
        
        const char* aa = strtok(NULL, ",");
        for (;aa; aa=strtok(NULL, ",")) {
            if (aa[0]=='A') {
                if(nargs==6) {
                    fprintf(stderr, "Maximum comparator count (6) exceed in %s\n", argv[i]);
                    return 103;
                }
                if( !(aa[1]>='0' && aa[1]<='5') ) {
                    fprintf(stderr, "A[0-5] expected in %s\n", argv[i]);
                    return 100;
                }
                int cmp = 0; /* invalid value */
                
                if(!strncmp(aa+2, "!=", 2)) cmp=SCMP_CMP_NE;
                if(!strncmp(aa+2, "<<", 2)) cmp=SCMP_CMP_LT;
                if(!strncmp(aa+2, "<=", 2)) cmp=SCMP_CMP_LE;
                if(!strncmp(aa+2, "==", 2)) cmp=SCMP_CMP_EQ;
                if(!strncmp(aa+2, ">=", 2)) cmp=SCMP_CMP_GE;
                if(!strncmp(aa+2, ">>", 2)) cmp=SCMP_CMP_GT;
                if(!strncmp(aa+2, "&&", 2)) cmp=SCMP_CMP_MASKED_EQ;
                    
                if (!cmp) {
                    fprintf(stderr, "After An there should be comparison operator like"
                        " != << <= == => >> ot &&; in %s\n", argv[i]);
                    return 101;
                }
                
                if (cmp != SCMP_CMP_MASKED_EQ) {
                    scmp_datum_t datum;
                    if(sscanf(aa+4, "%lli", &datum)!=1) {
                        fprintf(stderr, "After AxOP there should be some sort of number in %s\n", argv[i]);
                        return 102;
                    }
                    
                    args[nargs++] = SCMP_CMP(aa[1]-'0', cmp, datum);
                } else {
                    scmp_datum_t mask;
                    scmp_datum_t datum;
                    if(sscanf(aa+4, "%lli==%lli", &mask, &datum)!=2) {
                        fprintf(stderr, "After Ax&& there should be number==number; in %s\n", argv[i]);
                        return 104;
                    }
                    
                    args[nargs++] = SCMP_CMP(aa[1]-'0', SCMP_CMP_MASKED_EQ, mask, datum);
                }
            } else
            if (aa[0]=='e') {
                int errno_;
                if (sscanf(aa+1,"%i", &errno_)!=1) {
                    fprintf(stderr, "After e should be number in %s\n", argv[i]);
                    return 105;
                }
                
                action = SCMP_ACT_ERRNO(errno_);
            } else
            if (aa[0]=='k') {
                action = SCMP_ACT_KILL;
            } else
            if (aa[0]=='a') {
                action = SCMP_ACT_ALLOW;
            } else {
                fprintf(stderr, "Unknown %c in %s\n", aa[0], argv[i]);
                return 107;
            }
        }
        
        int ret = seccomp_rule_add_hack(ctx, action, syscall, nargs, args);
        
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
