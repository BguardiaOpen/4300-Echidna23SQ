<<<<<<< HEAD
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
=======
OBJS = SQLExec.o SchemaTables.o storage_engine.o cpsc4300.o ParseTreeToString.o HeapStorage.o

cpsc4300: $(OBJS)
	g++ -L/usr/local/db6/lib -o $@ $< -ldb_cxx -lsqlparser -w

HEAPSTORAGE_H = heap_storage.h storage_engine.h
SCHEMATABLES_H = SchemaTables.h $(HEAPSTORAGE_H)
SQLEXEC_H = SQLExec.h $(SCHEMATABLES_H)
ParseTreeToString.o : ParseTreeToString.h
HeapStorage.o : $(HEAPSTORAGE_H)
SQLExec.o : $(SQLEXEC_H)
SchemaTables.o : $(SCHEMATABLES_H) ParseTreeToString.h
storage_engine.o : storage_engine.h
cpsc4300.o : $(SQLEXEC_H) ParseTreeToString.h

%.o: %.cpp
	g++ -I/usr/local/db6/include -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -w -ggdb -o "$@" "$<"
>>>>>>> c84a5c87c0f4143d9f20e304b9aa7c5df82c258d

main: cpsc4300.o heap_storage.o SQLExec.o
	g++ -L/usr/local/db6/lib -o $(EXECUTABLE_FILE) cpsc4300.o heap_storage.o SQLExec.o -ldb_cxx -lsqlparser

<<<<<<< HEAD
cpsc4300.o:    heap_storage.h storage_engine.h
heap_storage.o: heap_storage.h storage_engine.h
SQLExec.o:      SQLExec.h SchemaTables.cpp

#compilation rules for all object files
%.o : %.cpp
	g++ -I/usr/local/db6/include -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -fpermissive -O3 -std=c++11 -c -o $@ $<

clean : 
	rm -f $(EXECUTABLE_FILE) *.o
=======
# Makefile, Kevin Lundeen, Seattle University, CPSC5300, Winter 2023
# 
# CCFLAGS     = -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -ggdb
# COURSE      = /usr/local/db6
# INCLUDE_DIR = $(COURSE)/include
# LIB_DIR     = $(COURSE)/lib

# # following is a list of all the compiled object files needed to build the sql5300 executable
# OBJS       = sql5300.o SlottedPage.o HeapFile.o HeapTable.o ParseTreeToString.o SQLExec.o schema_tables.o storage_engine.o

# # Rule for linking to create the executable
# # Note that this is the default target since it is the first non-generic one in the Makefile: $ make
# sql5300: $(OBJS)
# 	g++ -L$(LIB_DIR) -o $@ $(OBJS) -ldb_cxx -lsqlparser

# # In addition to the general .cpp to .o rule below, we need to note any header dependencies here
# # idea here is that if any of the included header files changes, we have to recompile
# HEAP_STORAGE_H = heap_storage.h SlottedPage.h HeapFile.h HeapTable.h storage_engine.h
# SCHEMA_TABLES_H = schema_tables.h $(HEAP_STORAGE_H)
# SQLEXEC_H = SQLExec.h $(SCHEMA_TABLES_H)
# ParseTreeToString.o : ParseTreeToString.h
# SQLExec.o : $(SQLEXEC_H)
# SlottedPage.o : SlottedPage.h
# HeapFile.o : HeapFile.h SlottedPage.h
# HeapTable.o : $(HEAP_STORAGE_H)
# schema_tables.o : $(SCHEMA_TABLES_) ParseTreeToString.h
# sql5300.o : $(SQLEXEC_H) ParseTreeToString.h
# storage_engine.o : storage_engine.h

# # General rule for compilation
# %.o: %.cpp
# 	g++ -I$(INCLUDE_DIR) $(CCFLAGS) -o "$@" "$<"

# # Rule for removing all non-source files (so they can get rebuilt from scratch)
# # Note that since it is not the first target, you have to invoke it explicitly: $ make clean
# clean:
# 	rm -f sql5300 *.o
>>>>>>> c84a5c87c0f4143d9f20e304b9aa7c5df82c258d
