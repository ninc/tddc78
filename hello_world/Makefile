
CFLAGS = 

LFLAGS= -lpthread -lrt -lm

all: hello_world

clean:
	-$(RM) *.o hello_world

hello_world: main.o
	cc -o $@ main.o $(LFLAGS)