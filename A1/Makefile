all: ish
ish: ish.o
	gcc -std=gnu99 -o ish ish.o 
ish.o: ish.c
	gcc -Wall -std=gnu99 -g -c ish.c
clean:
	rm -f *.o
	rm -f ish
