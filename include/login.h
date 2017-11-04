#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <dirent.h>
#include <myprotocol.h>
#include <my_utility.h>
#ifndef LOGIN
#define LOGIN
const char* logins_file = "logins.txt";
int pipe_to_child[2], pipe_to_parent[2];
int user_logged_in = 0; 

char isUserLogged(){
    if(!user_logged_in){
        printf("Nu sunteti logat! Logati-va cu comanda \"login <username>\".\n");
    }
    return user_logged_in;
}

void free_double_pointer(char ** ch){
    for(int i=0; ch[i]!= NULL; i++)
        free(ch[i]);
    free(ch);
}

int countLogins(){
    FILE* file = fopen(logins_file, "r");
    char ch;
    int lines=0;
    while(!feof(file))
    {
        ch = fgetc(file);
        if(ch == '\n')
        {
            lines++;
        }
    }
    fclose(file);
    return lines + 1;
}

char** getLogins(){
    
    FILE* file = fopen(logins_file, "r");
    
    int lines_nr = countLogins();
    char ** logins = (char**)malloc(sizeof(char*) * (lines_nr+1));
    logins[lines_nr] == NULL;
    for(int i=0; i<lines_nr; i++){
        char login[64];
        fscanf(file, "%s", login);
        logins[i] = (char *)malloc(sizeof(char) * strlen(login));
        strcpy(logins[i], login);
    }
    fclose(file);
    return logins;
}



char isValidLogin( char ** all_logins,const char * login){
    int i=0;
    while(all_logins[i] != NULL){
        if(strcmp(login, all_logins[i])==0){
            free_double_pointer(all_logins);
            return 1;
        }
    i++;
    }
    free_double_pointer(all_logins);
    return 0;
}

int login_parent(){
    char *login_name = spacetok(NULL);
    // printf("String read: %s\n", login_name);
    sendDataProtocol(pipe_to_child[1], login_name);
    char * mess_from_child = receiveDataProtocol(pipe_to_parent[0]);
    printf("%s\n", mess_from_child);
    mess_from_child = receiveDataProtocol(pipe_to_parent[0]);
    if(mess_from_child[0]=='y'){
        user_logged_in = 1;
    }
    free(mess_from_child);
}

int login_child(){
    char * arg = receiveDataProtocol(pipe_to_child[0]);
    printf("After receive");
    char ** logins = getLogins();
    
    if(isValidLogin(logins, arg)){
        sendDataProtocol(pipe_to_parent[1],"Succesful log in.");
        sendDataProtocol(pipe_to_parent[1],"y");
    }
    else
    {
        sendDataProtocol(pipe_to_parent[1],"Unsuccesful attempt to log in!");
        sendDataProtocol(pipe_to_parent[1],"n");
    }
    free_double_pointer(logins);
    free(arg);
}

#endif

