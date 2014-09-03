/* Justin Kwok, Ian Hamilton, Yugraj Singh, Ben Lieu, Greg Arnheiter */
/* Created 5/20/14 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "linked_list.h"

/* Global Variables */
int running_std_dev = 0;

/* Structs */
typedef struct Node {
    void *address;
    size_t length;
    unsigned timestamp;
    char *location;
    struct Node* next;
} Node;

typedef Node* Node_ref;

typedef struct linked_list {
    Node_ref head;
    Node_ref tail;
    int length;
} linked_list;

/* Constructors & Destructors */
Node_ref new_node(void *a, size_t b, unsigned c, char *d) {    
    Node_ref my_node = malloc(sizeof(Node));
    my_node->address = a;
    my_node->length = b;
    my_node->timestamp = c;
    my_node->location = d;
    return my_node;
}

List newlist(void) {
    List my_list = malloc(sizeof(linked_list));
    my_list->head = my_list->tail = NULL;
    my_list->length = 0;
    return my_list;
}

void free_List(List L) {    
    Node_ref my_node, temp;
    my_node = L->head;
    while(my_node) {
        temp = my_node;
        my_node = my_node->next;
        if(temp->address)
            free(temp->address);
        if(temp)
            free(temp);
    }
    my_node = NULL;
}

/* Insert node function */
int insert_node(List L, void *a, size_t b, unsigned c, char *d, double mean) {
    Node_ref my_node;
    if(!L)
        return -1;
    my_node = new_node(a,b,c,d);
    
    if(L->length == 0) 
        L->head = L->tail = my_node;
    else {
        L->tail->next = my_node;
        L->tail = my_node;
    }
    L->length++;
    
    running_std_dev = ((b - mean) * (b - mean));
    return 0;
}

/* Remove node function */
size_t remove_node(List L, void *input) {
    Node_ref temp_head, prev;
    size_t return_size;
    if(!L)
        return -1;
    temp_head = L->head;
    if(!temp_head)
        return -1;
    
    if(temp_head->address == input) {
        return_size = temp_head->length;
        free(temp_head->address);
        free(temp_head);
        temp_head = NULL;
        L->length--;
        L->head = NULL;
        return return_size;
    }
    
    prev = temp_head;
    temp_head = temp_head->next;
    while(temp_head) {
        if(temp_head->address == input) {
            return_size = temp_head->length;
            prev->next = temp_head->next;
            free(temp_head->address);
            free(temp_head);
            temp_head = NULL;
            L->length--;
            return return_size;
        }
        prev = temp_head;
        temp_head = temp_head->next;
    } 
    return -1;
}

/* Get size of list */
int find_size(List L) {
    return L->length;
}

/* Print list */
void print_list(List L) {
    Node_ref current = L->head;
    while(current) {        
        printf("Memory Address: %p\n", current->address);
        printf("Size of malloc: %d\n", current->length);
        printf("Time stamp: %d\n", current->timestamp);
        printf("Location: %s\n\n", current->location);
        
        current = current->next;
    }
}

/* Get standard deviation */
double get_std_dev() {
    return sqrt(running_std_dev);
}