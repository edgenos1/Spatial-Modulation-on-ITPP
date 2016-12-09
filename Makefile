# specify the target file below
EXEC := spatial_modulation

# specify any compile options, such as -g -Wall ...
CXXFLAGS := -g -Wall

# any libs to be linked
LIBS :=

#additional sources
SRCS :=

# location of header files
SOURCEPATH := $(ITPP_HOME)/include

##########################################################
# You shouldn't need to change anything below this point
# ########################################################

ITPPFLAGS :=  `itpp-config --cflags`
ITPPLIBS := `itpp-config --libs`
CXXFLAGS += $(ITPPFLAGS) -I$(SOURCEPATH)

CXX := g++-5
SRCS += $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)


.Phony: all depend objs clean rebuild run

all: $(EXEC)

depend:
	@$(CXX) -MM $(CXXFLAGS) $(SRCS) > .depend

objs: $(OBJS)

clean:
	@rm -f *.o

rebuild: clean all

run: ./$(EXEC)
	./$(EXEC) ${ARGS}


$(EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(addprefix -l , $(LIBS)) $(ITPPLIBS)

-include .depend
