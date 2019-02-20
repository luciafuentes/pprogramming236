EXE = Cellular2D-Parallel
CC = mpicc
CFLAGS = -g -std=c11 -W -Wall -Winline -Wextra
MPIEXE = mpiexec
NP = -n 2
MPIFLAGS = -quiet

all: $(EXE)

Cellular2D-Parallel: Cellular2D-Parallel.o 
	$(CC) $(CFLAGS) -o Cellular2D-Parallel Cellular2D-Parallel.o functions.o -lm

Cellular2D-Parallel.o: Cellular2D-Parallel.c
	$(CC) $(CGLAGS) -c Cellular2D-Parallel.c functions.c -lm

functions.o: functions.c functions.h
	$(CC) $(CGLAGS) -c functions.c functions.h -lm

run:
	$(MPIEXE) $(NP) $(MPIFLAGS) ./Cellular2D-Parallel initial_configuration.txt transformation_function.txt 3

clean:
	@rm -f *.o *.exe
	@rm -f Cellular2D-Parallel
	@echo Deleted .o and .exe files
