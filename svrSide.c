/*
 * Author: Daniel Moreno
 * Description: This program will establish the machine as a server listening on a port, will print a message whether it receives a packet or lost it, and will generate a random number to determine whether the packet is "lost"
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
	
#define MAXLINE 1024
	
// Driver code
int main(int argc, char* argv[]) {
	//If anything other than 2 arguments were passed (the program call and the port number), then print a usage statement and terminate the program
	if (argc != 2) {
		printf("Usage: %s <port>\n", argv[0]);
		exit(1);
	} 
	//if exactly 2 arguments were passed, start listening for pings from the client and respond if the server receives oe that isn't "lost"
	else {		
		//seed the random number generator based on the time and PID
		srand(time(NULL) + getpid());
		//variable definitions and initializations
		int sockfd;
		int len;
		int n;
		int randomNum;
		char buffer[MAXLINE];
		char *hello = "PONG";
		char *end;
		const long PORT = strtol(argv[1], &end, 10); //converts port argument into a usable long, stores any nonnumerical characters in the end variable
		struct sockaddr_in servaddr;
		struct sockaddr_in cliaddr;
		
		//catch errors with the passed port number
		if (*end != '\0') {
			printf("Port argument is not a valid number");
			exit(1);
		}
				
		//Create UDP socket file descriptor for INET address family
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
			printf("Socket creation failed");
			exit(1);
		}
			
		//copies the value 0 into every byte of the pointer
		memset(&servaddr, 0, sizeof(servaddr));
		memset(&cliaddr, 0, sizeof(cliaddr));
			
		//Fill in server information
		servaddr.sin_family = AF_INET; // IPv4
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(PORT); 
			
		// Bind the socket with the server address
		if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) {
			printf("Socket binding failed\n");
			exit(1);
		}
		
		//set value of len to prevent buffer overflow
		len = sizeof(cliaddr); 
		
		//Print to console that the server is ready
		printf("[server]: ready to accept data...\n");	
		//Infinite loop so server keeps listening until it is interrupted
		while (1) {
			//get the message sent from the client and store it in the buffer
			n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
			buffer[n] = '\0';
			
			//determine whether the packet should be "lost" based on a random number
			//generate the random number from [1,10]
			randomNum = rand() % 10 + 1;
			//if the number is 3 or less, "lose" the packet as 30%
			if (randomNum <= 3) {
				printf("[server]: dropped packet\n");
			} else {
				printf("[client]: %s\n", buffer);
				sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
			}
		}
			
		return 0;
	}
}
