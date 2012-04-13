#ifndef ELEMENT_LIST_H
#define ELEMENT_LIST_H

struct listnode{
	struct listnode* next;
	Element* element;
};
typedef struct listnode Node;

struct linkedlist{
	Node* head;
	int size;
	Node* tail;
};
typedef struct linkedlist LinkedList;

struct nodelistnode{
	struct nodelistnode* next;
	unsigned int node;
};
typedef struct nodelistnode NodeListNode;

struct nodelinkedlist{
	NodeListNode* head;
	int size;
	NodeListNode* tail;
};
typedef struct nodelinkedlist NodeLinkedList;

void initLinkedList(LinkedList* list);
void addToList(LinkedList* list, Element* element);

void initNodeLinkedList(NodeLinkedList* list);
void addToNodeList(NodeLinkedList* list, unsigned int node);
void printNodeList(NodeLinkedList* list);

#endif
