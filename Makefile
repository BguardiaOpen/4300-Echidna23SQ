## Old Makefile content ---
#cpsc4300: cpsc4300.o SQLExec.o
#	g++ -L/usr/local/db6/lib -o $@ $< -ldb_cxx -lsqlparser
#
#cpsc4300.o: cpsc4300.cpp
#	g++ -I/usr/local/db6/include -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -std=c++11 -c -o $@ $<
#
#%.o: %.cpp
#	g++ -I/usr/local/db6/include -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -ggdb -o "$@" "$<"
#
#clean:
#	rm -f *.o cpsc4300
#

EXECUTABLE_FILE = sql4300

all: main

main: cpsc4300.o heap_storage.o SQLExec.o
	g++ -L/usr/local/db6/lib -o $(EXECUTABLE_FILE) cpsc4300.o heap_storage.o SQLExec.o -ldb_cxx -lsqlparser

cpsc4300.o:    heap_storage.h storage_engine.h
heap_storage.o: heap_storage.h storage_engine.h
SQLExec.o:      SQLExec.h SchemaTables.cpp

#compilation rules for all object files
%.o : %.cpp
	g++ -I/usr/local/db6/include -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -fpermissive -O3 -std=c++11 -c -o $@ $<

clean : 
	rm -f $(EXECUTABLE_FILE) *.o
