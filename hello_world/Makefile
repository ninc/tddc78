COMPILER = icc

LFLAGS = -Nmpi

# -lpthread -lrt -lm -Nmpi

all: hello_world

clean:
	-$(RM) *.o hello_world

hello_world: main.o
	$(COMPILER) $(LFLAGS) -o main main.c