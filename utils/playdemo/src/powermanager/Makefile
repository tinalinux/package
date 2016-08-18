SRC := c_powermanager.cpp
OBJ := $(SRC:.cpp=.o)

.PHONY:all clean

all:$(OBJ)

$(OBJ):$(SRC)
	$(CXX) -c $(CFLAGS) -o $@ $<

clean:
	$(RM) $(OBJ)
