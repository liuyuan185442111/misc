#!/bin/sh
#file args like:
#--threads=2 --num=10000 --value_size=1024 --write_buffer_size=4048576 --benchmarks=fillrandom
if [ $# -eq 0 ];then
	xargs -a args $0
	exit $?
fi
name=$*
name=${name//_/}
name=${name// /_}
name=${name//-/}
name=${name//=/}
if [ -f $name ];then
	echo $name "exists"
	exit 2
fi
env time ./db_bench $* --histogram=1 >$name 2>${name}.err
tail -2 ${name}.err >> $name
