#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "netlistparser.h"
#include "LinkedList.h"
#include <math.h>
#include "sparse/src/spMatrix.h" 

#define M_PI 3.1415926

#define spREAL double


/**
 * Greg Sterin and Quincy Wu EE415 Lab 1
 * Main file for parsing netlist
 */

LinkedList* elementList;
NodeLinkedList* nodeList;

//decodes the command arguments and calls appropriate functions
int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "error: No netlist file specified\n");
		return 1;
	}

	FILE *file = fopen(argv[1], "r");
	if(file == NULL){
		fprintf(stderr, "error: %s not found\n", argv[1]);
		return 1;
	}

	elementList = (LinkedList*)malloc(sizeof(LinkedList));
	initLinkedList(elementList);
	
	nodeList = (NodeLinkedList*)malloc(sizeof(NodeLinkedList));
	initNodeLinkedList(nodeList);
	
	parseNetlist(file);
	
	buildMatrix();
	
	fclose(file);

	//printList();
	//printNodeList(nodeList);

	return 0;
}



//parses the netlist
void parseNetlist(FILE* file){
	char line[MAX_LINE];
	int lineNum = 1;
	char* word;
	
	//allocate a hashtable for keeping track of already used names
	StrMap* nameMap = sm_new(MAX_ELEMENTS);
	
	//allocate a hashtable for mapping node name to a node number.
	StrMap* nodeNameMap = sm_new(4*MAX_ELEMENTS);
	
	while(fgets(line, MAX_LINE, file)){
		int lineLen = strlen(line);
		
		word = strtok(line, " \t\n");
		
		if(sm_exists(nameMap, word)){
			throwError("Reuse of element name", lineNum);
		}
		
		//this is the element word and also tells us what kind of element it is
		if(word != NULL && lineLen > 1){
			
			if(strlen(word) <= 1){
				throwError("No name given", lineNum);
			}
			
			//this will give us the element, based on what the element is we'll call other functions
			char elementChar = tolower(*word);	

			//create element with 2 nodes and a value
			if(elementChar == 'r' || elementChar == 'i' || elementChar == 'v' || elementChar == 'c' || elementChar == 'l'){
				//parse rest of line according to definition for 2 terminal device: word node node value				
				char* posNode = strtok(NULL, " \t\n");
				char* negNode = strtok(NULL, " \t\n");
				char* value = strtok(NULL, " \t\n");

				//if any of these are null, the netlist did not have enough args
				if(word == NULL || posNode == NULL || negNode == NULL || value == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				if(strtok(NULL, " \t\n") != NULL){
					throwError("Too many arguments given", lineNum);
				}

				//make sure we have a valid value
				if(!isDouble(value)){
					throwError("The value argument must be in scientific notation", lineNum);
				}

				//make sure that none of the strings are > MAX_STRING chars
				if(strlen(word) > MAX_STRING || strlen(posNode) > MAX_STRING || strlen(negNode) > MAX_STRING){
					throwError("all words must be less than 16 characters", lineNum);
				}
				
				//add name to the nameMap
				sm_put(nameMap, word, "T");
				
				//initialize the struct
				TwoTerminalElement* element = (TwoTerminalElement*)malloc(sizeof(TwoTerminalElement));
				char* wordPtr = (char*)malloc(strlen(word)*sizeof(char));
				element->name = strncpy(wordPtr, word, strlen(word));
				element->value = valueToDouble(value);
				element->posNode = stringToNodeNum(posNode ,nodeNameMap);
				element->negNode = stringToNodeNum(negNode, nodeNameMap);
				element->type = getType(elementChar);
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);

			//element is a mosfet
			}else if(elementChar == 'm'){
				char* source = strtok(NULL, " \t\n");
				char* drain = strtok(NULL, " \t\n");
				char* gate = strtok(NULL, " \t\n");
				char* body = strtok(NULL, " \t\n");
				char* model = strtok(NULL, " \t\n");
				char* length = strtok(NULL, " \t\n");
				char* width = strtok(NULL, " \t\n");
	
				//if any of these are null, the netlist did not have enough args
				if(word == NULL || source == NULL || drain == NULL || gate == NULL || body == NULL || model == NULL || length == NULL || width == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				if(strtok(NULL, " \t\n") != NULL){
					throwError("Too many arguments given", lineNum);
				}

				//make sure we have a valid length
				if(!isDouble(length)){
					throwError("The length argument must be in scientific notation", lineNum);
				}

				//make sure we have a valid width
				if(!isDouble(width)){
					throwError("The width argument must be in scientific notation", lineNum);
				}

				//make sure that none of the strings are > MAX_STRING chars
				if(strlen(word) > MAX_STRING || strlen(model) > MAX_STRING || strlen(source) > MAX_STRING || 
				strlen(drain) > MAX_STRING || strlen(model) > MAX_STRING || strlen(gate) > MAX_STRING || strlen(body) > MAX_STRING){
					throwError("all words must be less than 16 characters", lineNum);
				}

				//add name to the nameMap
				sm_put(nameMap, word, "T");

				//initialize the struct.
				Mosfet* element = (Mosfet*)malloc(sizeof(Mosfet));
				char* wordPtr = (char*)malloc(strlen(word)*sizeof(char));
				element->name= strncpy(wordPtr, word, strlen(word));
				char* modelPtr = (char*)malloc(strlen(model)*sizeof(char));
				element->modelName = strncpy(modelPtr, model, strlen(model));
				element->type = getType(elementChar);
				element->source = stringToNodeNum(source, nodeNameMap);
				element->drain = stringToNodeNum(drain, nodeNameMap);
				element->gate = stringToNodeNum(gate, nodeNameMap);
				element->body = stringToNodeNum(body, nodeNameMap);
				element->length = valueToDouble(length);
				element->width = valueToDouble(width);
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);


			//element has four nodes and a value
			}else if(elementChar == 'g' || elementChar == 'f' || elementChar == 'e' || elementChar == 'h'){			
				char* node0 = strtok(NULL, " \t\n");
				char* node1 = strtok(NULL, " \t\n");
				char* node2 = strtok(NULL, " \t\n");
				char* node3 = strtok(NULL, " \t\n");
				char* value = strtok(NULL, " \t\n");

				//if any of these are null, the netlist did not have enough args
				if(word == NULL || node0 == NULL  || node1 == NULL || node2 == NULL || node3 == NULL ||value == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				if(strtok(NULL, " \t\n") != NULL){
					throwError("Too many arguments given", lineNum);
				}

				//make sure we have a valid value
				if(!isDouble(value)){
					throwError("The value argument must be in scientific notation", lineNum);
				}

				//make sure that none of the strings are > MAX_STRING chars
				if(strlen(word) > MAX_STRING || strlen(node0) > MAX_STRING || strlen(node1) > MAX_STRING ||  strlen(node2) > MAX_STRING ||  strlen(node3) > MAX_STRING){
					throwError("all words must be less than 16 characters", lineNum);
				}

				//add name to the nameMap
				sm_put(nameMap, word, "T");

				FourTerminalElement* element = (FourTerminalElement*)malloc(sizeof(FourTerminalElement));
				char* wordPtr = (char*)malloc(strlen(word)*sizeof(char));
				element->name = strncpy(wordPtr, word, strlen(word));
				element->value = valueToDouble(value);
				element->node0 = stringToNodeNum(node0, nodeNameMap);
				element->node1 = stringToNodeNum(node1, nodeNameMap);
				element->node2 = stringToNodeNum(node2, nodeNameMap);
				element->node3 = stringToNodeNum(node3, nodeNameMap);
				element->type = getType(elementChar);
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);

			}else if(elementChar == 'd'){
				//parse rest of line according to definition for 2 terminal device: word node node model_name				
				char* posNode = strtok(NULL, " \t\n");
				char* negNode = strtok(NULL, " \t\n");
				char* model = strtok(NULL, " \t\n");
				double value = 1;

				//if any of these are null, the netlist did not have enough args
				if(word == NULL || posNode == NULL || negNode == NULL || model == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				char* extras = strtok(NULL, " \t\n");
				if(extras != NULL && (*extras != '\n')){
					//optional value param found
					if(!isDouble(extras)){
						throwError("The value argument must be in scientific notation", lineNum);
					}else{
						value = valueToDouble(extras);
					}

					//more that the allows args are present
					extras = strtok(NULL, " \t\n");
					if(extras != NULL && (*extras != '\n')){
						throwError("Too many arguments given", lineNum);
					}
				}

				//make sure that none of the strings are > MAX_STRING chars
				if(strlen(word) > MAX_STRING || strlen(posNode) > MAX_STRING || strlen(negNode) > MAX_STRING){
					throwError("all words must be less than 16 characters", lineNum);
				}
				
				//add name to the nameMap
				sm_put(nameMap, word, "T");
				
				//initialize the struct
				Diode* element = (Diode*)malloc(sizeof(Diode));
				char* wordPtr = (char*)malloc(strlen(word)*sizeof(char));
				element->name = strncpy(wordPtr, word, strlen(word));
				char* modelPtr = (char*)malloc(strlen(model)*sizeof(char));
				element->modelName = strncpy(modelPtr, model, strlen(model));
				element->posNode = stringToNodeNum(posNode, nodeNameMap);
				element->negNode = stringToNodeNum(negNode, nodeNameMap);
				element->type = getType(elementChar);
				element->value = value;
				
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);
			}else{
				throwError("Invalid element declaration", lineNum);
			}
		}
		lineNum++;
	}
	sm_delete(nameMap);
	sm_delete(nodeNameMap);
}




