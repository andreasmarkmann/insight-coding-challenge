
include Version

PROJECT = rolling_median
OBJ = rolling_median.o epochtime.o hashtable.o graph.o stringutils.o venmodata.o venmoio.o

INC = -I/usr/local/include
LIB = -lm
CDBG = -g -ggdb
#CDBG = -D{NDEBUG}
COPT = -std=c++11
#COPT = -std=c++11 -O2

CXX = g++
CXXFLAGS = -DVERSION=\"$(MAJOR).$(MINOR).$(PATCH)\" $(CDBG) $(INC) $(COPT)

all: $(PROJECT)

$(PROJECT): $(OBJ)
	$(CXX) -o $@ $(OBJ) $(INC) $(LIB);

archive:
	mkdir -p Archive;\
	tar cvf - Makefile *.cpp *.h *.txt \
	| gzip -c > Archive/$(PROJECT)_$(MAJOR).$(MINOR).$(PATCH).tar.gz

clean:
	rm *.o $(PROJECT)

## ../script/mkinclude.sh output follows:
epochtime.o: epochtime.cpp epochtime.h stringutils.h
graph.o: graph.cpp stringutils.h epochtime.h venmodata.h venmoio.h hashtable.h graph.h
hashtable.o: hashtable.cpp graph.h stringutils.h
rolling_median.o: rolling_median.cpp venmodata.h venmoio.h hashtable.h graph.h stringutils.h
stringutils.o: stringutils.cpp epochtime.h stringutils.h
venmodata.o: venmodata.cpp venmodata.h
venmoio.o: venmoio.cpp venmoio.h venmodata.h epochtime.h stringutils.h
