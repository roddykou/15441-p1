/******************************************************************************
* echo_server.c                                                               *
*                                                                             *
* Description: This file contains the C source code for an echo server.  The  *
*              server runs on a hard-coded port and simply write back anything*
*              sent to it by connected clients.  It does not support          *
*              concurrent clients.                                            *
*                                                                             *
* Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                         *
*          Wolf Richter <wolf@cs.cmu.edu>                                     *
*                                                                             *
*******************************************************************************/

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>

#define ECHO_PORT     9991
#define BUF_SIZE      4096
#define NUM_CLIENTS   10

int close_socket(int sock) {
	if (close(sock)) {
		fprintf(stderr, "Failed closing socket.\n");
		return 1;
	}
	return 0;
}

int main(int argc, char* argv[]) {
	int master_socket, client_sockets[NUM_CLIENTS], new_socket;
	int max_sd, sd;			/* socket descriptor */
	int activity;
	int i = 0, outer_looper_counter = 0;					/* iterator */
	ssize_t readret = 0;
	socklen_t addr_size;
	struct sockaddr_in addr;
	char buf[BUF_SIZE];
	fd_set readfds;
	fd_set writefds;

	fprintf(stdout, "----- Echo Server -----\n");

	/* all networked programs must create a socket */
	if ((master_socket = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		fprintf(stderr, "Failed creating master socket.\n");
		return EXIT_FAILURE;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(ECHO_PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	/* servers bind sockets to ports - notify the OS they accept connections */
	if (bind(master_socket, (struct sockaddr *) &addr, sizeof(addr)))
	{
		close_socket(master_socket);
		fprintf(stderr, "Failed binding master socket.\n");
		return EXIT_FAILURE;
	}

	/* try to specify maximum of 5 pending connections for the master socket */
	if (listen(master_socket, 5))
	{
		close_socket(master_socket);
		fprintf(stderr, "Error listening on socket.\n");
		return EXIT_FAILURE;
	}

	memset(client_sockets, 0, sizeof(client_sockets));
	addr_size = sizeof(addr);

	/* finally, loop waiting for input and then write it back */
	while (1) {
		printf("Outer Loop #%d\n", outer_looper_counter++);


		/* clear the socket set */
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		/* add master socket to set */
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		for (i = 0; i < NUM_CLIENTS; i++) {
			/* add valid socket descriptor to read list */
			sd = client_sockets[i];
			if (sd > 0) {
				printf("socket descriptor #%d is set prioir to the select function\n", sd);
				FD_SET(sd, &readfds);
				FD_SET(sd, &writefds);
			}

			/* we need highsest descriptor for select function */
			if (sd > max_sd)
				max_sd = sd;
		}

     	/* wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely */
     	if ((activity = select( max_sd + 1 , &readfds , &writefds , NULL , NULL)) < 0) {
     		fprintf(stderr, "Error on select function. \n");
			return EXIT_FAILURE;
     	}
     	printf("number of ready socket descriptor is %d\n", activity);

     	/* if something happened on master socket, meaning a new incoming connection */
     	if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, (struct sockaddr *) &addr,	&addr_size)) == -1) {
				close(master_socket);
				fprintf(stderr, "Error accepting connection.\n");
				return EXIT_FAILURE;
			}

			fprintf(stdout, "Accepted a new socket.\n");

			/* add new socket to our client sockets list */
			for (i = 0; i < NUM_CLIENTS; i++) {
				if (client_sockets[i] == 0) {
					client_sockets[i] = new_socket;
					fprintf(stdout, "Add new socket into the list at position %d.\n", i);
					break;
				}
			}
		}

		for (i = 0; i < NUM_CLIENTS; i++) {
			sd = client_sockets[i];
			if (FD_ISSET(sd, &readfds)) {
				printf("socket descriptor #%d is ready to be read\n", sd);
				readret = 0;

				if ((readret = recv(sd, buf, BUF_SIZE, 0)) > 0) {
					int j;
					for (j = 0; j < readret; j++)
						fprintf(stdout, "%c", buf[j]);
					fprintf(stdout, "\n");
					printf("finish one reading session at sd #%d\n", sd);
				}

				if (readret == 0) {
					client_sockets[i] = 0;
					FD_CLR(sd, &readfds);
					FD_CLR(sd, &writefds);
					printf("Securely close socket connection at sd #%d\n", sd);
				} 

				if (readret == -1)
				{
					close_socket(sd);
					fprintf(stderr, "Error reading from client socket.\n");
					return EXIT_FAILURE;
				}
			}

			if (FD_ISSET(sd, &writefds) && readret > 0) {
				printf("socket descriptor #%d is ready to be write\n", sd);
					if (send(sd, buf, readret, 0) != readret)
					{
						close_socket(sd);
						fprintf(stderr, "Error sending to client.\n");
						return EXIT_FAILURE;
					}
				memset(buf, 0, BUF_SIZE);
				readret = 0;
				printf("finish one writing session at sd #%d\n", sd);
			}
		}

	}

	close_socket(master_socket);

	return EXIT_SUCCESS;
}
