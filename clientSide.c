/*
 * Author: Daniel Moreno
 * Description: This program will establish this machine as a client, will send ping packets to the specified server and port, wait 1 second for a response, will print a message depending on whether a response is received, and will print statistics regarding the round-trip response times
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <limits.h>
#include <time.h>
#include <float.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
	
#define MAXLINE 1024
	
// Driver code
int main(int argc, char* argv[]) {
	//If anything other than 3 arguments were passed (the program call, hostname, the port number), then print a usage statement and terminate the program
	if (argc != 3) {
		printf("Usage: %s <hostname> <port>\n", argv[0]);
		exit(1);
	} 
	//if exactly 3 arguments were passed, start sending the pings
	else {
		int t;
		int n;
		int sockfd;
		int len;
		int pingsSent = 0;
		int pingsReceived = 0;
		double pingXMtoRCratio;
		double avgRTT;
		double shortestRTT = FLT_MAX;
		double longestRTT = FLT_MIN;
		double sumOfRTT = 0;
		char buffer[MAXLINE];
		char *hello = "PING";
		char *end;
		const long PORT = strtol(argv[2], &end, 10); //converts port argument into a usable long, stores any nonnumerical characters in the end variable
		struct sockaddr_in servaddr;
		struct hostent *hostNameStruct;
		struct timeval stTimeOut;
		fd_set stReadFDS;
		clock_t timeSpentOnOp;
		
		
		
		//catch errors with the passed port number
		if (*end != '\0') {
			printf("Port argument is not a valid number\n");
			exit(1);
		}
		
		//set up the host name for the local host
		hostNameStruct = gethostbyname (argv[1]);
		if (!hostNameStruct) {
			printf("Error when getting the IP address associated with the specified hostname\n");
			exit(1);
		}
		
		//Create UDP socket file descriptor for INET address family
		if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
			printf("Socket creation failed\n");
			exit(1);
		}
		
		//copies the value 0 into every byte of the pointer
		memset(&servaddr, 0, sizeof(servaddr));
			
		//Fill in server information
		servaddr.sin_family = AF_INET; // IPv4
		servaddr.sin_addr = *((struct in_addr *)hostNameStruct->h_addr);
		servaddr.sin_port = htons(PORT); 
			
		//while loop that will execute 10 times
		while (pingsSent < 10) {
			//set/reset file descriptors each iteration
			//for unknown reason, these statements must be placed before the sendto function to prevent a malfunction from the select function
			FD_ZERO(&stReadFDS);
			FD_SET(sockfd, &stReadFDS);
			
			//set timeout to 1 second and 0 milliseconds
			stTimeOut.tv_sec = 1;
			stTimeOut.tv_usec = 0;
			
			//start the round-trip timer
			timeSpentOnOp = clock();
			
			//send the ping
			sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
			printf("%3d: Sent... ", pingsSent + 1);
			
			//wait for the set period until something is sent to the client
			t = select(sockfd+1, &stReadFDS, NULL, NULL, &stTimeOut);
			if (t) {
				//if you can, read from the socket
				if (FD_ISSET(sockfd, &stReadFDS)) {
					//get the message sent back
					n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &servaddr, &len);
					//stop the clock
					timeSpentOnOp = clock() - timeSpentOnOp;
					//terminate the buffer string
					buffer[n] = '\0';
					//calculate the clock
					double timeTaken = ((double)timeSpentOnOp)/CLOCKS_PER_SEC;
					//print the time taken
					printf("RTT=%f ms\n", timeTaken);
					//increment the number of received pings
					pingsReceived++;
					//add the timeTaken to the overall sum of RTTs
					sumOfRTT = sumOfRTT + timeTaken;
					//if lower, store the new RTT
					if (shortestRTT > timeTaken) {
						shortestRTT = timeTaken;
					}
					//if higher, store the new RTT
					if (longestRTT < timeTaken) {
						longestRTT = timeTaken;
					}
				} 
				else {
					printf("Cannot read from the socket\n");
				}
			} else {
				printf("Timed Out\n");
			}
						
			//incrementing the number of pingsSent so while loop will eventually terminate
			pingsSent++;
		}
		
		//if all of the pings timed out, print a slightly different message
		if (pingsReceived == 0) {
			printf("%3d pkts xmited, %3d pkts rcvd, %5.0f%% pkt loss\n", pingsSent, pingsReceived, pingXMtoRCratio);
			printf("min: %8.6f ms, max: %8.6f ms, avg: %8.6f ms\n", 0.0, 0.0, 0.0);
		} else {
			//print the closing two lines after calculating the averages
			pingXMtoRCratio = 100 - (((double)pingsReceived / (double)pingsSent) * 100);
			avgRTT = (sumOfRTT / pingsReceived);
			printf("%3d pkts xmited, %3d pkts rcvd, %5.0f%% pkt loss\n", pingsSent, pingsReceived, pingXMtoRCratio);
			printf("min: %8.6f ms, max: %8.6f ms, avg: %8.6f ms\n", shortestRTT, longestRTT, avgRTT);
		}
		
		//Close socket before the program terminates
		close(sockfd);
		return 0;
	}
}
