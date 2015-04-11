/* Comp 112 Assignment 5
 * 
 * Siddhartha Prasad
 * Nikhil Shinday
 * Shea Clark-Tiesche
 *
 * This assignment implements a simple distributed interpreter
 * 
 *  Compile With: gcc -g iserv.c -lnsl
 *
 *  Run with port number as a command line argument
 * 
*/
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

FILE *child_in; //input into child

//FILE* child_out;

FILE *ilog;

int int_pid;
int hsize = 6;
void service_client(int);

struct __attribute__((__packed__)) header {
    unsigned short type;
    unsigned int len;
};

typedef struct __attribute__((__packed__)) header header;

void c_error(int i);

int sockfd, hisockfd;
int connectlist[200*sizeof(int)];


int clsize = 200;
fd_set socks;

void send_ack(int i) {

    header ack;
    ack.type = htons(2);
    ack.len = 0;
   
    write(connectlist[i], &ack, 6);

}

void sig_handler(int signum) {

    (void) signum;
    fclose(ilog);
    fclose(child_in);
    kill(int_pid, 9);   //Killing interpreter

    fprintf(stderr, "Closing server\n");
    exit(0);
}




void error(const char *msg)
{
    perror(msg);
    exit(1);
}


void write_to_all() {
    int i, read_len;
    char out[400];
    bzero((char *)&out, 400);

    //This step is going to be the issue
   // read_len = read(int_pid, out, 400);
    //
    header ack;
    ack.type = htons(2);
    ack.len = strlen("Placeholder");
    for (i=0; i<clsize; i++) {
        if (connectlist[i] != 0) {
            write(connectlist[i], &ack, 6);
            write(connectlist[i],"Placeholder" , read_len);
        }
    }
    fprintf(stdout, "-> %s\n",out);
}


void c_error(int i) {

    fprintf(stderr, "ERROR Closing\n");
 
}

/* Builds the sel_list, also gets highest socket number */
void build_sel_list () {
    int i;
    char buff[2];

    for(i = 0; i < clsize; i++) {
        if(connectlist[i] != 0 && read(connectlist[i], buff, 0) < 0) {
            close(connectlist[i]);
            connectlist[i] = 0;
        }
    }

    /* Making sure socks contains no file descriptors */
    FD_ZERO(&socks);

    /* Now adding sockfd to socks, so that select will return
     * if a connection comes in on that socket */   
    FD_SET(sockfd, &socks);
    /* Adds all possible connections to the fd_set socks */
    for(i = 0; i < clsize; i++) {
        if(connectlist[i] > 0) {
            FD_SET(connectlist[i], &socks);
            if (connectlist[i] > hisockfd)
                hisockfd = connectlist[i];
        }
    }

}

void new_connection () {
    //confd holds the fd for the new connection
    int i, confd;

    //Might want client address and client length here
    confd = accept(sockfd, NULL, NULL);

    if (confd < 0) {
        error("accept error");
    }
    
    for(i = 0; (i < clsize) && (confd != -1); i++) {
        if((connectlist[i] == 0)) {
            printf("\n Connection accepted at FD=%d; List slot=%d\n", confd, i);
            connectlist[i] = confd;
            confd = -1;
        }
    }
    
    if(confd != -1) {
        printf("No space!");
        close(confd);
    }

}

void read_socks() {
    int i;
    /* If a listening socket is part of the fd_set, we accept a new connection*/
    if (FD_ISSET(sockfd, &socks))
        new_connection();
    
    /* Now go through the sockets in the list, if anything happened,
     * deal with them */

    for(i = 0; i < clsize; i++) {
        if(FD_ISSET(connectlist[i], &socks)) {
            service_client(i);
        }
    }
}

int main(int argc, char *argv[])
{

    int fd_[2];
    pipe(fd_);
    int newsockfd, portno, i;
    int num_socks, numsocks; /* Holds the number of sockets ready for reading */
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval timeout; /* timeout for select */    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    /* Get file descriptor for " listening socket" */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    if(int_pid = fork()) {


        /*** Set up a signal handler for Ctr-C ***/
        struct sigaction act;
        act.sa_handler = &sig_handler;
        sigaction(SIGINT, &act, NULL);
        /****************************************/



        child_in = fdopen(fd_[1], "w");
        /* Set socket so that it is non-blocking */ 
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = atoi(argv[1]);

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);


        if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0) 
            error("ERROR on binding");

        listen(sockfd,5);


        /* We keep track of the highest socket fd*/
        hisockfd = sockfd;
        bzero((char*)connectlist, sizeof(connectlist));

        for (i = 0; i < 200; i++) {
            connectlist[i] = 0;
        }

        /* We set up a log of all commands sent as a file */

        ilog = fopen("logfile.scm", "w");


        /****************************************************/




        /* Now loop forever */ 
        while(1) {

            build_sel_list();

            num_socks = select(hisockfd+1, &socks, (fd_set *)0, (fd_set *) 0, NULL);

            if( num_socks < 0) {
                error("Selectd failed\n");

            } else if (num_socks == 0) {
                printf("Nothing selected\n");
            } else {
                read_socks();
            }  

        }
        close(sockfd);
        return 0; 
    } else {

        close(0);
        dup(fd_[0]);
        close(fd_[1]);
        close(fd_[0]);
        execl("/comp/105/bin/uscheme", "/comp/105/bin/uscheme", NULL);
    }
}



void service_client(int index) {

    int temp, i;
    int rd = 0;
    char msg[400];
    char final[401];
    bzero((char *) &msg, 400);
    bzero((char *) &final, 401);

    header hd;

    int read_s = read(connectlist[index], &hd, hsize);
   
    /* If an entire header is not read, we drop the connection */
     if(read_s < hsize) {
        printf("Connection lost with FD=%d, Slot=%d\n", connectlist[index], index);
        close(connectlist[index]);

        connectlist[index] = 0;
    } else {

    hd.type = ntohs(hd.type);   
    hd.len = ntohl(hd.len);
    
    if(hd.len > 400) {
        printf("Message length too long (for now)!\n");
        c_error(index);
        return;
    }
 
    if(hd.len > 0) {
        rd = read(connectlist[index], msg, 400);
        if(hd.len != rd) {
            printf("Error! Bad length Read %d Reported %d\n", rd, hd.len);
            c_error(index);
            return;
        }
        printf("Received string: %s\n", msg); 
    }
        // type 1 is to connect
        if(hd.type == 1) {
                       
            send_ack(index); 
        //Type 3 is interpreter command
        } else if (hd.type == 3 && (hd.len > 0)) {

                if(connectlist[index]) {                                  
                                
                    header ack; 
                    ack.type = htons(4);
                    ack.len = 0;
                    write(connectlist[index], &ack, 6);
                    
                                        
                    strcpy(final, (char *)strcat(msg, "\n"));
                    fwrite(final, strlen(final), 1, child_in);
                    fwrite(final, strlen(final), 1, ilog);
                    fflush(child_in);  
                    fflush(ilog);
                    write_to_all();

                }
                else {
                    /* ERROR */
                    c_error(index);
                }
        //Else problem!
        } else {
            fprintf(stderr, "Unrecognized type %hu\n", hd.type);
            c_error(index);
        }
    }
}


