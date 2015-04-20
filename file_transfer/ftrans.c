#ifndef __FTRANS__
#define __FTRANS__
/*
 *      interface that makes file transfer easy among a TCP client and server
 *      TCP packets arrive in-order, so we only need to be concerned about the length of 
 *      each packet. 
 *
 */

/*
 * parameters: a socket file descriptor and a file name to read from
 * return val: integer that represents different states of success/failure:
 *             - 0 indicates successful exit
 *             - -1 indicates that the file read failed
 *             - -2 indicates that there was a write error
 */
int FILE_SEND(int sockfd, char* fname)
{
        FILE *fp = fopen(fname, "r");
        char* buffer[1460];
        int len;
        if (fp == NULL) {
                return -1;
        } 
        while ((len = fread(buffer, 1, 1460, fp)) > 0) {
                if (write(sockfd, &len, 4) != 4) {
                        close(sockfd);
                        return -2;
                } 
                if (write(sockfd, buffer, len) != len) {
                        close (sockfd);
                        return -2;
                }
        }
        len = -1;
        write(sockfd,&len,4);
        fclose(fp);
        return 0;
}

/*
 * parameters: a socket file descriptor and a file name to write to
 * return val: integer that represent different states of success:
 *             - 0 indicates successful exit
 *             - -1 indicates that the file write failed
 *             - -2 indicates that the socket closed during the transfer
 */
int FILE_RECV(int sockfd, char* fname)
{
        FILE* fp = fopen(fname, "w");
        char*buffer[1460];
        int len, len_msg;
        if (fp == NULL) return -1;
        len = read(sockfd, &len_msg, 4);
        while (len_msg != -1) {
                // printf("test1: len_msg is %d\n", len_msg);
                // fflush(stdout);
                while ((len = recv(sockfd, buffer, len_msg, MSG_PEEK)) != len_msg) {
                        if (len == 0) {
                                close(sockfd);
                                return -2;
                        } else if (len < 0) {
                                perror("FILE_RECV: ");
                        }
                }
                len = recv(sockfd, buffer, len_msg, 0); 
                // printf("test2: len is %d\n", len);
                // fflush(stdout);
                fwrite(buffer,1,len,fp);
                // printf("test3\n");
                // fflush(stdout);
                while ((len = recv(sockfd, &len_msg, 4, 0)) != 4) {
                        if (len == 0) {
                                close(sockfd);
                                return -2;
                        } else if (len < 0) {
                                perror("FILE_RECV: ");
                        }
                }
                
        }
        fclose(fp);
        return 0;
}

// int FILE_SEND(int sockfd, char* fname) 
// {
//         FILE* fp = fopen(fname, "r");
//         if (fp == NULL) return -1;
//         size_t len, size;
//         fseek(fp, 0, SEEK_END);
//         size = ftell(fp);
//         fseek(fp, 0, SEEK_SET);
//         char buffer[size];
//         len = fread(buffer, 1, size, fp); 
//         if (len != size) return FILE_SEND(sockfd, fname);
//         write(sockfd, &size, sizeof(size));
//         write(sockfd, buffer, size);
//         return 0;
// }

// int FILE_RECV(int sockfd, char* fname) 
// {
//         FILE* fp = fopen(fname, "w");
//         if (fp == NULL) return -1;
//         size_t len, size;
//         read(sockfd, &size, sizeof(size));
//         char buffer[size];
//         len = read(sockfd, buffer, size); 
//         if (len != size) return FILE_RECV(sockfd, fname);
//         fwrite(buffer, 1, size, fp);
//         return 0;
// }
#endif