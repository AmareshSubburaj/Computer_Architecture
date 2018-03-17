CC = g++ -std=c++11
#OPT = -O3 -m32
#OPT = -g -m32
WARN = -Wall
CFLAGS = $(WARN) $(INC) $(LIB)

# List all your .cc files here (source files, excluding header files)
SIM_SRC = main.cpp processor.cpp cache.cpp 

# List corresponding compiled object files here (.o files)
SIM_OBJ = main.o processor.o  cache.o 
 
#################################

# default rule

all: sim
	@echo "my work is done here..."


# rule for making sim_cache

sim: $(SIM_OBJ)
	$(CC) -o sim $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM_CACHE-----------"


# generic rule for converting any .cc file to any .o file
 
.cpp.o:
	$(CC) $(CFLAGS)  -c $*.cpp


# type "make clean" to remove all .o files plus the sim_cache binary

clean:
	rm -f *.o sim


# type "make clobber" to remove all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o


