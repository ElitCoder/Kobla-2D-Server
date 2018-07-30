CPP_FILES	:= $(wildcard src/*.cpp)
OBJ_FILES	:= $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
CC_FLAGS	:= -std=c++14 -Wall -Wextra -pedantic-errors
#CC_FLAGS	+= -O3
#CC_FLAGS	+= -g
LD_LIBS		:= -lpthread -ltmx-loader -lsfml-graphics -lstdc++ -lm

EXECUTABLE	:= bin/Kobla-2D-Server-Rebased

$(EXECUTABLE): $(OBJ_FILES)
	zapcc $(LD_FLAGS) -o $@ $^ $(LD_LIBS)

obj/%.o: src/%.cpp
	zapcc $(CC_FLAGS) -c -o $@ $<
	
clean:
	rm -f obj/* $(EXECUTABLE)

CC_FLAGS += -MMD
-include $(OBJFILES:.o=.d)