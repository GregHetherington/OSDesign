all: ish
ish: ish.o
	gcc -o ish ish.o 
ish.o: ish.c
	gcc -Wall -g -c ish.c
clean:
	rm -f *.o
	rm -f ish