void throwError(const char* errorMessage, int lineNum){
	printf("ERROR: %s at line %d", errorMessage, lineNum);
	//free all allocated memory.
	printf("\n");
	exit(1);
}	

bool isDouble(char* value){
	if(atof(value) > 0){
		return TRUE;
	}else if(strtok(value, "e") != NULL && strtok(NULL, "e") != NULL){
		return TRUE;
	}else{
		return FALSE;
	}
}	

double valueToDouble(char* value){
	if(atof(value) > 0){
		return atof(value);
	}else{
		//there is some scientific notation AKA an eNUM following some number, or it is zero
		char* baseString = strtok(value, "e");
		if(baseString != NULL){
			double returnVal = atof(baseString);
			char* power = strtok(NULL, "e");
			if(power != NULL){
				returnVal *= 10^(atoi(power));
			}else{
				return 0;
			}
		}else{
			return 0;		
		}
	}
}

unsigned char getType(char typeCode){
	switch(typeCode){
		case 'r':
			return RESISTOR;
			break;
		case 'i':
			return CURRENT_SOURCE;
			break;
		case 'v':
			return VOLTAGE_SOURCE;
			break;	
		case 'g':
			return VCCS;
			break;
		case 'f':
			return CCCS;
			break;
		case 'e':
			return VCVS;
			break;
		case 'h':
			return CCVS;
			break;
		case 'c':
			return CAPACITOR;
			break;
		case 'l':
			return INDUCTOR;
			break;
		case 'd':
			return DIODE;
			break;
		case 'm':
			return MOSFET;
			break;
		default:
			return 0;
			break;
	}
}

