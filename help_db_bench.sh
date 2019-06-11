#!/bin/sh
if [ $# -eq 0 ];then
	echo $0 "need parameters"
	exit 1
fi
name=${1// /_}
name=${name//-/}
name=${name//=/}
if [ -f $name ];then
	echo $name "exists"
	exit 2
fi
env time ./db_bench --histogram=1 $1 >$name 2>${name}.err
tail -2 ${name}.err >> $name
