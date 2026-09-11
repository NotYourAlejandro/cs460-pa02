#include "client.h"

/************************************************************************
 * MAIN
 ************************************************************************/
int main(int argc, char * argv[])
{
	char input[100];                    // buffer for user input
	int client_socket;                  // client side socket
	struct sockaddr_in client_address;  // client socket naming struct
	int server_port = atoi(argv[2]);
	char * server_ip = argv[1];
	char c;

	printf("Echo client\n");

	// create an unnamed socket
	if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	// create addr struct
	client_address.sin_family      = AF_INET;
	client_address.sin_port        = htons(server_port);
	//client_address.sin_addr.s_addr = inet_addr(server_ip); // legacy inet_addr
	if (inet_pton(AF_INET, server_ip, &client_address.sin_addr) != 1) 
	{
		fprintf(stderr, "Invalid IP address\n");
		exit(EXIT_FAILURE);
	}

	// connect to server socket
	if (connect(client_socket, (struct sockaddr *)&client_address, sizeof(client_address)) == -1)
	{
		perror("Error connecting to server!\n");
		exit(EXIT_FAILURE);
	}

	while (TRUE)
	{
		printf("Input: ");
		// read string
		fgets(input, sizeof(input), stdin);

		int i = 0;
		while (*(input + i))
		{
			// make the request to the server
			write(client_socket, input + i, sizeof(char));
			// get the result
			read(client_socket, &c, sizeof(char));
			if (c == 'q')
			{
				close(client_socket);
				printf("\nDone!\n");
				exit(EXIT_SUCCESS);
			}
			printf("%c", c);
			i++;
		}
	}

	return EXIT_SUCCESS;
}

