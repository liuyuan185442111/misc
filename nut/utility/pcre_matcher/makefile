all:
	g++ test.cpp -L. -lmatch /lib64/libpcre.so.0.0.1
o:
	gcc -c pcre_matcher.c -o pcre_matcher.o
so:
	gcc -c pcre_matcher.c -o libmatch.so -shared -fPIC
