#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

//copy regular file
static int copy_reg(char *src_path, char *dst_path)
{
    int src_fd = open(src_path, O_RDONLY);
    if(src_fd < 0)
    {
        fprintf(stderr,"cannot open %s for reading\n",src_path);
        return -1;
    }
    struct stat ss;
    if(fstat(src_fd, &ss))
    {
        fprintf(stderr,"cannot fstat %s\n",src_path);
        return -1;
    }
    if(ss.st_mode & S_IFMT != S_IFREG)
    {
        fprintf(stderr,"not regular file %s\n",src_path);
        return -1;
    }
    unsigned int BLKSIZE = ss.st_blksize;

    int dst_fd = 0;
    if((dst_fd = open(dst_path, O_WRONLY|O_CREAT|O_TRUNC, ss.st_mode)) == -1)
    {
        fprintf(stderr,"open %s error\n",dst_path);
        return -1;
    }
    struct stat ds;
    errno = 0;
    if(fstat(dst_fd, &ds))
    {
        if(errno != ENOENT)
        {
            fprintf(stderr,"cannot fstat %s\n",dst_path);
            return -1;
        }
    }
    else
    {
        if(ds.st_mode & S_IFMT != S_IFREG)
        {
            fprintf(stderr,"not regular file %s\n",dst_path);
            return -1;
        }
        if(ss.st_ino == ds.st_ino)
        {
            fprintf(stderr,"same ino\n");
            return -1;
        }
    }

    ssize_t n_read = 0;
    int last_write_made_hole = 0;
    char *zero = (char*)malloc(BLKSIZE);
    char *buf = (char*)malloc(BLKSIZE);
    if(!zero || !buf)
    {
        fprintf(stderr,"malloc error\n");
        return -1;
    }
    while((n_read = read(src_fd, buf, BLKSIZE)) > 0)
    {
        if(memcmp(zero,buf,n_read) == 0)
        {
            lseek(dst_fd,BLKSIZE,SEEK_CUR);
            last_write_made_hole = 1;
            continue;
        }
        if(write(dst_fd, buf, n_read) != n_read)
        {
            unlink(dst_path);
            fprintf(stderr,"write %s error\n",dst_path);
            return -1;
        }
        last_write_made_hole = 0;
    }
    if(last_write_made_hole)
    {
        if(write(dst_fd, "", 1) != 1 || ftruncate(dst_fd,ss.st_size) < 0)
        {
            fprintf(stderr, "write %s error", dst_path);
            return -1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    if(argc != 3)
    {
        fprintf(stderr,"copy regular file, usage: %s src_file dst_file\n", argv[0]);
        return -1;
    }
    return copy_reg(argv[1], argv[2]);
}
