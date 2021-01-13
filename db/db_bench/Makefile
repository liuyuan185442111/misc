debug_version = false

cpp = g++
ld = g++

DB_DIR = ../db

INCLUDES = -I$(DB_DIR)

DEFINES = -Wall

ifeq ($(debug_version),true)
	DEFINES += -g -ggdb -DDEBUG
else
	DEFINES += -O2
endif

DBSRC = $(DB_DIR)/performance.cpp $(DB_DIR)/page.cpp $(DB_DIR)/pagehash.cpp $(DB_DIR)/pagecache.cpp

DBOBJS = $(DBSRC:$(DB_DIR)/%.cpp=%.o)

bench: $(DBOBJS) histogram.cc db_bench.cpp
	$(cpp) $(DEFINES) $(INCLUDES) $^ -o $@

$(DBOBJS): %.o: $(DB_DIR)/%.cpp
	$(cpp) -c $(DEFINES) $(INCLUDES) $< -o $@

clean:
	$(RM) bench $(DBOBJS)

%.o: %.cpp
	$(cpp) -c $(DEFINES) $(INCLUDES) $< -o $@
