all: limit_syscalls ban_CLONE_NEWUSER

limit_syscalls: limit_syscalls.c
		${CC} ${CFLAGS} -Wall limit_syscalls.c -lseccomp -o limit_syscalls

ban_CLONE_NEWUSER: ban_CLONE_NEWUSER.c
		${CC} ${CFLAGS} -Wall ban_CLONE_NEWUSER.c -lseccomp -o ban_CLONE_NEWUSER
