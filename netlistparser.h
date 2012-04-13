#ifndef NETLIST_PARSER_H_
#define  NETLIST_PARSER_H_

#include "strmap.h"

#define RESISTOR 1
#define CURRENT_SOURCE 2
#define VOLTAGE_SOURCE 3
#define VCCS 4
#define CCCS 5
#define VCVS 6
#define CCVS 7
#define CAPACITOR 8
#define INDUCTOR 9
#define DIODE 10
#define MOSFET 11

#define MAX_LINE 500
#define MAX_STRING 16
#define MAX_ERROR_MESSAGE 70

#define MAX_ELEMENTS 100

typedef char bool;
#define TRUE 1
#define FALSE 0

//generic element struct, that must be cast to a more
//specific struct based on its type in order to be used.
typedef struct{
	char* name;
	unsigned char type;
} Element;

//an element with two nodes and a value
typedef struct{
	char* name;
	unsigned char type;
	unsigned int posNode;
	unsigned int negNode;
	double value;	
} TwoTerminalElement;

//a mostfet element
typedef struct{
	char* name;
	unsigned char type;
	unsigned int source;
	unsigned int drain;
	unsigned int gate;
	unsigned int body;
	char* modelName;
	double width;
	double length;
} Mosfet;

//an element with four nodes and a value
typedef struct{
	char* name;
	unsigned char type;
	unsigned int node0;
	unsigned int node1;
	unsigned int node2;
	unsigned int node3;
	double value;
} FourTerminalElement;


typedef struct{
	char* name;
	unsigned char type;
	unsigned int posNode;
	unsigned int negNode;
	char* modelName;	
} Diode;


void parseNetlist(FILE* file);
bool isDouble(char* value);
void throwError(const char* errorMessage, int lineNum);
double valueToDouble(char* value);
unsigned char getType(char typeCode);
unsigned int stringToNodeNum(char* node,  unsigned int* nodeHash, StrMap* nodeNameMap);
void printElement(Element* element);
void printList();

#endif
