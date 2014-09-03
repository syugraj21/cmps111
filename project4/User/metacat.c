#include <lib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
    int fd; 
    size_t nbytes;
    void *buffer;
    int size;
    int i =0;
    
    /* For simplicity, we only allow exactly 2 arguments. */
    if (argc != 2) {
        printf("Missing parameters. We only allow 2 arguments.\n");
        exit(1);
    }
    else{
        /* We need to save the file descriptor when we open the file,
           which is the argument immediatley after the metacat call.
           Our max amount of bytes to be used in a message is 1024 
           bytes and this buffer will be for the meatdat in userspace.*/
        fd = open(argv[1], O_RDWR);
        
        /* Error if we cant open the file. */
        if (fd == -1) {
            fprintf(stderr, "Error with opening file\n");
            exit(1);
        }
        nbytes = 1024;
        buffer = (void*) malloc(nbytes);
    }
    
    metaread(fd, buffer, nbytes);
    
    printf("%s\n", buffer); /* Print whats on the buffer after meatread
                               puts (meta) data on it. */

}