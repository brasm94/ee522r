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

#include <sys/stat.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
//Server functions setup
int setup_server(char * portNum,struct addrinfo *p);
// void handle_request(int nfd,char * buf,int maxBufSize,int bufSize);
void handle_request(int nfd,char * buf,char * oBuf,int maxBufSize,int buffSizes);

// int handle_request(int nfd,char * buf,int maxBufSize,int buffSize,long int * requestPointers);

void send_response(int nfd,char * msg);
int handle_http(char * inMsg,char * outMsg,char * filename);
void send_file(int nfd,char * filename);



int main(int argc, char * argv[])
{
    const int maxBufSize = 65536;
    printf("Setting Up Server\n");
    struct addrinfo *p;
    struct sockaddr_storage their_addr;
    int new_fd;
    char s[INET_ADDRSTRLEN];
    

    socklen_t  sin_size;
        
    int port = 8080;
    int bufferSize = 1024;
    if(argc == 2)
    {
        port = atoi(argv[1]);
    }
    if(argc == 3)
    {
        port = atoi(argv[1]);
        bufferSize = atoi(argv[2]);
    }

    int socket_fd = setup_server(argv[1],p);
    printf("Started Server on port:%i with BufferSize:%i\n",port,bufferSize);

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


        if(!fork())
        {
            //need to add fucntionality for recv :D see above not implented function
            // sleep(5);
            close(socket_fd);
            while(1)
            {
                char inBuffer[maxBufSize];
                char outBuffer[maxBufSize];
                char singleMsg[maxBufSize];
                handle_request(new_fd,inBuffer,singleMsg,maxBufSize,bufferSize);
                char filename[1024];
                printf("The Request = \n%s\n",singleMsg);
                int afile = handle_http(singleMsg,outBuffer,filename);
                printf("Filename = %s\n",filename);
                send_response(new_fd,outBuffer);
                printf("Respose = %s \nafile = %i",outBuffer,afile);
                if(afile == 1)
                {
                    send_file(new_fd,filename);
                }
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


void handle_request(int nfd,char * buf,char * oBuf,int maxBufSize,int buffSizes)
{
    int recBytes;
    // int request = 0;
    // char * pBuf = buf;
    printf("\nHi Remaining Buffer**\n%s** \n",buf);
    printf("Length of Remain Buf %li\n",strlen(buf));
    int totalBytes = strlen(buf);
    while(1)
    {
        sleep(2);
        printf("One\n");
        fflush(stdout);
        recBytes = recv(nfd,buf + totalBytes,buffSizes-totalBytes,MSG_DONTWAIT);
        if(recBytes == -1)// || recBytes == 0) // AKA coecket was closed by Client
            exit(0);
        totalBytes += recBytes;
        // buf[totalBytes] = '\0';
        char * location = strstr(buf,"\r\n\r\n");
        if(location)
        {
            strncpy(oBuf,buf,(location+4)-buf);
            printf("found MSG one %s\n",oBuf);
            int size = totalBytes - (location + 4 - buf);
            memcpy(buf,location + 4,buffSizes - size);
            bzero(buf + buffSizes - size,size);
            printf("Remaining Buffer %s \n",buf);
            break;
        }

    }
    buf[totalBytes] = '\0';
    printf("My count vs strlen %i =? %lu\n",totalBytes,strlen(buf));
    printf("REC\n*****\n%s****\n",buf);
}

void send_response(int nfd,char * msg)
{
    int result = 0;
    int totalBytes = 0;
    while((result = send(nfd,msg,strlen(msg) - totalBytes,0) + totalBytes) < strlen(msg))
    {
        fprintf(stdout,"Didn't send complete MSG :( \n");
        totalBytes += result;
    }
    totalBytes += result;

    printf("Result %i bytes Sent\n",totalBytes);
}
void send_file(int nfd,char * filename)
{
    int result = 0;
    struct stat st;
    
    stat(filename, &st);
    FILE * fpic = fopen(filename,"r");
    char buffer[65536];
    int byteSent = 0;
    while(!feof(fpic))
    { 
        int bread = fread(buffer, 1, sizeof(buffer), fpic);
        int totalBytes = 0;
        while((result = send(nfd, buffer, bread,0)) + totalBytes < MIN(sizeof(buffer),st.st_size - byteSent))
        {
            totalBytes += result;
        }
        totalBytes += result;
        bzero(buffer, sizeof(buffer));
        byteSent += totalBytes;
    }
    fclose(fpic);
}

int handle_http(char * inMsg,char * outMsg,char * filename)
{
    char method[1024];
    char uri[1024];
    char version[1024];
    char * token = strtok(inMsg,"\r\n");

    char * files_names[] = {"page.html","Wilde.jpg"};
    sscanf(token,"%s %s %s",method,uri,version);
    // printf("Method = [%s]\n",method);
    // printf("URI = [%s]\n",uri);
    // printf("Version = [%s]\n",version);
    token = strtok(NULL,"\r\n"); // this doesn't actually work because searches for \r or \n but ehh
    int line = 1;
    while(token != NULL)
    {
        // printf("Line %i: %s\n",line,token);
        token = strtok(NULL,"\r\n");
        line++;
    }
    int idx = -1;
    for(int i = 0;i < 2;i++)
    {
        if(0 == strcmp(files_names[i],uri+1)) // +1 so doesn't compare the /
        {
            idx = i;
            break;
        }
    }
    //Not found return not found status lol
    if(idx == -1)
    {
        char * notfound = "HTTP/1.0 404 File Not Found\r\nServer:BrianRasmussen-server\r\nConnection: close\r\n\r\n";
        strncpy(outMsg,notfound,strlen(notfound));
        strncpy(filename,"NO FILE NOOB\0",strlen("NO FILE NOOB\0"));
        return -1;
    }
    else
    {
        char found[65536];
        int n;
        char file2open[1024];
        strcat(file2open,"files/");
        strcat(file2open,files_names[idx]);
     
        struct stat st;
        stat(file2open, &st);
        int filesize = st.st_size;

        char * status = "HTTP/1.0 200 OK";
        char * server = "Server:BrianRasmussen-server";
        // char * connection = "Connection: close";
        char * contentType = "Content-Type:";
        char * contenLen = "Content-Length:";

        sprintf(found,"HTTP/1.0 200 OK\r\n");
        sprintf(found,"%s%s\r\n",found,server);
        // sprintf(found,"%s%s\r\n",found,connection);
        sprintf(found,"%s%s%s\r\n",found,contentType,strchr(file2open,'.')+1);
        sprintf(found,"%s%s%li\r\n\r\n",found,contenLen,st.st_size);
        strncpy(outMsg,found,strlen(found));
        printf("FILE @ OPEN = %s\n",file2open);
        strncpy(filename,file2open,strlen(file2open));
        bzero(file2open,sizeof(file2open));
    }
    return 1;
}

// GET /page.html HTTP/1.1

// GET /page.html HTTP/1.1

// GET /page.html HTTP/1.1

// GET /page.html HTTP/1.1

// GET /page.html HTTP/1.1

// GET /page.html HTTP/1.1

