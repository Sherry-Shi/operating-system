Documentation for Warmup Assignment 1
=====================================

+-------+
| BUILD |
+-------+
warmup2: warmup2.o my402list.o
	gcc -o warmup2 -g -pthread  warmup2.o my402list.o -lrt
warmup2.o: warmup2.c warmup2.h
	gcc -c -g -Wall -std=gnu99 -pthread -D_POSIX_PTHREAD_SEMANTICS warmup2.c 
my402list.o: my402list.c my402list.h
	gcc -c -g -Wall -std=gnu99  my402list.c
clean:
	rm -f warmup2 warmup2.o my402list.o

Comments: make command would create the executable file warmup2 and two link file warmup2.o, my402list.o. make clean command would remove warmup2, warmup2.o and my402list.o.

+---------+
| GRADING |
+---------+

Basic running of the code : 100pts

Missing required section(s) in README file : 
Cannot compile : 
Compiler warnings : no warnings
"make clean" : 
Segmentation faults : no seg faults
Separate compilation : 
Using busy-wait : 
Handling of commandline arguments:
    1) -n : 
    2) -lambda : 
    3) -mu : 
    4) -r : 
    5) -B : 
    6) -P : 
Trace output :
    1) regular packets: no error
    2) dropped packets: no error
    3) removed packets: no error
    4) token arrival (dropped or not dropped): no error
Statistics output :
    1) inter-arrival time : 
    2) service time : 
    3) number of customers in Q1 : 
    4) number of customers in Q2 : 
    5) number of customers at a server : 
    6) time in system : 
    7) standard deviation for time in system : 
    8) drop probability : 
Output bad format : 
Output wrong precision for statistics (should be 6-8 significant digits) : 
Large service time test : 
Large inter-arrival time test : 
Tiny inter-arrival time test : 
Tiny service time test : 
Large total number of customers test : 
Large total number of customers with high arrival rate test : 
Dropped tokens test : 
Cannot handle <Cntrl+C> at all (ignored or no statistics) : 
Can handle <Cntrl+C> but statistics way off : 
Not using condition variables and do some kind of busy-wait : 
Synchronization check : 
Deadlocks : no deadlocks

+------+
| BUGS |
+------+

Comments: no bugs found

+-------+
| OTHER |
+-------+

Comments on design decisions: 
Comments on deviation from spec: 