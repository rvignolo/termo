#!/bin/sh

input=test.was

if [ -e ${input} ]; then
 testdir=""
elif [ -e examples/${input} ]; then
 testdir="examples/"
else
 echo wrong PWD
 exit 1
fi

if [ -e ./mate ]; then
 matebin=./mate
elif [ -e ../mate ]; then
 matebin=../mate
elif [ ! -z "`which wasora`" ]; then
 matebin="wasora -p mate"
else
 echo "do not know how to run mate :("
 exit 1
fi

${matebin} ${testdir}${input}
outcome=$?

exit $outcome
