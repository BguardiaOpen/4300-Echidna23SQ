cpsc4300: cpsc4300.o
	g++ -L/usr/local/db6/lib -o $@ $< -ldb_cxx -lsqlparser

cpsc4300.o: cpsc4300.cpp
	g++ -I/usr/local/db6/include -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -std=c++11 -c -o $@ $<

%.o: %.cpp
	g++ -I/usr/local/db6/include -std=c++11 -std=c++0x -Wall -Wno-c++11-compat -DHAVE_CXX_STDHEADERS -D_GNU_SOURCE -D_REENTRANT -O3 -c -ggdb -o "$@" "$<"

clean:
	rm -f cpsc4300.o cpsc4300

