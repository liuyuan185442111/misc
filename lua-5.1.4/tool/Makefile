# makefile for Lua etc

TOP= ..
LIB= $(TOP)/src
INC= $(TOP)/src
BIN= $(TOP)/src
SRC= $(TOP)/src

CC= gcc
CFLAGS= -O2 -Wall -I$(INC) $(MYCFLAGS)
MYCFLAGS= 
MYLDFLAGS= -Wl,-E
MYLIBS= -lm
#MYLIBS= -lm -Wl,-E -ldl -lreadline -lhistory -lncurses
RM= rm -f

default:
	@echo 'Please choose a target: min noparser one strict clean'

noparser: noparser.o
	$(CC) noparser.o $(SRC)/lua.o -L$(LIB) -llua $(MYLIBS)
	-./a.out -e'a=1'

one:
	$(CC) $(CFLAGS) all.c $(MYLIBS)

strict:
	-$(BIN)/lua -e 'print(a);b=2'
	-$(BIN)/lua -lstrict -e 'print(a)'
	-$(BIN)/lua -e 'function f() b=2 end f()'
	-$(BIN)/lua -lstrict -e 'function f() b=2 end f()'

clean:
	$(RM) a.out core core.* *.o luac.out

.PHONY:	default noparser one strict clean
