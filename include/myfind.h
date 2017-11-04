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
#include <mystat.h>
#ifndef MY_FIND
#define MY_FIND



const char* fifo_to_child = "to_child";
const char* fifo_to_parent = "to_parent";

typedef struct{
    struct stat stats;
    char path[200]; 
} FileInfo;


FileInfo * filesFound = NULL;
short files_nr = 0;

char * concatPath(const char * path,const char * folder){
    int sz = strlen(path) + strlen(folder)+2;
    char * result = malloc(sz);
    result[0]='\0';
    strcat(result, path);
    strcat(result, "/");
    strcat(result, folder);
    return result;
}


void addFile(struct stat* another_one, char *pathToFile){
    if(files_nr == 0){
        filesFound = malloc(sizeof(FileInfo));    
    }
    else{
        filesFound = realloc(filesFound, sizeof(FileInfo) * (1 + files_nr));
    }
    strcpy(filesFound[files_nr].path, pathToFile);
    filesFound[files_nr].stats = *another_one;
    files_nr++;
}

void findFile(char * path, char * totalPath, const char* searchedFile, int depth){
    struct dirent *entry;
    DIR* dp;
    struct stat statbuf;
    if(depth == 0)
        return;

    if((dp = opendir(path)) == NULL){
        // printf("Cannot open directory\n");
        return;
    }

    chdir(path);
    while((entry = readdir(dp))!=NULL){
        lstat(entry->d_name, &statbuf);
        
        if(S_ISDIR(statbuf.st_mode)){
             if(strcmp(".",entry->d_name) == 0 ||
                strcmp("..",entry->d_name) == 0)
                continue;
            // printf("%s/\n",entry->d_name);
            char * newTotalPath = concatPath(totalPath, entry->d_name);
            findFile(entry->d_name, newTotalPath, searchedFile, depth-1);
            free(newTotalPath);
        }
        else if(strcmp(entry->d_name, searchedFile)==0 && S_ISREG(statbuf.st_mode)){
            char * result = concatPath(totalPath, searchedFile);
            // printf("File found: %s\n", result);
            addFile(&statbuf, result);
            free(result);
        }
        // else printf("%s:file\n",entry->d_name);
    }
    chdir("..");
    closedir(dp);

}

void listenAboutFilesFound(int fd_in){
    char * file_nr_s = receiveDataProtocol(fd_in);
    int files_found = atoi(file_nr_s);
    
    if(files_found == 0){
        char * mess = receiveDataProtocol(fd_in);
        printf("%s\n", mess);
        free(mess);
        return;
    }

    char * buff = receiveDataProtocol(fd_in);
    printf("%s", buff);
    free(buff);
    for(int i=0; i<files_found; i++){
        buff = receiveDataProtocol(fd_in);
        printf("%s\n", buff);
        free(buff);
        receiveStats(fd_in);
    }
}

int myfind_parent(){ // myfind [<depth>] <folder> <filename>
    char * depth_s = "5";
    if(word_count() == 4){
    depth_s = spacetok(NULL);
    }
    char * folder = spacetok(NULL);
    char * file = spacetok(NULL);
    int fd_out = open(fifo_to_child, O_WRONLY);
    int fd_in = open(fifo_to_parent, O_RDONLY);
    sendDataProtocol(fd_out, depth_s);
    sendDataProtocol(fd_out, removeQuotes(folder));
    sendDataProtocol(fd_out, removeQuotes(file));

    listenAboutFilesFound(fd_in);
    close(fd_in);
    close(fd_out);
    return 1;
}

void tellAboutFilesFound(int fd_out, char * filename, char * folder){
    int i = 0;
    int cache = files_nr;
    while(cache > 0){
        i++;
        cache/=10;
    }
    char * buff = calloc(i+1, 1); 
    sprintf(buff, "%i", files_nr);
    sendDataProtocol(fd_out, buff);
    free(buff);
    DIR* dp;
    if(files_nr == 0){
        if((dp = opendir(folder)) == NULL)
            sendDataProtocol(fd_out, 
                "Directory doesn't exist or you have no rights to view it");
        else
            sendDataProtocol(fd_out, "No files found.");
        

        return;
    }
    char mess[256];
    sprintf(mess, "File %s was found in following places: ", filename);
    sendDataProtocol(fd_out, mess);
    buff = malloc(60);
    for(int i = 0; i < files_nr; i++){
        snprintf(mess, 256, "\n>>>> %s:\n", filesFound[i].path);
        sendDataProtocol(fd_out, mess);
        sendStats(NULL, &(filesFound[i].stats), fd_out);
    }
    free(buff);
}

int myfind_child(){
    int fd_in = open(fifo_to_child, O_RDONLY);
    int fd_out = open(fifo_to_parent, O_WRONLY);
    char* depth_s = receiveDataProtocol(fd_in);
    char* folder = receiveDataProtocol(fd_in);
    char* filename = receiveDataProtocol(fd_in);

    int depth = atoi(depth_s);
    findFile(folder, folder, filename, depth);
    tellAboutFilesFound(fd_out, filename, folder);


    free(depth_s);
    free(folder);
    free(filename);
    close(fd_in);
    close(fd_out);
    
}
#endif