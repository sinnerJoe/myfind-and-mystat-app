#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#ifndef MY_PROTOCOL
#define MY_PROTOCOL

int sendDataProtocol(int fd, const char* data){
    
    unsigned short sz = strlen(data);
    write(fd, &sz, 2);
    if(sz)
        dprintf(fd, "%s",  data);

}

char * receiveDataProtocol(int fd){
    unsigned short sz;
    read(fd, &sz, 2);
    if(sz>0){
        char * buffer = (char*)calloc(sz+1,1);
        read(fd, buffer, sz);
        // printf("Buffer:%s\nsz=%i\n", buffer, sz);
        return buffer;
    }
    else{
        return NULL;
    }
}
#endif