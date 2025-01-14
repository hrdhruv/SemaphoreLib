#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<stdbool.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<sys/time.h>
#include<sys/poll.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/resource.h>
#include<signal.h>
#include<time.h>
#include<pthread.h>
#include<sched.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>

#define PORT 8080

struct User{
    char username [100];
    char password [100];
    int type;
};

struct Book{
    char title[100];
    char author[100];
    bool valid;
    int copies;
    int id;
};

struct issue{
    char username[100];
    int bookid;
    bool valid;
};