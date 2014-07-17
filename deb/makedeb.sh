#!/bin/bash
set -e

P=syscalllimiter
V=0.1
FILES=".gitignore LICENSE Makefile README.md limit_syscalls.c monitor.sh writelimiter "

rm -Rf "$P"-$V
trap "rm -fR \"$P\"-$V" EXIT 
mkdir "$P"-$V
for i in $FILES; do cp -Rv ../"$i" "$P"-$V/; done
tar -czf ${P}_$V.orig.tar.gz "$P"-$V

cp -R debian "$P"-$V
(cd "$P"-$V && debuild)
