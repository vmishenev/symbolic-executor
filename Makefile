# Makefile
PROJECT_ROOT = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OBJS	= bison.o lex.o main.o

CC	= g++
CFLAGS	= -g -Wall -std=c++11

calc:		$(OBJS)
		$(CC) $(CFLAGS) $(OBJS) -o calc -lfl -lcvc4 -lgmp

lex.c:		calc.lex 
		flex calc.lex
		cp lex.yy.c lex.c

bison.c:	calc.y
		bison -d -v calc.y
		cp calc.tab.c bison.c
		cmp -s calc.tab.h tok.h || cp calc.tab.h tok.h

lex.o:		lex.c
		g++ $(CFLAGS) -c $(PROJECT_ROOT)lex.c -o lex.o
		
bison.o:	bison.c
		$(CC) $(CFLAGS) -c $(PROJECT_ROOT)bison.c -o bison.o

main.o:		main.cc
		$(CC) $(CFLAGS) -c $(PROJECT_ROOT)main.cc -o main.o

lex.o yac.o main.o	: heading.h
lex.o main.o		: tok.h
main.o			: sygma.h

clean:
	rm -f *.o *~ lex.c lex.yy.c bison.c tok.h calc.tab.c calc.tab.h calc.output calc

