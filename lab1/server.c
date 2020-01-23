#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

//Server functions setup
int setup_server(char * portNum,struct addrinfo *p);
void handle_request(int nfd,char * buf,int maxBufSize);


int main(int argc, char * argv[])
{
    const int maxBufSize = 65536;
    printf("Setting Up Server\n");
    struct addrinfo *p;
    struct sockaddr_storage their_addr;
    int new_fd;
    char s[INET_ADDRSTRLEN];
    char buffer[maxBufSize];
    socklen_t  sin_size;
    
    int socket_fd = setup_server(argv[1],p);

    while(1){
        sin_size = sizeof(their_addr);
        new_fd = accept(socket_fd,(struct sockaddr *)&their_addr,&sin_size);
        if(new_fd == -1){
            fprintf(stderr,"Error accepting :( \n");
            exit(0);
            continue;
        }
        struct sockaddr *sa = (struct sockaddr *)&their_addr;
        struct sockaddr_in * si = (struct sockaddr_in *)sa;
        inet_ntop(their_addr.ss_family,&(si->sin_addr),s,sizeof(s));
        printf("Server: connected to [%s]\n",s);


        if(!fork()){
            printf("Entered Here\n");
            //need to add fucntionality for recv :D see above not implented function
            close(socket_fd);
            printf("Entered Here2\n");

            handle_request(new_fd,buffer,maxBufSize);
            if(send(new_fd,"stupid",strlen("stupid"),0) != -1)
            {
                fprintf(stderr,"error sending dummy response\n");
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    return 0;
}


int setup_server(char * portNum,struct addrinfo *p)
{
    int status;
    struct addrinfo hints;
    struct addrinfo *result;
    // struct addrinf *p;
    int socket_fd;
    int yes=1;

    memset(&hints,0,sizeof hints);

    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(NULL,portNum,&hints,&result)) != 0)
    {
        fprintf(stderr,"GetAddrinfo - :(\n");
        exit(1);
    }
    for(p = result;p != NULL;p = p->ai_next)
    {
        if((socket_fd = socket(p->ai_family,p->ai_socktype,p->ai_protocol)) == -1){
            fprintf(stderr,"Trouble with Socket and File Descriptor\n");
            continue;
        }
        if(setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1){
            fprintf(stderr,"Trouble with OPTIONS Socket and File Descriptor\n");
            exit(1);
        } 
        if(bind(socket_fd,p->ai_addr,p->ai_addrlen) == -1){
            close(socket_fd);
            fprintf(stderr,"Trouble Binding FD with Address\n");
            continue;
        }
        break;
    }
    freeaddrinfo(result);
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(socket_fd, 5) == -1) {
        perror("listen");
        exit(1);
    }
    return socket_fd;
}


void handle_request(int nfd,char * buf,int maxBufSize)
{
    int recBytes;
    int totalBytes = 0;

    while(((recBytes = recv(nfd,buf + totalBytes,2,0)) != -1)
    {
        totalBytes += recBytes;
        fprintf(stdout,"%s\n",buf);
        printf("My count vs strlen %i =? %lu Rec Byte %i\n",totalBytes,strlen(buf),recBytes);
        // Need to add check to see if \r\n :D
    }

    buf[totalBytes] = '\0';
    printf("My count vs strlen %i =? %lu\n",totalBytes,strlen(buf));
    printf("REC %sHere\n",buf);

}