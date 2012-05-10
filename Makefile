#Greg Sterin  and Quincy Wu
#EE415 SPICE SIMULATOR MAKEFILE

OFILES = sparse/src/spAllocate.o sparse/src/spBuild.o sparse/src/spFactor.o sparse/src/spOutput.o sparse/src/spSolve.o sparse/src/spUtils.o sparse/src/spFortran.o

netlistparser: netlistparser.o LinkedList.o strmap.o $(OFILES)
	gcc -Wall -g -o netlistparser netlistparser.o LinkedList.o strmap.o $(OFILES)
	
netlistparser.o: netlistparser.c strmap.h netlistparser.h LinkedList.h sparse/src/spMatrix.h
	gcc -Wall -g -c netlistparser.c
	
LinkedList.o: LinkedList.c LinkedList.h netlistparser.h
	gcc -Wall -g -c LinkedList.c

strmap.o: strmap.c strmap.h
	gcc -Wall -g -c strmap.c
