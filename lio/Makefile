CC = g++
LD = g++

SINGLE_THREAD = false

DONE = done
IO_DIR = io

INCLUDES = -I. -I$(DONE) -I$(IO_DIR)
DEFINES += -Wall -O0 -g -ggdb
LDFLAGS += -O0 -g -ggdb

SHARESRC = $(DONE)/octets.cpp $(DONE)/thread.cpp $(DONE)/timer.cpp $(IO_DIR)/itimer.cpp $(IO_DIR)/pollio.cpp $(IO_DIR)/httpclient.cpp

ifeq ($(SINGLE_THREAD),true)
	SHAREOBJ := $(SHARESRC:%.cpp=%.o)
else
	DEFINES = -pthread -D_DEADLOCK_DETECT_ -D_REENTRANT_
	LDFLAGS = -pthread
	SHAREOBJ := $(SHARESRC:%.cpp=%_m.o)
endif

OBJS = message.o gmatrix.o

delivery : $(SHAREOBJ) $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(SHAREOBJ) $(OBJS)

ifeq ($(SINGLE_THREAD),true)
$(SHAREOBJ): %.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $< -o $@
else
$(SHAREOBJ): %_m.o: %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $< -o $@
endif

%.o : %.cpp
	$(CC) -c $(DEFINES) $(INCLUDES) $< -o $@

clean:
	rm -rf $(SHAREOBJ) $(OBJS) delivery
