CC=cc
ARGS=-Wall -pedantic -std=c99
ARGS=-Wall -Wextra -pedantic -std=c99
OBJS=main.o file.o parse_c.o parse_cpp.o print.o

compro: $(OBJS) Makefile
	cc $(OBJS) -o compro

main.o: main.c globals.h
	$(CC) $(ARGS) -c main.c

file.o: file.c globals.h
	$(CC) $(ARGS) -c file.c

parse_c.o: parse_c.c globals.h
	$(CC) $(ARGS) -c parse_c.c

parse_cpp.o: parse_cpp.c globals.h
	$(CC) $(ARGS) -c parse_cpp.c

print.o: print.c globals.h
	$(CC) $(ARGS) -c print.c

clean:
	rm -f compro *.o
