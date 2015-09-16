warmup2: warmup2.o my402list.o
	gcc -o warmup2 -g -pthread  warmup2.o my402list.o  token-core.o token-packet.o  helper_function.o token-arrival.o token-server.o monitor.o -lrt
warmup2.o: warmup2.c warmup2.h
	gcc -c -g -Wall -std=gnu99 -pthread -D_POSIX_PTHREAD_SEMANTICS warmup2.c 
helper_function:
	gcc -g -c -Wall -pthread helper_function.c
token-token-arrival:
	gcc -g -c -Wall -pthread token-arrival.c
token-server:
	gcc -g -c -Wall -pthread token-server.c
monitor:
	gcc -g -c -Wall -pthread monitor.c
my402list.o: my402list.c my402list.h
	gcc -c -g -Wall -std=gnu99  my402list.c
clean:
	rm -f warmup2 *.o
