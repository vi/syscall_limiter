all: libwritelimiter.so writelimiter_broker

libwritelimiter.so: override.c send_fd.c recv_fd.c writelimiter.h
		${CC} ${CFLAGS} -Wall -g3 override.c send_fd.c recv_fd.c -shared -fPIC -ldl -o libwritelimiter.so

writelimiter_broker: writelimiter_broker.c send_fd.c recv_fd.c writelimiter.h popen2.h popen2.c safer.c
		${CC} ${CFLAGS} -Wall -g3 writelimiter_broker.c send_fd.c recv_fd.c safer.c popen2.c -o writelimiter_broker

