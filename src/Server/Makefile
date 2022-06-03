TARGET:=Server
SRC:=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp,%.o,$(SRC))
CXXFLAGS:= -Wall -g -Wreorder
LIBS:=-lthread

$(TARGET):$(OBJS)
	$(CXX) $^ -o $@ $(CXXFLAGS) $(LIBS)

all:
	echo $(SRC)

.PHONY:
clean:
	$(RM) $(OBJS) $(TARGET)
