EXE = Cellular1D-Sequential
CC = gcc
CFLAGS = -g -std=c11 -W -Wall -Winline -Wextra

all: $(EXE)

Cellular1D-Sequential: Cellular1D-Sequential.o 
	$(CC) $(CFLAGS) -o Cellular1D-Sequential Cellular1D-Sequential.o

Cellular1D-Sequential.o: Cellular1D-Sequential.c
	$(CC) $(CGLAGS) -c Cellular1D-Sequential.c 

clean:
	@rm -f *.o *.exe *.gch 
	@rm -f Cellular1D-Sequential
	@echo Deleted .o and .exe files

run:
	./Cellular1D-Sequential middle30.txt mod2.txt 31
