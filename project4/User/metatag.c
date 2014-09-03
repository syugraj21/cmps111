#include <lib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char **argv)
{
    int fd;
    size_t nbytes;
    void *buffer;
    
    /* For simplicity, we only allow exactly 3 arguments. */
    if (argc != 3) {
        printf("Missing parameters. We only allow 3 arguments.\n");
        exit(1);
    }
    else{
        /* The metadata message is taken from the third argument at 
           the command line. 1024 bytes is our max space to write. The
           second argument is the file which we need to save the file
           descriptor for. */
        buffer = argv[2];
        nbytes = 1024;
        
        /* We need to check if the data is its too large and if we can 
           open the file or not. */
        if (strlen(argv[2]) > 1024) {
            fprintf(stderr, "Message size too large\n");
            exit(1);
        }
        fd = open(argv[1], O_RDWR);
        if (fd == -1) {
            fprintf(stderr, "Error with opening file\n");
            exit(1);
        }
    }
    
    metawrite(fd, buffer, nbytes);
}