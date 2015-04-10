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
    unsigned int len;
};

typedef struct __attribute__((__packed__)) header header;

