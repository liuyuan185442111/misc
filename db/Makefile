single_thread = false
debug_version = true

cc = g++
cpp = g++
ld = g++

TOP_DIR = .

COMMON = common
DB = db
WDB = wdb
TOOL = tool
COM_DIR = $(TOP_DIR)/$(COMMON)
DB_DIR = $(TOP_DIR)/$(DB)
WDB_DIR = $(TOP_DIR)/$(WDB)
TOOL_DIR = $(TOP_DIR)/$(TOOL)

INCLUDES = $(COM_DIR) $(DB_DIR) $(WDB_DIR) $(TOOL_DIR)
TMP = $(TOP_DIR)/tmp
INCLUDES := -I. $(foreach n,$(INCLUDES),-I$(n))

DEFINES = -Wall -W -Wno-unused-parameter

ifeq ($(debug_version),true)
	DEFINES += -g -ggdb -DDEBUG
else
	DEFINES += -O2
endif

ifeq ($(single_thread),false)
	DEFINES += -D_REENTRANT_
endif
LDFLAGS = -pthread

#DEFINES += -DSAME_BYTE_ORDER -DDB_USE_MMAP
LDFLAGS += -lsnappy

DBSRC = $(COM_DIR)/octets.cpp $(COM_DIR)/conf.cpp $(COM_DIR)/split.cpp\
		$(DB_DIR)/performance.cpp $(DB_DIR)/page.cpp $(DB_DIR)/pagehash.cpp $(DB_DIR)/pagecache.cpp\
		$(WDB_DIR)/global_logger.cpp $(WDB_DIR)/table.cpp $(WDB_DIR)/storage.cpp\
		$(TOOL_DIR)/checkfileempty.cpp

DBOBJS = $(DBSRC:$(TOP_DIR)/%.cpp=$(TMP)/%.o)

all: testdb gamedb dbtool readmagic

testdb: $(DBOBJS) $(TOP_DIR)/testdb.cpp
	$(cpp) $(DEFINES) $(INCLUDES) $(LDFLAGS) $^ -o $@

gamedb: $(DBOBJS) $(TOP_DIR)/gamedb.cpp
	$(cpp) $(DEFINES) $(INCLUDES) $(LDFLAGS) $^ -o $@

dbtool: $(DBOBJS) $(TOP_DIR)/dbtool.cpp
	$(cpp) $(DEFINES) $(INCLUDES) $(LDFLAGS) $^ -o $@

readmagic: $(TOP_DIR)/readmagic.cpp $(TOP_DIR)/db/performance.cpp
	$(cpp) $(DEFINES) $(INCLUDES) $(LDFLAGS) $^ -o $@

gs: $(COM_DIR)/thread.cpp $(TOP_DIR)/gs.cpp
	$(cpp) $(DEFINES) -I$(COM_DIR) $(LDFLAGS) $^ -o $@

clean:
	@$(RM) testdb gamedb dbtool readmagic gs
	@rm -rf $(TMP)

$(DBOBJS): $(TMP)/%.o: $(TOP_DIR)/%.cpp
	@dirname $@ | xargs mkdir -p
	$(cpp) -c $(DEFINES) $(INCLUDES) $< -o $@

%.o: %.cpp
	$(cpp) -c $(DEFINES) $(INCLUDES) $< -o $@
