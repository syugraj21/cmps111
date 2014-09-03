/* Justin Kwok, Ian Hamilton, Yugraj Singh, Ben Lieu, Greg Arnheiter */
/* Created 5/20/14 */

#include "linked_list.h"
#include "slug_malloc.h"

#define TOO_BIG 134217728


/* Global Functions */

List* hash_table = NULL;
int total_allocations = 0;
int current_allocations = 0;
size_t total_size = 0;
size_t current_size = 0;
double mean = 0;

/* Insert function for the Hash-Table */
void hash_insert(void *address, size_t size, unsigned timestamp, char *location){
    int index, result;
    index = ((int)address) % 61;    
    total_allocations++;
    
    current_allocations++;
    total_size += size;
    current_size += size;
    calculate_mean();
    
    result = insert_node(hash_table[index], address, size, timestamp, location, mean);
}

/* Remove function for Hash-Table */
void hash_remove(void *address, char *WHERE) {
    int index, result;
    index = ((int) address) % 61;
    result = remove_node(hash_table[index], address);
    if(result == -1) {
        fprintf(stderr, "Error %s: Attempt to free memory at this location is invalid.\n", WHERE);
        exit(1);
    }
    current_allocations--;
    current_size -= result;
}

/* Function to calculate the mean */
void calculate_mean() {
    mean = (double)total_size/(double)total_allocations;
}

/* Slug_malloc overrides default malloc function */
void *slug_malloc(size_t size, char *WHERE) {
    int i;
    void *address;
    struct timeval time;
    unsigned timestamp;
    if(hash_table == NULL) {
        hash_table = (List *)malloc(61 * sizeof(List));
        for(i = 0; i < 61; i++)
            hash_table[i] = newlist();
        atexit(slug_memstats);
    }
    
    /* If desired memory allocation is larger than 128MB */
    if(size > TOO_BIG) {
        fprintf(stderr, "%s\n", "Size Allocation is too big");
        exit(1);
    }
    
    /* Print warning message for memory allocation of size 0 */
    if(size == 0) 
        fprintf(stderr, "%s\n", "Unusual Behavior: Attempt to allocate memory of size 0");
    
    address = malloc(size);
    gettimeofday(&time, NULL);
    timestamp = time.tv_sec;
    hash_insert(address, size, timestamp, WHERE);
    return address;
}

/* Slug_free overrides default free function */
void slug_free(void *addr, char *WHERE) {
    if(!hash_table) {
        fprintf(stderr, "%s\n", "Error: Malloc was never called.");
        exit(1);
    }
    hash_remove(addr, WHERE);
}

/* Prints memory data when program exits */
void slug_memstats(void) {
    int i;
    printf("\n********** Memory Summary **********\n");
        if(current_allocations > 0) {
            printf("There are still memory allocations that were not freed\n");
            
            for(i = 0; i < 61; i++) {
                print_list(hash_table[i]);
            }
        }
        calculate_mean();
        printf("\nTotal Number of Allocations: %d\n", total_allocations);
        printf("Current Number of Allocations: %d\n", current_allocations);
        printf("Amount of Memory Allocated - Total: %d\n", total_size);
        printf("Amount of Memory Allocated - Current: %d\n", current_size);
        printf("Total Amount of Memory - Mean: %f\n", mean);
        printf("Total Amount of Memory - Standard Deviation: %f\n", get_std_dev());
        
        /*Free all leftover memory */
        if(hash_table) {
            for(i = 0; i < 61; i++) {
                if(hash_table[i]) {
                    free_List(hash_table[i]);
                    free(hash_table[i]);
                }
                hash_table[i] = NULL;
            }
            free(hash_table);
            hash_table = NULL;
        }
}