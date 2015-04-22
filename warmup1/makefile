warmup1: warmup1.o my402list.o
	gcc -o warmup1 -g warmup1.o my402list.o

warmup1.o: warmup1.c my402list.h
	gcc -g -std=c99 -c -Wall warmup1.c

listtest: my402list.o listtest.o
	gcc -o listtest -g my402list.o listtest.o

listtest.o: my402list.h listtest.c
	gcc -g -c -std=c99 -Wall listtest.c

my402list.o: my402list.c my402list.h
	gcc -g -std=c99 -c -Wall my402list.c

clean:
	rm -f *.o warmup1 listtest