unsigned int stringToNodeNum(char* node, StrMap* nodeNameMap){
	static unsigned int freeNode = 0;
	char* out_buff = (char*)malloc(4*sizeof(char));
	if(sm_exists(nodeNameMap, node)){
		sm_get(nodeNameMap, node, out_buff, 4);
		return atoi(out_buff);
	}else{
		
		sprintf(out_buff, "%d", freeNode);
		sm_put(nodeNameMap, node, out_buff);
		addToNodeList(nodeList, freeNode);	
		return freeNode++; 
	}
	free(out_buff);
}

void printElement(Element* element){
	printf("type: %d\n", element->type);

	if(element->type == RESISTOR || element->type == CURRENT_SOURCE || element->type == CAPACITOR || element->type == INDUCTOR || element->type == VOLTAGE_SOURCE){
		TwoTerminalElement* myElement = (TwoTerminalElement*) element;
		printf("name: %s \n posNode: %d \n negNode: %d \n value: %f", myElement->name, myElement->posNode, myElement->negNode, myElement->value);

	}else if(element->type == CCVS || element->type == CCCS || element->type == VCVS || element->type == VCCS){
		FourTerminalElement* myElement = (FourTerminalElement*) element;
		printf("name: %s \n node0: %d \n node1: %d \n node2: %d \n node3: %d \n value: %f", myElement->name, myElement->node0, myElement->node1, myElement->node2, myElement->node3, myElement->value);

	}else if(element->type == DIODE){
		Diode* myElement = (Diode*) element;		
		printf("name: %s \n posNode: %d \n negNode: %d \n modelName: %s value: %f\n", myElement->name, myElement->posNode, myElement->negNode, myElement->modelName, myElement->value);

	}else if(element->type == MOSFET){
		Mosfet* myElement = (Mosfet*) element;
		printf("name: %s \n source: %d \n drain: %d \n gate: %d \n body: %d \n length: %f \n width: %f",
				myElement->name, myElement->source, myElement->drain, myElement->gate, myElement->body, myElement->length, myElement->width
		);
	}
	printf("\n\n");
}


void printList(){
	if(elementList != NULL){
		Node* current = elementList->head;
		while(current != NULL){
			printElement(current->element);
			current = current->next;
		}
	}
}	

void buildMatrix(){
	struct complex{
		double re;
		double im;
	} x[3],b[3];

	double f,omega;

	spMatrix A;
	spError err;
	struct spTemplate Stamp[3];
	
	A = spCreate(0, 1, &err);
	
	spGetAdmittance(A,1,0, &Stamp[0]);
    spGetAdmittance(A,1,2, &Stamp[1]);
    spGetAdmittance(A,0,3, &Stamp[2]);
    
    b[0].re=0.0; b[0].im=0.0;
    b[1].re=1.0; b[1].im=0.0;
    b[2].re=0.0; b[2].im=0.0;
    
    spADD_REAL_QUAD(Stamp[0],1.0);
    spADD_REAL_QUAD(Stamp[1],1.0);
    spADD_REAL_QUAD(Stamp[2],2.0);
    
    spPrint(A, 1, 1, 1);
    
    err=spFactor(A);
    spSolve(A, (spREAL *)b, (spREAL *)x);
    
    printf("x0=%6.2e\n",x[0].re);
    printf("x1=%6.2e\n",x[1].re);
    printf("x2=%6.2e\n",x[2].re);
}
