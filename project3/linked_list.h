/* Justin Kwok, Ian Hamilton, Yugraj Singh, Ben Lieu, Greg Arnheiter */
/* Created 5/20/14 */

#include <stdio.h>
#include <stdlib.h>

typedef struct linked_list* List;

List newlist(void);

void free_List(List L);

int insert_node(List L, void *a, size_t b, unsigned c, char *d, double mean);

size_t remove_node(List L, void *input);

void print_list(List L);

int find_size(List L);

double get_std_dev();
