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
    FILE *child_out;
    FILE *child_in;
    int fd_[2];
    pipe(fd_);
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    if(int_pid = fork()) {


        //child_out = fdopen(fd_[0], "r");
        child_in = fdopen(fd_[1], "w");
        
        close(fd_[0]);
        char buff[512];
        
        char* msg = malloc(512);
        fgets(buff, 256, stdin);
        printf(buff);
        fwrite(buff, strlen(buff), 1, child_in);
        fprintf(child_in, "(define x (y) (+ y 1))");
        fprintf(child_in, "(x 9)\n");
       /* while(1) {
            bzero(msg, 512); 
            bzero(buff, 512);
            //fprintf(stdout, "Give me a command: \n");
            fgets(buff, 256,  stdin);
            msg = strcat(buff, "\n");
            
        }*/

        return 0; 
    } else {

       // HOPEFULLY, this will connect parents stdout to childs stdin 
       close(0);
       dup(fd_[0]);


        // This connects childs stdout to parent stdin
        //close(1);
        //dup(fd_[1]);
        close(fd_[1]);
        close(fd_[0]);
        char buff[500];
        

        execl("/comp/105/bin/uscheme", "/comp/105/bin/uscheme", NULL);
    }
}
