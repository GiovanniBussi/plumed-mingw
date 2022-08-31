#! /usr/bin/env bash

file=$1

grep -q "\x00" $file  && exit 0

test -n "$PLUMED_REGTEST_DOS2UNIX" && $PLUMED_REGTEST_DOS2UNIX $file

sed "s/-\(0\.0*0 \)/ \1/g; 
     s/-\(0\.0*0$\)/ \1/g" $file > $file.$$.tmp
mv $file.$$.tmp $file

