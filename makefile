FCC = mpicc
LDC = mpicc
LD_FLAGS= -Wall -fopenmp 
FLAGS= -Wall -fopenmp 
PROG = mergesort 
RM = /bin/rm
OBJS = ms.o

#all rule
all: $(PROG)

$(PROG): $(OBJS)
	$(LDC) $(LD_FLAGS) $(OBJS) -o $(PROG)

%.o: %.c
	$(FCC) $(FLAGS) -c $<

#clean rule
clean:
	$(RM) -rf *.o $(PROG) *.mod
