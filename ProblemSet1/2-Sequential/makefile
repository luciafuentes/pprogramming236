EXE = Cellular2D-Sequential
CC = gcc
CFLAGS = -g -std=c11 -W -Wall -Winline -Wextra

all: $(EXE)

Cellular2D-Sequential: Cellular2D-Sequential.o 
	$(CC) $(CFLAGS) -o Cellular2D-Sequential Cellular2D-Sequential.o

Cellular2D-Sequential.o: Cellular2D-Sequential.c
	$(CC) $(CGLAGS) -c Cellular2D-Sequential.c

clean:
	@rm -f *.o *.exe 
	@rm -f Cellular2D-Sequential
	@echo Deleted .o and .exe files

run:
	./Cellular2D-Sequential initial_configuration.txt gameOfLife.txt 3


