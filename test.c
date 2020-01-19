#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>

void testing();

int main(int argc, char * argv[])
{
	printf("I am smart\n");
	testing();
}


void testing()
{
	int status;
	struct addrinfo hints,*res , *p;

	char ipstr[INET6_ADDRSTRLEN];
	char * website = "www.example.net";
	memset(&hints,0,sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if((status = getaddrinfo(website,"http",&hints,&res)) != 0)
	{
		fprintf(stderr,"getaddrinfo Error: %s" , gai_strerror(status));
		exit(1);
	}
	int fd;
	fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(fd == -1)
		fprintf(stderr,"Getting the socket was bad :(\n");

	freeaddrinfo(res);
}

