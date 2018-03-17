CC = g++ -std=c++11
#OPT = -O3 -m32
#OPT = -g -m32
WARN = -Wall
CFLAGS = $(WARN) $(INC) $(LIB)

# List all your .cc files here (source files, excluding header files)
SIM_SRC = main.cpp L2cache.cpp L1cache.cpp IssueQueue.cpp Dynscheduler.cpp DispatchQueue.cpp

# List corresponding compiled object files here (.o files)
SIM_OBJ = main.o IssueQueue.o  L1cache.o L2cache.o Dynscheduler.o DispatchQueue.o
 
#################################

# default rule

all: sim_cache
	@echo "my work is done here..."


# rule for making sim_cache

sim_cache: $(SIM_OBJ)
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


