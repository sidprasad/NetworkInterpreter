/* Comp 112 Assignment 5
 * 
 * Siddhartha Prasad
 * Nikhil Shinday
 * Shea Clark-Tieche
 *
 * This assignment implements a network interpreter
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
#include "./file_transfer/ftrans.c"

FILE *child_in; //input into child
FILE* intermediate;
FILE *ilog;

char *interpreter_address = "interpreters/uscheme";

int int_pid;
int hsize = 6;

int sockfd, hisockfd;
int connectlist[200*sizeof(int)];

int clsize = 200;
fd_set socks;

typedef struct __attribute__((__packed__)) header {
    unsigned short type;
    unsigned int len;
} header;

void c_error(int i);
void service_client(int);
int readMsg(int, int, char **); 
int parenCheck(char*, int);

void send_ack(int i) 
{
    header ack;
    ack.type = htons(8);
    ack.len = 0;
   
    write(connectlist[i], &ack, hsize);
}

void sig_handler(int signum) 
{
    (void) signum;
    fclose(ilog);
    fclose(child_in);
    kill(int_pid, 9);   //Killing interpreter
    remove("intermediate");
    fprintf(stderr, "Closing server\n");
    exit(0);
}


void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void write_to_all() 
{
    FILE *inter_in;
    int fd[2];
    int temp_pid;

    pipe(fd);
    
    if(temp_pid = fork()) {
        char *out = NULL;
        char *out2 = NULL;
        size_t n = 400;
        size_t n2 = 400;
        int i;

        inter_in = fdopen(fd[0], "r");
        close(fd[1]);
        getline(&out, &n, inter_in);
        system("cat intermediate");
        n = strlen(out);
        
        header ack;
        ack.type = htons(2);
        ack.len = htonl(n);
    
        for (i = 0; i < clsize; i++) {
            if (connectlist[i] != 0) {
                fprintf(stderr, "Writing: %s to fd:%d with length %d \n",
                            out, connectlist[i], n);
                write(connectlist[i], &ack, hsize);
                write(connectlist[i], out, n);
            }
        }

    } else {
        close(1);
        dup(fd[1]);
        close(fd[1]);
        close(fd[0]);
        execl("/usr/bin/tail", "/usr/bin/tail", "-1", "intermediate", NULL);
    }
}


void c_error(int i) 
{
    fprintf(stderr, "ERROR Closing\n");
}

/* Builds the sel_list, also gets highest socket number */
void build_sel_list () 
{
    int i;
    char buff[2];

    for (i = 0; i < clsize; i++) {
        if (connectlist[i] != 0 && read(connectlist[i], buff, 0) < 0) {
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
    for (i = 0; i < clsize; i++) {
        if (connectlist[i] > 0) {
            FD_SET(connectlist[i], &socks);
            if (connectlist[i] > hisockfd)
                hisockfd = connectlist[i];
        }
    }
}

void new_connection () {
    /* confd holds the fd for the new connection */
    int i, confd;

    confd = accept(sockfd, NULL, NULL);

    if (confd < 0) {
        error("accept error");
    }
    
    for (i = 0; (i < clsize) && (confd != -1); i++) {
        if (connectlist[i] == 0) {
            printf("\n Connection accepted at FD=%d; List slot=%d\n", confd, i);
            connectlist[i] = confd;
            confd = -1;
        }
    }
    
    if (confd != -1) {
        fprintf(stderr, "No space!\n");
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
    for (i = 0; i < clsize; i++) {
        if (FD_ISSET(connectlist[i], &socks)) {
            service_client(i);
        }
    }
}

int main(int argc, char *argv[])
{
    int fd_[2];
    int newsockfd, portno, i;
    int num_socks, numsocks; /* Holds the number of sockets ready for reading */
    struct sockaddr_in serv_addr, cli_addr;
    struct timeval timeout; /* timeout for select */    
    
    pipe(fd_);
    
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    /* Get file descriptor for " listening socket" */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    intermediate = fopen("intermediate", "a+");
    setvbuf(intermediate, NULL, _IONBF, BUFSIZ);
    int inter_fd = fileno(intermediate); 

    if (int_pid = fork()) {
        
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
                    sizeof(serv_addr)) < 0) {
            error("ERROR on binding");
        }
        listen(sockfd,5);

        /* We keep track of the highest socket fd*/
        hisockfd = sockfd;

        bzero((char*)connectlist, sizeof(connectlist));

        /* We set up a log of all commands sent as a file */
        ilog = fopen("logfile.scm", "w");
        /****************************************************/
        /* Now loop forever */ 
        while(1) {

            build_sel_list();

            num_socks = select(hisockfd+1, &socks, (fd_set *)0, (fd_set *) 0, NULL);

            if (num_socks < 0) {
                error("Selectd failed\n");
            } else if (num_socks == 0) {
                fprintf(stderr, "Nothing selected\n");
            } else {
                read_socks();
            }  

        }
        close(sockfd);
        return 0; 
    } else {

        close(2);
        dup(inter_fd);
        close(1);
        dup(inter_fd);
        close(0);
        dup(fd_[0]);
        close(inter_fd);
        close(fd_[1]);
        close(fd_[0]);
        execl(interpreter_address, interpreter_address, "-q",  NULL);
    }
}

void service_client(int index) {

    int temp, i;
    int rd = 0;
    char *msg;
    char *final;

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
   
        msg = malloc(hd.len);

        if(hd.len > 0) {
            rd = readMsg(index, hd.len, &msg);
            if(hd.len != rd) {
                fprintf(stderr, "Error! Bad length Read %d Reported %d\n", rd, hd.len);
                c_error(index);
                return;
            }   
        }   
        /* Type 1: Connection Request */
        if(hd.type == 1) {
            send_ack(index);
        /* Type 3: Interpreter Command */
        } else if (hd.type == 3 && (hd.len > 0)) {
            if (connectlist[index]) {                                  
                header ack; 
                ack.type = htons(4);
                ack.len = htonl(0);
                write(connectlist[index], &ack, hsize);
                
                if (parenCheck(msg, rd)) { 
                    final = malloc(rd + 1);
                    strcpy(final, (char *)strcat(msg, "\n"));                    
                    fwrite(final, strlen(final), 1, child_in);
                    fwrite(final, strlen(final), 1, ilog);
                    fflush(child_in);  
                    fflush(ilog);
                    write_to_all();
                } else {
                    header errorAck;
                    char *err = "Mismatched parentheses\n";
                    errorAck.type = htons(2);
                    errorAck.len = htonl(24);

                    write(connectlist[index], &errorAck, hsize);
                    write(connectlist[index], err, 24);         
                }
            } else {
                c_error(index);
            }

        } else if (hd.type == 3) {
            printf("Disregarding empty message\n");
        } else if (hd.type == 5 && connectlist[index]) {
            fprintf(stderr, "Graceful exit with user %d\n", index);
            /* Send copy of uScheme interpreter so user can continue offline */
            header ack;
            ack.type = htons(hsize);
            ack.len = 0;
            write(connectlist[index], &ack, hsize);
            if (FILE_SEND(connectlist[index], interpreter_address) == 0) {
                ack.type = htons(7);
                write(connectlist[index], &ack, hsize);

                if (FILE_SEND(connectlist[index], "logfile.scm") == 0) {}
                else  printf("transferring log file failed\n");
            } else printf("transferring intepreter file failed\n"); 
            close(connectlist[index]);
            connectlist[index] = 0;
        } else {
            fprintf(stderr, "Unrecognized type %hu\n", hd.type);
            c_error(index);
        }
    }
}

int readMsg(int sock, int totalBytes, char **str)
{
    int bytesRead = 0;
    int rd;
    char *msg = *str;

    while (bytesRead < totalBytes) {
        rd = read(connectlist[sock], msg + bytesRead, totalBytes - bytesRead);
        if (rd < 1) {
            fprintf(stderr, "Error Reading Data\n");
            exit(1);
        }
        bytesRead += rd;
    }
    return bytesRead;
}

int parenCheck(char *msg, int n)
{
    int i, count = 0;
    char c;
    if (msg == NULL) return 0;
    
    for (i = 0; i < n; i++){
        c = msg[i];
        if (c == '(') {
            count++;
        } else if (c == ')') {
            count--;
            if (count < 0) 
                return 0;
        }
    }
    if (count > 0)
        return 0;
    return 1;
}








