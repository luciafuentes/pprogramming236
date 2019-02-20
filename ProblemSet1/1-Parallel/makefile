EXE = Cellular1D-Parallel
CC = mpicc
CFLAGS = -g -std=c11 -W -Wall -Winline -Wextra
MPIEXE = mpiexec
NP = -n 10
MPIFLAGS = -quiet

all: $(EXE)

Cellular1D-Parallel: Cellular1D-Parallel.o 
	$(CC) $(CFLAGS) -o Cellular1D-Parallel Cellular1D-Parallel.o functions.o -lm

Cellular1D-Parallel.o: Cellular1D-Parallel.c
	$(CC) $(CGLAGS) -c Cellular1D-Parallel.c functions.c -lm

functions.o: functions.c functions.h
	$(CC) $(CGLAGS) -c functions.c functions.h -lm

run:
	$(MPIEXE) $(NP) $(MPIFLAGS) ./Cellular1D-Parallel initial_configuration.txt transformation_function.txt 31

clean:
	@rm -f *.o *.exe
	@rm -f Cellular1D-Parallel
	@echo Deleted .o and .exe files
