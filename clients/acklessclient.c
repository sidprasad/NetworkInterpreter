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
    header hd, msg;
    
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

        //Sending connect message
        hd.type = htons(1);
        hd.length = htonl(0);
        n = write(sockfd, &hd, 6);
        int sn = 0;

        header ack;
        char interpreter_msg[400];
        char temp[100];
        /*
        read(sockfd, &ack, 6);
        printf("Received ack with type %d\n", ntohs(ack.type)); 
        fflush(stdout); 
        */
    while (sn >= 0) {
      
        bzero(buffer, 256);
        printf(">: ");
        fgets(buffer, 255, stdin);

        header ex;
        ex.type = htons(3);
        ex.length = htonl(strlen(buffer));

        sn = write(sockfd, &ex, 6);
        write(sockfd, buffer, strlen(buffer));

        if (sn < 0) {
            error("ERROR writing to socket");
        }

       /* sn = read(sockfd, &ack, 6);
        printf("ACKTYPE: %d\n", ntohs(ack.type));
     
        sn = read(sockfd, &ack, 6);
        sn = read(sockfd, interpreter_msg, ntohl(ack.length));
    
        if (sn < 0) {
            error("ERROR reading from socket");
        } else {
           printf("Type: %d >: %s \n", ntohs(ack.type), interpreter_msg);

        }
*/
        
        read(sockfd, &ack, 6);
        printf("Received ack with type %d length %d\n", ntohs(ack.type), ntohl(ack.length)); 
        fflush(stdout); 

        read(sockfd, &ack, 6);
        printf("Received ack with type %d length %d\n", ntohs(ack.type), ntohl(ack.length)); 

        bzero(buffer, 256);
        read(sockfd, &buffer, ntohl(ack.length));
        printf("GOT: %s\n", buffer);
        fflush(stdout); 


   }
    printf("Exiting\n");
    close(sockfd);

    return 0;
}
