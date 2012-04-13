#Greg Sterin  and Quincy Wu
#EE415 SPICE SIMULATOR MAKEFILE

netlistparser: netlistparser.o LinkedList.o strmap.o
	gcc -Wall -g -o netlistparser netlistparser.o LinkedList.o strmap.o
	
netlistparser.o: netlistparser.c strmap.h netlistparser.h LinkedList.h
	gcc -Wall -g -c netlistparser.c
	
LinkedList.o: LinkedList.c LinkedList.h netlistparser.h
	gcc -Wall -g -c LinkedList.c

strmap.o: strmap.c strmap.h
	gcc -Wall -g -c strmap.c
