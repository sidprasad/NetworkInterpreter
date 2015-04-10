/* Comp 112 Assignment 5
 * 
 * Siddhartha Prasad
 *
 * This assignment implements a simple distributed interpreter
 * 
 *  Compile With: gcc -g iserv.c -lnsl
 *
 *  Run with port number as a command line argument
 * 
*/

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>


int int_pid;
int hsize = 6;


int main(int argc, char *argv[])
{
    FILE *child_in;
    int fd_[2];
    pipe(fd_);
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    if(int_pid = fork()) {

        //This connects parent stdout to child stdin
        //close(1);
        //dup(fd_[1]);
        //close(fd_[1]);
        //close(fd_[0]);

        child_in = fdopen(fd_[0], "r");
        close(fd_[1]);
        char buff[512];
        
        while(1) {
        
            //fprintf(stdout, "(+ 6 3)\n");
            bzero(buff, 512);
            //fprintf(stderr, "Sent\n");
            fgets(buff, 22, child_in);
            //if( n < 0) {
              //  printf("PROBLEM\n");
           // }
            fprintf(stderr, "Intepreter says: %s\n", buff);
            
            //bzero(buff, 512);
            //printf("Input please: ");
            //fgets(buff, 255, stdin);
            
            //write(int_pid, buff, 512);
        }

        return 0; 
    } else {

        // This connects childs stdout to parent stdin
        close(1);
        dup(fd_[1]);
        close(fd_[1]);
        close(fd_[0]);
        execl("/comp/105/bin/uscheme", "/comp/105/bin/uscheme", NULL);
    }
}

