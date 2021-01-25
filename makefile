CC	= g++
CPPFLAGS= -Wno-deprecated -O3 -c -g -ggdb
LDFLAGS	= -O3 
SOURCES	= main.cpp Graph.cpp GraphUtil.cpp Grail.cpp TCSEstimator.cpp 
OBJECTS	= $(SOURCES:.cpp=.o) interval_tree.o
EXECUTABLE=grail

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(FCFLAGS) -o $@

.cpp.o : 
	$(CC) $(CPPFLAGS) $< -o $@

interval_tree.o : interval_tree.c interval_tree.h templatestack.h
	g++ -c interval_tree.c 

clean:
	-rm -f *.o grail
