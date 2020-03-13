#!/bin/sh
#file args like:
#--num=10000 --value_size=1024 --benchmarks=fillrandom,readrandom

if [ $# -eq 0 ];then
	if [ ! -z $recursion_flag ];then
		if [ $recursion_flag -eq 6379 ]
		then
			echo "args is empty"
			exit 1
		fi
	fi
	export recursion_flag=6379
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

if [ -x db_bench ];then
	env time ./db_bench $* --histogram=1 >$name 2>${name}.err
	tail -2 ${name}.err >> $name
fi

if [ -x bench ];then
	env time ./bench $* --histogram=1 >$name 2>${name}.err
	tail -2 ${name}.err >> $name
fi
