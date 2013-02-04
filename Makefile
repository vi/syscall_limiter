all: limit_syscalls

limit_syscalls: limit_syscalls.c
		${CC} -ggdb limit_syscalls.c -lseccomp -o limit_syscalls