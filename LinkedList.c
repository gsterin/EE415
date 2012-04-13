#include <stdlib.h>
#include <stdio.h>
#include "netlistparser.h"
#include "LinkedList.h"


//initialize the linked list
void initLinkedList(LinkedList* list){
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void addToList(LinkedList* list, Element* element){
	Node* elementNode = (Node*)malloc(sizeof(Node));
	elementNode->element = element;
	elementNode->next = NULL;
	if(list->size == 0){
		list->head = elementNode;
		list->tail = elementNode;
	}else{
		list->tail->next = elementNode;
		list->tail = elementNode;
	}
	(list->size)++;
}

//initialize the linked list
void initNodeLinkedList(NodeLinkedList* list){
	list->head = NULL;
	list->tail = NULL;
	list->size = 0;
}

void addToNodeList(NodeLinkedList* list, unsigned int node){
	NodeListNode* elementNode = (NodeListNode*)malloc(sizeof(NodeListNode));
	elementNode->node = node;
	elementNode->next = NULL;
	if(list->size == 0){
		list->head = elementNode;
		list->tail = elementNode;
	}else{
		list->tail->next = elementNode;
		list->tail = elementNode;
	}
	(list->size)++;
}

void printNodeList(NodeLinkedList* list){ 
	printf("[");
	if(list != NULL){
		NodeListNode* current = list->head;
		while(current != NULL){
			printf("%d, ", current->node);
			current = current->next;
		}
	}
	printf("]");
}
