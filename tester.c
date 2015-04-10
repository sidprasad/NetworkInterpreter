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
    int fd_[2];
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    if(int_pid = fork()) {

        char buff[512];
        
        sleep(2);
        while(1) {
            bzero(buff, 512);
            read(int_pid,  buff, 512);
            printf(">: %s\n", buff);

            bzero(buff, 512);
            fgets(buff, 512, stdin);
            
            write(int_pid, buff, 512);
        }

        return 0; 
    } else {
        close(1);
        dup(fd_[1]);
        close(fd_[1]);
        close(fd_[0]);
        execl("/usr/sup/bin/sml", "/usr/sup/bin/sml", NULL);
    }
}

