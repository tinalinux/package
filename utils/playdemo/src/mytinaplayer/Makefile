TARGET := c_tinaplayer

SRCS := $(TARGET).cpp
OBJS := $(SRCS:.cpp=.o)
INCLUDES = -I../ \
		   -I../bluetooth/

.PHONY:all clean

all:$(OBJS)

%.o:%.cpp
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -rf *.o
