if [ -z $1 ];then
	echo usage: $0 dbhome
	exit 1
fi
if [ ! -f dbtool ];then
	echo "dbtool not exist"
	exit 1
fi
if [ ! -x dbtool ];then
	chmod u+x dbtool
fi
echo -e "dbtool \c"
./dbtool -v
home=${1%*/}
data="data"
mkdir $home/${data}.fix
if [ $? -ne 0 ]; then
	exit 1
fi
ls $home/$data/ | awk '{print "./dbtool -r -s '"$home"'/'"${data}"'/"$1" -d '"$home"'/'"${data}"'.fix/"$1}' | /bin/sh
