#include <lib.h>
#define write	_write
#include <unistd.h>

PUBLIC ssize_t metawrite(fd, buffer, nbytes)
int fd;
void *buffer;
size_t nbytes;
{
    /* metawrite() is identical to write() except that 66 is the number
       in table.c corresponding to metawrite for the vfs. */
    message m;
    
    m.m1_i1 = fd;
    m.m1_i2 = nbytes;
    m.m1_p1 = (char *) buffer;
    return(_syscall(VFS_PROC_NR, 66, &m));
}
