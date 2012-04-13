#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "netlistparser.h"
#include "LinkedList.h"


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
	
	fclose(file);

	printList();
	printNodeList(nodeList);

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
	
	//create an array that we will use for hashing the nodes
	unsigned int* nodeHash = (unsigned int*)malloc((4 * MAX_ELEMENTS)*sizeof(unsigned int));
	//initialize the hash with all zero
	int i;
	for(i = 0; i < 4*MAX_ELEMENTS; i++){
		nodeHash[i] = 0;
	}
	
	
	while(fgets(line, MAX_LINE, file)){
		int lineLen = strlen(line);
		
		word = strtok(line, " ");
		
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
				char* posNode = strtok(NULL, " ");
				char* negNode = strtok(NULL, " ");
				char* value = strtok(NULL, " ");

				//if any of these are null, the netlist did not have enough args
				if(word == NULL || posNode == NULL || negNode == NULL || value == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				if(strtok(NULL, " ") != NULL){
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
				element->posNode = stringToNodeNum(posNode, nodeHash, nodeNameMap);
				element->negNode = stringToNodeNum(negNode, nodeHash, nodeNameMap);
				element->type = getType(elementChar);
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);

			//element is a mosfet
			}else if(elementChar == 'm'){
				char* source = strtok(NULL, " ");
				char* drain = strtok(NULL, " ");
				char* gate = strtok(NULL, " ");
				char* body = strtok(NULL, " ");
				char* model = strtok(NULL, " ");
				char* length = strtok(NULL, " ");
				char* width = strtok(NULL, " ");
	
				//if any of these are null, the netlist did not have enough args
				if(word == NULL || source == NULL || drain == NULL || gate == NULL || body == NULL || model == NULL || length == NULL || width == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				if(strtok(NULL, " ") != NULL){
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
				element->source = stringToNodeNum(source, nodeHash, nodeNameMap);
				element->drain = stringToNodeNum(drain, nodeHash, nodeNameMap);
				element->gate = stringToNodeNum(gate, nodeHash, nodeNameMap);
				element->body = stringToNodeNum(body, nodeHash, nodeNameMap);
				element->length = valueToDouble(length);
				element->width = valueToDouble(width);
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);


			//element has four nodes and a value
			}else if(elementChar == 'g' || elementChar == 'f' || elementChar == 'e' || elementChar == 'h'){			
				char* node0 = strtok(NULL, " ");
				char* node1 = strtok(NULL, " ");
				char* node2 = strtok(NULL, " ");
				char* node3 = strtok(NULL, " ");
				char* value = strtok(NULL, " ");

				//if any of these are null, the netlist did not have enough args
				if(word == NULL || node0 == NULL  || node1 == NULL || node2 == NULL || node3 == NULL ||value == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				if(strtok(NULL, " ") != NULL){
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
				element->node0 = stringToNodeNum(node0, nodeHash, nodeNameMap);
				element->node1 = stringToNodeNum(node1, nodeHash, nodeNameMap);
				element->node2 = stringToNodeNum(node2, nodeHash, nodeNameMap);
				element->node3 = stringToNodeNum(node3, nodeHash, nodeNameMap);
				element->type = getType(elementChar);
				addToList(elementList, (Element*)element);
				//printElement((Element*)element);

			}else if(elementChar == 'd'){
				//parse rest of line according to definition for 2 terminal device: word node node model_name				
				char* posNode = strtok(NULL, " ");
				char* negNode = strtok(NULL, " ");
				char* model = strtok(NULL, " ");

				//if any of these are null, the netlist did not have enough args
				if(word == NULL || posNode == NULL || negNode == NULL || model == NULL){
					throwError("Not enough parameters have been given", lineNum);
				}
				//if there is more tokens on line, netlist had too many args
				char* extras = strtok(NULL, " ");
				if(extras != NULL && (*extras != '\n')){
					throwError("Too many arguments given", lineNum);
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
				element->posNode = stringToNodeNum(posNode, nodeHash, nodeNameMap);
				element->negNode = stringToNodeNum(negNode, nodeHash, nodeNameMap);
				element->type = getType(elementChar);
				
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
	free(nodeHash);
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

unsigned int stringToNodeNum(char* node, unsigned int* nodeHash, StrMap* nodeNameMap){
	int nodeNum = atoi(node);
	if(nodeNum > 0 || *node == '0'){
		if(nodeHash[nodeNum] == 0){
			addToNodeList(nodeList, nodeNum);
		}
		
		nodeHash[nodeNum] = 1;
		return nodeNum;
	}else{
		char* out_buff = (char*)malloc(4*sizeof(char));
		if(sm_exists(nodeNameMap, node)){
			sm_get(nodeNameMap, node, out_buff, 4);
			return atoi(out_buff);
		}else{
			int i = 0;
			while(nodeHash[i] == 1 && i < 4*MAX_ELEMENTS){
				i++;
			}
			if(i < 4*MAX_ELEMENTS){
				sprintf(out_buff, "%d", i);
				nodeHash[i] = 1;
				sm_put(nodeNameMap, node, out_buff);
				addToNodeList(nodeList, i);
				return i; 
			}else{
				throwError("Node Overflow", 0);
			}	
		}
		free(out_buff);
	}
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
		printf("name: %s \n posNode: %d \n negNode: %d \n modelName: %s \n", myElement->name, myElement->posNode, myElement->negNode, myElement->modelName);

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
