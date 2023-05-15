
#note: for whatever reason, Ryan's makefile wasn't linking everything properly,
#			 so we used my makefile from the previous Sprint --Ishan
#
#EXECUTABLE_FILE = sql4300
#
#all: main
##
#main: heap_storage.o SQLExec.o SchemaTables.o cpsc4300.o
#	g++ -L/usr/local/db6/lib -o $(EXECUTABLE_FILE) cpsc4300.o heap_storage.o SQLExec.o SchemaTables.o -ldb_cxx -lsqlparser
#
#SQLExec.o:      SQLExec.h SchemaTables.h heap_storage.h
#SchemaTables.o: heap_storage.h storage_engine.h
#heap_storage.o: heap_storage.h storage_engine.h
#
#cpsc4300.o: SQLExec.h SchemaTables.h heap_storage.h storage_engine.h
#
##compilation rules for all object files
#%.o : %.cpp
#	g++ -I/usr/local/db6/include -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -ggdb -o $@ $<
#
#clean : 
#	rm -f $(EXECUTABLE_FILE) *.o
#

CCFLAGS     = -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -ggdb
COURSE      = /usr/local/db6
INCLUDE_DIR = $(COURSE)/include
LIB_DIR     = $(COURSE)/lib

OBJS       =  storage_engine.o SlottedPage.o HeapFile.o HeapTable.o heap_storage.o ParseTreeToString.o SchemaTables.o SQLExec.o  cpsc4300.o 

cpsc4300: $(OBJS)
	g++ -L$(LIB_DIR) $(OBJS) -ldb_cxx -lsqlparser -o $@


storage_engine.o : storage_engine.h 

SlottedPage.o: SlottedPage.h 

HeapFile.o: HeapFile.h

HeapTable.o: HeapTable.h 

heap_storage.o: heap_storage.h

ParseTreeToString.o : ParseTreeToString.h

SchemaTables.o : SchemaTables.h

SQLExec.o : SQLExec.h

cpsc4300.o: cpsc4300.cpp


# General rule for compilation
%.o: %.cpp *.h
	g++ -I$(INCLUDE_DIR) $(CCFLAGS) -o "$@" "$<" -ldb_cxx -lsqlparser


clean:
	rm -f cpsc300 *.o
