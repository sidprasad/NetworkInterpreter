#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

struct __attribute__((__packed__)) header {
    unsigned short type;
    char source[20];
    char dest[20];
    unsigned int length;
    unsigned int message_id; 
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
    header hd;
    
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

    //Sending hello
        hd.type = 1;
        strcpy(hd.source, argv[3]);
        hd.dest[0] = '\0'; 
        hd.length = 0;
        hd.message_id = 0;
        n = write(sockfd, &hd, 50);


        header ack, clist;
        char client_list[400];
        char temp[100];

        read(sockfd, &ack, 50);
        read(sockfd, &clist, 50);
        read(sockfd, &client_list, clist.length);
        
    
  //  while (1) {
        printf("Enter Message: ");
        fgets(buffer, 255, stdin);

        header ex;
        ex.type = 6;
        strcpy(ex.source, argv[3]);
        strcpy(ex.dest, "Server");
        ex.length = 0;
        ex.message_id = 0;

    //    write(sockfd, &ex, 50);
    //    write(sockfd, "", 0);

        if (n < 0) {
            error("ERROR writing to socket");
        }

       // bzero(buffer, 256);
      //  n = read(sockfd, buffer, 255);

        if (n < 0) {
            error("ERROR reading from socket");
        }

        //printf("%s\n", buffer);
  //  }

    close(sockfd);

    return 0;
}
