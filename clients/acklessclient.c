#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include "../file_transfer/ftrans.c"

int to_transfer;
int received;

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
int sockfd;

void read_acks() {

    int sn;
 /* Probably want a mutex lock or semaphore for both kinds of reads and writes*/
    char buffer[256];
    header ack;;

    while(1) {
        read(sockfd, &ack, 6);
        bzero(buffer, 256);
        
        if(ntohs(ack.type) == 2) {
            read(sockfd, &buffer, ntohl(ack.length));
            printf(":> %s", buffer);
        }else if(ntohs(ack.type) == 4) {
            //Create local log here!! Alternately, use locks and then
            //you can send an "accept dropped message"
        } else if (ntohs(ack.type) == 6) {
            
            to_transfer = 1;
            system ("rm uscheme1; touch uscheme1; chmod +x uscheme1");
            sn = FILE_RECV(sockfd, "uscheme1");
            if (sn == -1) {
                fprintf(stderr, "file didn't write correctly\n");
                exit(1);
            } else if (sn == -2) {
                fprintf(stderr, "socket closed during transfer of logfile\n");
                exit(1);
            }

        } else if (ntohs(ack.type) == 7) {
            printf("Getting logfile\n"); 
            sn = FILE_RECV(sockfd, "logfile.scm");
            if (sn == -1) {
                fprintf(stderr, "file didn't write correctly\n");
                exit(1);
            } else if (sn == -2) {
                fprintf(stderr, "socket closed during transfer of logfile\n");
                exit(1);
            }
                to_transfer = 1;
            //To change this
                break;
        } else {
            fprintf(stderr, "Bad type %d", ntohs(ack.type));
        }
        fflush(stdout);
    }

    received = 1; 
    //Do stuff here

    //system("./uscheme1");
}


int main(int argc, char *argv[])
{
    int  portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    header hd, msg;
   
    received = 0;
    to_transfer = 0; 

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

    if (read(sockfd, &ack, 6) == 6) { 
        printf("%d\n", ack.type);
        if (ntohs(ack.type) != 8) { 
            printf("client connect unsuccessful\n"); 
            exit(1); 
        } 
        else printf("client connect successful\n");
    } 
    
    
    pthread_t read_from;
    pthread_create(&read_from, NULL, read_acks, NULL);
    
    printf(">: ");
    while (!to_transfer) {
        bzero(buffer, 256);
         
        fgets(buffer, 255, stdin);
        if (strlen(buffer) > 0) {
            printf(">: ");
        }
        header ex;
        if (strcmp(buffer, "(exit)\n") == 0) {

            ex.type = htons(5);
            ex.length = htonl(0);
          
            sn = write(sockfd, &ex, 6);
            if (sn < 0) {
                error("ERROR writing to socket");
            }

        } else {
            ex.type = htons(3); 
            ex.length = htonl(strlen(buffer));

            sn = write(sockfd, &ex, 6);
            write(sockfd, buffer, strlen(buffer));

            if (sn < 0) {
                error("ERROR writing to socket");
            }    
        }
        
        
   }
    close(sockfd);
    printf("Closed connection\n");
    fflush(stdout);
    while(!received) {
            printf("!");
            fflush(stdout);
    }

    FILE * in;
    int fd[2];
    int temp_pid;

    pipe(fd);
    
    if(temp_pid = fork()) {
        char *input = NULL;
        size_t n = 256;
        
        // Read things and write them to in
        fprintf(in, "(use logfile.scm)");
      while(1) {
        getline(&input, &n, stdin);
        fprintf(in, input);
        }

    } else {
        close(0);
        dup(fd[0]);
        close(1);
        execl("uscheme1", "uscheme1", NULL);

    }


    return 0;
}
