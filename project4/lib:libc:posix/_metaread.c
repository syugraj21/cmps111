#include <lib.h>
#define read	_read
#include <unistd.h>

PUBLIC ssize_t metaread(fd, buffer, nbytes)
int fd;
void *buffer;
size_t nbytes;
{
    /* metaread() is identical to read() except that 65 is the number
     in table.c corresponding to metaread for the vfs. */
    message m;
    
    m.m1_i1 = fd;
    m.m1_i2 = nbytes;
    m.m1_p1 = (char *) buffer;
    return(_syscall(VFS_PROC_NR, 65, &m));
}