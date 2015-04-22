#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "../file_transfer/ftrans.c"

struct __attribute__((__packed__)) header {
    unsigned short type;
    unsigned int length;
};
typedef struct __attribute__((__packed__)) header header;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    fd_set readfd, temp_readfd;
    header hd, msg;
    
    FD_ZERO(&readfd);
    FD_ZERO(&temp_readfd);



    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) { 
        error("ERROR connecting");
    }

    FD_SET(sockfd, &readfd);
    FD_SET(0, &readfd);

    //Sending connect message
    hd.type = htons(1);
    hd.length = htonl(0);
    n = write(sockfd, &hd, 6);
    int sn = 0;

    header ack;
    char interpreter_msg[400];
    char temp[100];

    read(sockfd, &ack, 6);
    printf("Received ack with type %d\n", ntohs(ack.type)); 
    fprintf(stdout, ">: ");
    fflush(stdout);
    while (sn >= 0) {
        temp_readfd = readfd;
        
        // means that there's input waiting to be read on stdin or the socket
        sn = select(sockfd+1, &temp_readfd, NULL, NULL, NULL);
        // check first for the socket
        if (FD_ISSET(sockfd, &temp_readfd)) { 
            bzero(buffer, 256);
            //read the ack then message
            header ack;
            sn = read(sockfd, &ack, 6);
            
            switch(ntohs(ack.type)) {
            case 2: 
                // an update to the environment -- just print out the string
                break;
            case 4: 
                // interpreter command ack
                fprintf(stdout, ">: ");
                fflush(stdout);
                break;
            case 6: 
                // send the interpreter file
                printf("got into case 6\n");
                FILE_RECV(sockfd, "interpreter");
                break;
            case 7: 
                // send the logfile
                printf("got into case 7\n");
                FILE_RECV(sockfd, "logfile");
                system("chmod +x interpreter; ./interpreter < logfile");
                break;
            default: 
                fprintf(stderr, "ack not recognized\n");
                fflush(stderr);
            }
        // something's waiting to be read from stdin
        } else if (FD_ISSET(0, &temp_readfd)) {
            bzero(buffer, 256);
            fgets(buffer, 256, stdin);
            printf("%s\n", buffer);
            header ex;
            if (strcmp("(local)\n", buffer) == 0) {
                ex.type = htons(5);
                ex.length = 0;
                
                sn = write(sockfd, &ex, 6);
                if (sn < 0) error("ERROR writing to socket");
            } else {
                ex.type = htons(3);
                ex.length = htonl(strlen(buffer));
            /*TESTING WITH LARGE MSG SIZES BELOW*/
                sn = write(sockfd, &ex, 6);
                if (sn < 0) error("ERROR writing to socket");
                 
                sn = write(sockfd, buffer, strlen(buffer));
                if (sn < 0) error("ERROR writing to socket");/*
                char *threehuno = "ABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXY\n";
                int strLen = strlen(threehuno) + 1;
                ex.length = htonl(strLen);
                printf("SIZE IS: %d\n", strLen);
                sn = write(sockfd, &ex, 6);
                sn = write(sockfd, threehuno, strLen);   */
            }
        }
    }
    printf("Exiting\n");
    close(sockfd);

    return 0;
}
