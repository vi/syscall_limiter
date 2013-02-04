This program allows user to set up simple [seccomp][1] filter that limits which syscalls can be called. All the rest syscalls return pre-defined error value `Channel number out of range`.

The supplied `monitor.sh` script assists executing of binaries under strace for automatic creating of the allowed syscalls list.

This program uses [libseccomp][2] library.

Here is pre-built static x86 binary: [limit_syscalls_static][3].

Examples
===

Basic
---

```
$ ./limit_syscalls
Usage: limit_syscalls syscall1 syscall2 ... syscallN -- program [arguments]
Example:
   limit_syscalls execve exit write read open close mmap2  fstat64 access  mprotect set_thread_area -- /bin/echo qqq
   
   
$ ./limit_syscalls execve exit write read open close mmap2  fstat64 access  mprotect set_thread_area -- /bin/echo qqq
qqq

$ # Let's cut off "exit".
$ ./limit_syscalls execve write read open close mmap2  fstat64 access  mprotect set_thread_area -- /bin/echo qqq
qqq
Segmentation fault

```

ping
---

```
# ./monitor.sh ping 127.0.0.1
PING 127.0.0.1 (127.0.0.1) 56(84) bytes of data.
64 bytes from 127.0.0.1: icmp_req=1 ttl=64 time=0.132 ms
64 bytes from 127.0.0.1: icmp_req=2 ttl=64 time=0.108 ms
^C
--- 127.0.0.1 ping statistics ---
<cut>
# limit_syscalls access brk close connect execve exit_group fstat64 getpid getsockname getsockopt gettimeofday getuid32 ioctl mmap2 mprotect munmap open poll read recvmsg rt_sigaction sendmsg setsockopt set_thread_area setuid32 sigreturn socket write  -- /bin/ping

# ./limit_syscalls access brk close connect execve exit_group fstat64 getpid getsockname getsockopt gettimeofday getuid32 ioctl mmap2 mprotect munmap open poll read recvmsg rt_sigaction sendmsg setsockopt set_thread_area setuid32 sigreturn socket write  -- /bin/ping 127.0.0.1
PING 127.0.0.1 (127.0.0.1) 56(84) bytes of data.
64 bytes from 127.0.0.1: icmp_req=1 ttl=64 time=0.132 ms
64 bytes from 127.0.0.1: icmp_req=2 ttl=64 time=0.108 ms
^C
--- 127.0.0.1 ping statistics ---
2 packets transmitted, 2 received, 0% packet loss, time 999ms
rtt min/avg/max/mdev = 0.108/0.120/0.132/0.012 ms

# 
```

firefox
---

```
$ ./monitor.sh /opt/firefox-17esr/firefox
$
$ limit_syscalls access bind brk chmod clock_getres clock_gettime clone close connect dup2 epoll_create epoll_ctl epoll_wait eventfd2 execve exit exit_group fallocate fcntl64 fstat64 fstatfs fsync ftruncate64 futex getcwd getdents getdents64 getegid32 geteuid32 getgid32 getpeername getresgid32 getresuid32 getrlimit getrusage getsockname getsockopt gettid gettimeofday getuid32 ioctl _llseek lseek lstat64 madvise mkdir mmap2 mprotect munmap open pipe poll prctl pwrite64 read readahead readlink recv recvfrom recvmsg rename rmdir rt_sigaction rt_sigprocmask sched_getaffinity sched_getparam sched_get_priority_max sched_get_priority_min sched_getscheduler sched_setscheduler send sendmsg sendto set_robust_list setsockopt set_thread_area set_tid_address shmat shmctl shmdt shmget shutdown sigaltstack socket socketpair stat64 statfs statfs64 symlink sysinfo time umask uname unlink utime waitpid write writev  -- /opt/firefox-17esr/firefox

$ ./limit_syscalls access bind brk chmod clock_getres clock_gettime clone close connect dup2 epoll_create epoll_ctl epoll_wait eventfd2 execve exit exit_group fallocate fcntl64 fstat64 fstatfs fsync ftruncate64 futex getcwd getdents getdents64 getegid32 geteuid32 getgid32 getpeername getresgid32 getresuid32 getrlimit getrusage getsockname getsockopt gettid gettimeofday getuid32 ioctl _llseek lseek lstat64 madvise mkdir mmap2 mprotect munmap open pipe poll prctl pwrite64 read readahead readlink recv recvfrom recvmsg rename rmdir rt_sigaction rt_sigprocmask sched_getaffinity sched_getparam sched_get_priority_max sched_get_priority_min sched_getscheduler sched_setscheduler send sendmsg sendto set_robust_list setsockopt set_thread_area set_tid_address shmat shmctl shmdt shmget shutdown sigaltstack socket socketpair stat64 statfs statfs64 symlink sysinfo time umask uname unlink utime waitpid write writev  -- /opt/firefox-17esr/firefox

(firefox:10950): Gdk-WARNING **: shmget failed: error 44 (Channel number out of range)

$ # firefox seems to be working although

```

[1]:http://en.wikipedia.org/wiki/Seccomp
[2]:http://sourceforge.net/projects/libseccomp/
[3]:http://vi-server.org/pub/limit_syscalls_static
