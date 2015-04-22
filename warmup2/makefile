warmup2: warmup2.o my402list.o
	gcc -o warmup2 -g -pthread  warmup2.o my402list.o -lrt
warmup2.o: warmup2.c warmup2.h
	gcc -c -g -Wall -std=gnu99 -pthread -D_POSIX_PTHREAD_SEMANTICS warmup2.c 
my402list.o: my402list.c my402list.h
	gcc -c -g -Wall -std=gnu99  my402list.c
clean:
	rm -f warmup2 warmup2.o my402list.o
