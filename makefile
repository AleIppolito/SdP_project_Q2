CC	= g++
CPPFLAGS= -Wno-deprecated -c -O3 -pthread -g
LDFLAGS	= -O3  -pthread
SOURCES	= main.cpp Graph.cpp GraphUtil.cpp Grail.cpp 
OBJECTS	= $(SOURCES:.cpp=.o) interval_tree.o
EXECUTABLE=grail

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE) : $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(FCFLAGS) -o $@

.cpp.o : 
	$(CC) $(CPPFLAGS) $< -o $@


clean:
	-rm -f *.o grail
