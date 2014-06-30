CPP=$(if $(INTEL), icpc, g++)
MPICPP=$(if $(INTEL), icpc, mpic++)

CFLAGS  += -tcollect -I$(VT_ROOT)/include
FFLAGS  += -tcollect -I$(VT_ROOT)/include
LFLAGS  += -L$(VT_LIB_DIR) -lVT $(VT_ADD_LIBS)

SRC=physics.c ll.cpp
OBJS=$(SRC:.cpp=.o)
TARGETS=mainMPI2

all: $(TARGETS) objclean

mainMPI2: $(OBJS) mainMPI2.cpp
	icpc -Nmpi -g $(CFLAGS) $(MPIFLAGS) $(LFLAGS) $(FFLAGS) $(OBJS) \
		mainMPI2.cpp -o mainMPI2

%.o: %.cpp
	$(CPP) $(CFLAGS) -c $<

objclean:
	$(RM) *.o

clean: objclean
	$(RM) -R mainMPI2 only-set-in-job-environment *.stf* *.prot
