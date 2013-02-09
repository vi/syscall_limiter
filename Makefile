all: limit_syscalls

limit_syscalls: limit_syscalls.c
		${CC} -Wall -ggdb limit_syscalls.c -lseccomp -o limit_syscalls
