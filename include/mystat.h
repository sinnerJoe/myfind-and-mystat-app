#include<stdio.h>
#include<string.h>
#include<dirent.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <my_utility.h>
#include <myprotocol.h>
#include <errno.h>
#ifndef MY_STAT
#define MY_STAT

int sock_to_child[2], sock_to_parent[2];

int timespec2str(char *buf, uint len, struct timespec *ts) {
    struct tm t;
    tzset();
    if (localtime_r(&(ts->tv_sec), &t) == NULL)
        return 1;
    strftime(buf, len, "%d-%m-%Y %T", &t);
    return 0;
}


void permissionsToStr(char * buff, struct stat * file_stat, 
                unsigned long read_f, unsigned long write_f, unsigned long execute_f)
{
    char read, write, execute;
    read = (file_stat->st_mode & read_f) > 0;
    write = (file_stat->st_mode & write_f) > 0;
    execute = (file_stat->st_mode & execute_f) > 0;
    buff[0] = '\0';
    if(read)
        strcat(buff, "read ");
    if(write)
        strcat(buff, "write ");
    if(execute)
        strcat(buff, "execute");
}

void receiveStats(int fd_in){
    char * buff;
    for(int i=0; i<10; i++){
        buff = receiveDataProtocol(fd_in);
        printf("%s", buff);
        free(buff);
    }
}

void sendStats(char * path, struct stat * stats, int fd_out){
    struct stat local_stat;
    if(stats == NULL){
        stats = &local_stat;
        lstat(path, stats);
    }
    char mess[256], buff[60];
    timespec2str(buff, 60, &( stats->st_atim));
    snprintf(mess, 256, "Ultimul acces: %s\n", buff);
    sendDataProtocol(fd_out, mess);
    timespec2str(buff, 60, &( stats->st_mtim));
    snprintf(mess, 256, "Ultima modificare: %s\n", buff);
    sendDataProtocol(fd_out, mess);
    timespec2str(buff, 60, &( stats->st_mtim));
    snprintf(mess, 256, "Ultima modificare de stare: %s\n", buff);
    sendDataProtocol(fd_out, mess);
    snprintf(mess, 256, "Dimensiunea: %u B\n",  stats->st_size);
    sendDataProtocol(fd_out, mess);
    snprintf(mess, 256, "ID proprietar: %u \n",  stats->st_uid);
    sendDataProtocol(fd_out, mess);
    permissionsToStr(buff, stats, S_IRUSR, S_IWUSR, S_IXUSR);
    snprintf(mess, 256, "Permisiuni citire(owner): %s \n", buff);
    sendDataProtocol(fd_out, mess);
    permissionsToStr(buff, stats, S_IROTH, S_IWOTH, S_IXOTH);
    snprintf(mess, 256, "Permisiuni citire(others): %s \n", buff);
    sendDataProtocol(fd_out, mess);
    snprintf(mess, 256, "Numar de link-uri catre fisier: %u \n",  stats->st_nlink);
    sendDataProtocol(fd_out, mess);
    snprintf(mess, 256, "Numar de blocuri alocate: %u \n",  stats->st_blocks);
    sendDataProtocol(fd_out, mess);
    snprintf(mess, 256, "Dimensiunea blocurilor: %u B\n",  stats->st_blksize/8);
    sendDataProtocol(fd_out, mess);
}


int mystat_child(){
    char * path = receiveDataProtocol(sock_to_child[0]);
    // printf("Received path: %s\n",path);
    FILE* check = fopen(path, "r");
    
    if(check == NULL){
        sendDataProtocol(sock_to_parent[1], "n");
        sendDataProtocol(sock_to_parent[1], "Error: File doesn't exist or you don't have the rights to access it.");
    }
    else{
        fclose(check);
        // printf("child valid file\n");
        sendDataProtocol(sock_to_parent[1], "y");

        sendStats(path, NULL, sock_to_parent[1]);
    }
}

int mystat_parent(){
    char * path = spacetok(NULL);
    path = removeQuotes(path);
    // printf("Parent path %s\n", path);
    sendDataProtocol(sock_to_child[1], path);
    char * response = receiveDataProtocol(sock_to_parent[0]);
    if(response[0] == 'y'){
        // printf("Yes branch\n");
        receiveStats(sock_to_parent[0]);

    }
    else if(response[0] == 'n'){
        free(response);
        response = receiveDataProtocol(sock_to_parent[0]);
        printf("%s\n", response);

    }
    else {
        printf("Error in child process!\n");
        exit(1);
    }
    free(response);
}
#endif