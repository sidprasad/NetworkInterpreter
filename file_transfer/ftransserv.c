#include <signal.h>
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
#include "ftrans.c"

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
int main(int argc, char *argv[])
{
    int listener, sockfd, portno;
    struct sockaddr_in serv_addr, cli_addr;
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    /* Get file descriptor for " listening socket" */
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) 
        error("ERROR opening socket");
     // Set socket so that it is non-blocking  
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    if (bind(listener, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(listener, 5);
    while(1) {
        sockfd = accept(listener, NULL, NULL);
       
        fflush(stdout);
        FILE_SEND(sockfd, "uscheme");
        close(sockfd);
        
    }
    close(listener);
}

