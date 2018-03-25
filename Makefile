CPP_FILES	:= $(wildcard src/*.cpp)
OBJ_FILES	:= $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS	:= -std=c++11 -Wall -Wextra -pedantic-errors
CC_FLAGS	+= -O3

EXECUTABLE	:= kobla_2d_server

$(EXECUTABLE): $(OBJ_FILES)
	g++ $(LD_FLAGS) -o $@ $^ $(LD_LIBS)

obj/%.o: src/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $<
	
clean:
	rm -f obj/* $(EXECUTABLE)

CC_FLAGS += -MMD
-include $(OBJFILES:.o=.d)