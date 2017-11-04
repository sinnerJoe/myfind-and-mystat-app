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
#include <signal.h>
#include <myprotocol.h>
#include <myfind.h>
#include <login.h>
#include <my_utility.h>
#include <mystat.h>






void sig_exit(int dummy){
    remove(fifo_to_child);
    remove(fifo_to_parent);
    close(pipe_to_parent[0]);
    close(pipe_to_parent[1]);
    close(pipe_to_child[0]);
    close(pipe_to_child[1]);
    close(sock_to_child[0]);
    close(sock_to_child[1]);
    close(sock_to_parent[0]);
    close(sock_to_parent[1]);
    printf("\n");
    exit(dummy);
}


int command(int (*child_func)(), int (*parent_func)()){
    pid_t pid = fork();
    if(pid == -1){
        perror("Couldn't create a process!");
        printf("error");
        exit(1);
    }
    else if(pid == 0){ //child
         (*child_func)();
         _exit(0);
    }
    else{
        return (*parent_func)();
    }
}

void initialize_data_channels(){
    remove(fifo_to_child);
    remove(fifo_to_parent);

    if(pipe(pipe_to_child) == -1 || pipe(pipe_to_parent) == -1){
        printf("Eroare la deschiderea pipe-urilor");
        exit(1);
    }
    if(
        socketpair(AF_UNIX, SOCK_STREAM, 0, sock_to_child) < 0
        ||
        socketpair(AF_UNIX, SOCK_STREAM, 0, sock_to_parent) < 0
    )
    {
        printf("Eroare la deschiderea socketurilor!\n");
        exit(1);
    }
    if(
        mknod(fifo_to_child, S_IFIFO | 0666, 0) == -1 ||
        mknod(fifo_to_parent, S_IFIFO | 0666, 0) == -1 )
        {
            printf("Eroare la crearea fifo-urilor");
            exit(1);
        }
}

int main(){

    char input_line[128];
    
    initialize_data_channels();
    signal(SIGINT, sig_exit); 
    while(1){
        printf("Command: ");
        scanf("%[^\n]s\n", input_line);
        getchar(); // remove \0 at the end of stdin
        char * comm = spacetok(input_line);
        if(strcmp(comm, "quit")==0){
            printf("Bye!\n");
            sig_exit(0);
            break;
        }
        else if(strcmp("login", comm)==0){
            if(word_count()!=2)
                printf("Wrong number of arguments for command. Syntax: login <username>\n");
            else if(user_logged_in){
                printf("You have already logged in.\n");
            }
            else
                command(login_child, login_parent);
        }
        else if(strcmp("myfind", comm)==0){
            // printf("Inside myfind\n");
            if (!isUserLogged()) continue;
            if(word_count() < 3 || word_count() > 4){
                printf("Numar gresit de argumente pentru myfind. Sintaxa: myfind [<depth>] <folder> <file>\n");
            }
            else{
                command(myfind_child, myfind_parent);
            }
        }
        else if(strcmp("mystat", comm)==0){
            // printf("inside mystat\n");
            if(!isUserLogged()) continue;
            if(word_count() != 2){
                printf("Numar eronat de argumente pentru mystat. Sintaxa: mystat <path>\n");
            }   
            else{
                command(mystat_child, mystat_parent);
            }
        }
        
    }

    return 0;
}