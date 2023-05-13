OBJS = SQLExec.o SchemaTables.o storage_engine.o cpsc4300.o

cpsc4300: $(OBJS)
	g++ -L/usr/local/db6/lib -o $@ $< -ldb_cxx -lsqlparser

HEAPSTORAGE_H = heap_storage.h storage_engine.h
SCHEMATABLES_H = SchemaTables.h $(HEAPSTORAGE_H)
SQLEXEC_H = SQLExec.h $(SCHEMATABLES_H)
HeapStorage.o : $(HEAPSTORAGE_H)
SQLExec.o : $(SQLEXEC_H)
SchemaTables.o : $(SCHEMATABLES_H)
storage_engine.o : storage_engine.h
cpsc4300.o : $(SQLEXEC_H)

%.o: %.cpp
	g++ -I/usr/local/db6/include -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -ggdb -o "$@" "$<"

clean:
	rm -f *.o cpsc4300

