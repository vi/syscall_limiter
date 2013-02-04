#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: ./monitor.sh program [arguments]"
    echo "It should output the limit_syscalls command pointing with all occured syscall enabled"
    exit 1
fi

strace -f -o >( bash -c "(perl -ne '/^\d+\s+([a-z_0-9]+)\(/ and print \"\$1\n\"' | sort | uniq | sed 's/^_exit\$/exit/' | tr '\n' ' ' | sed 's/^/limit_syscalls /; s/\$/ -- /'; which \"$1\" ; echo)>&2";  ) -- "$@"
