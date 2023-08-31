/*
	Team Members:
	1. Aathik Thayyil Radhakrishnan - 110094762
	2. Nissy Saju - 110097704

	Section 3

	server.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

/*
 Constants and Global Variables are declared here.
*/
#define ATR_MAX_COMM_LEN 1000
#define ATR_BUFF_SIZE 1000
#define ATR_PORT_NUM 5555

char *atr_home_path, *atr_commands[100];
int atr_num_of_arguments = 0, atr_num_of_P_Clients = 0, atr_num_of_M_Clients = 0, atr_total_num_of_clients = 0;
int atr_type_of_sever = 1; // Primary server is 1 and Mirror Server is 0

/*
atr_check_If_File_Exists Function is used to check if a particular file is present in the current working directory or not using access().
It returns 1 if the file exists and 0 if it does not
*/
int atr_check_If_File_Exists(char *atr_fileName)
{
	if (access(atr_fileName, F_OK) != -1)
	{
		// File Exists
		return 1;
	}
	else
	{
		// File doesn't exist
		return 0;
	}
} // end atr_check_If_File_Exists

/*
atr_split_string function is used to split 'atr_str' into tokens and store it in atr_commands
*/
void atr_split_string(char *atr_str)
{
	atr_num_of_arguments = 0;
	char *atr_tok = strtok(atr_str, " ");
	while (atr_tok != NULL)
	{
		atr_commands[atr_num_of_arguments] = atr_tok;
		atr_num_of_arguments++;
		atr_tok = strtok(NULL, " ");
	}
} // end atr_split_string()

/*
 atr_execute_command function is used to execute the command using system().
 It returns 1 if the command fails and 0 if successful
*/
int atr_execute_command(char *atr_command)
{
	int atr_check = system(atr_command); // system() call returns 0 if successful
	if (atr_check != 0)
	{
		printf("\n-----Command Execution Failed-----\n");
		return 1;
	}
	else
	{
		printf("\n-----Command Executed Successfully----\n");
		return 0;
	}
} // end atr_execute_command()

/*
atr_send_response_to_client Function sends a a response to the client using the socket.  It takes 3 parameters:
	1. atr_socket: It represents a socket descriptor that is used for sending data to and receiving data from a client.
	2. atr_send_file_flag: If the atr_send_file_flag is 1, the function sends the file to the client. Otherwise it sends an error message.
	3. atr_fileName:
The function uses send() and recv() for the communication in the TCP/IP protocol.
*/
void atr_send_response_to_client(int atr_socket, int atr_send_file_flag, char *atr_fileName)
{
	if (atr_send_file_flag)
	{
		// To send File to the Client
		char atr_check_flag[10] = "FILETOCLNT"; // Flag to be sent to the Client
		while (1)
		{
			printf("\nSending the flag to the Client: %s\n", atr_check_flag);

			// Sending the atr_check_flag to the client with the help of the socket using send()
			send(atr_socket, atr_check_flag, strlen(atr_check_flag), 0);

			char atr_acknowledgement[12];
			printf("* * * * Waiting for Client's Acknowledgement * * * * \n");
			// Recieve the acknowledgement from the client with the help of the socket using recv() and store it in atr_acknowledgement
			recv(atr_socket, atr_acknowledgement, 12, 0);
			printf("Client's Acknowledgement: %s\n", atr_acknowledgement);
			if (strncmp(atr_acknowledgement, "flagReceived", 12) == 0)
			{
				break;
			}
		}

		/*
		The following code is to read the contents of the temp.tar.gz into atr_data and send it to the client accordingly
		*/
		// Use open() to open the file temp.tar.gz in read mode
		int atr_file_descriptor = open(atr_fileName, O_RDONLY);
		char atr_data[ATR_BUFF_SIZE];
		ssize_t atr_bytes_read;
		if (atr_file_descriptor == -1)
		{
			printf("--[FAILED] error in opening the file in read mode\n");
			exit(0);
		}

		/*
		The following code is to send the size of the file to the client
		*/
		struct stat atr_info_of_file; // Declare a variable atr_info_of_file of type struct stat to store the information of the file
		/*
		Check if the file exists and can be accessed using stat(), if yes it stores the information regarding the file in atr_info_of_file
		*/
		if (stat(atr_fileName, &atr_info_of_file) >= 0)
		{
			off_t atr_file_size = atr_info_of_file.st_size; // Get the size of the file in bytes and store it in
			send(atr_socket, &atr_file_size, sizeof(atr_file_size), 0);
		}
		else
		{
			// If file doesnt exist or is not accessible, exit with error message
			perror("--[FAILED] ERROR in STAT function");
			exit(EXIT_FAILURE);
		}

		// This is the loop to read the entire contents of the file
		while ((atr_bytes_read = read(atr_file_descriptor, atr_data, ATR_BUFF_SIZE)) > 0)
		{
			// To send the contents of the file to the client
			ssize_t atr_bytes_sent_to_client = send(atr_socket, atr_data, atr_bytes_read, 0);
			// Check for any mismatch in the sent data
			if (atr_bytes_sent_to_client < atr_bytes_read)
			{
				printf("Mismatch in data sent to the client\n");
				exit(0);
			}
		}

		// To remove the temp.tar.gz file from the server side
		remove(atr_fileName);
		// To close the file descriptor
		close(atr_file_descriptor);
	}
	else
	{
		// To send the error message to the Client
		char atr_check_flag[10] = "ERRORMESSG"; // Flag to be sent to the Client
		while (1)
		{

			// Sending the atr_check_flag to the client with the help of the socket using send()
			send(atr_socket, atr_check_flag, strlen(atr_check_flag), 0);

			// Recieve the acknowledgement from the client with the help of the socket using recv() and store it in atr_acknowledgement

			char atr_acknowledgement[12];
			recv(atr_socket, atr_acknowledgement, 12, 0);
			printf("Client's Acknowledgement: %s\n", atr_acknowledgement);
			if (strncmp(atr_acknowledgement, "flagReceived", 12) == 0)
			{
				break;
			}
		}

		// send the error message to client
		char *atr_error = "[FAILED] FILE NOT FOUND ! ! ! ";
		send(atr_socket, atr_error, strlen(atr_error), 0);
	}
} // end atr_send_response_to_client()

/*
 atr_filesrch function is used to to search for a specific file in a given directory and return some information about it namely the fileName, File size in Bytes,and the date the file was created.
*/
void atr_filesrch(int atr_socket)
{

	// Declare fileStat of type Struct stat to store the information about the file
	struct stat fileStat;
	char atr_linux_cmd[ATR_MAX_COMM_LEN], atr_result[ATR_MAX_COMM_LEN];
	// Create the respective linux command for the particular action and store it in atr_linux_cmd
	snprintf(atr_linux_cmd, sizeof(atr_linux_cmd), "find %s -name \"%s\" -type f -print -exec ls -lh --time-style=+\"%%Y-%%m-%%d %%H:%%M:%%S\" {} \\; 2>/dev/null | head -n1", atr_home_path, atr_commands[1]);
	// Use popen() to execute the command in read mode
	FILE *atr_file_desc_P = popen(atr_linux_cmd, "r");
	if (atr_file_desc_P == NULL)
	{
		// ERROR file descriptor atr_file_desc_P is NULL
		perror("--[FAILED] ERROR in popen()");
	}
	// Read the result of the linux command line-by-line and store it into atr_result and send it to the client
	if (fgets(atr_result, sizeof(atr_result), atr_file_desc_P) != NULL)
	{
		atr_result[strcspn(atr_result, "\n")] = 0;
		// To ge the file information
		stat(atr_result, &fileStat);
		printf("\nFileName: %s , Size(in Bytes): %lld , Date modified: %s\n", atr_result, fileStat.st_size, ctime(&fileStat.st_mtime));
		char *atr_msg = malloc(ATR_MAX_COMM_LEN * sizeof(char));
		sprintf(atr_msg, "\nFileName: %s , Size(in Bytes): %lld , Date modified: %s\n", atr_result, fileStat.st_size, ctime(&fileStat.st_mtime));
		printf("%s", atr_msg);
		// Send atr_msg to the client using the socket descriptor atr_socket
		send(atr_socket, atr_msg, strlen(atr_msg), 0);
	}
	else
	{
		// If the file doesnt exist, send the corresponding atr_msg to the client
		char *atr_msg = " [FAILED] FILE NOT FOUND ! ! ! ";
		send(atr_socket, atr_msg, strlen(atr_msg), 0);
	}
	// To close the file descriptor atr_file_desc_P
	pclose(atr_file_desc_P);
} // end atr_filesrch()

/*
	atr_tarfgetz function is used to find specific files whose file-sizes are inbetween size1 and size2 in a directory, compress them into a tar.gz file, and then send a response to a client indicating whether the file was created successfully or not
*/
void atr_tarfgetz(int atr_socket)
{
	char *atr_type_of_operation = "tarfgetz";
	char atr_linux_cmd[ATR_MAX_COMM_LEN];
	char atr_fileName[500];
	int atr_send_file_flag = 0; // Flag to suggest whether a file will be sent to the client

	// Create the respective linux command for the particular action and store it in atr_linux_cmd
	char *atr_result = malloc(ATR_MAX_COMM_LEN * sizeof(char));
	int lowerlimit = atoi(atr_commands[1]) - 1;
	int upperlimit = atoi(atr_commands[2]) + 1;
	sprintf(atr_result, "find %s -type f -name '*.*' -size +%dc -size -%dc -print0 ", atr_home_path, lowerlimit, upperlimit);
	sprintf(atr_linux_cmd, "%s | if grep -q . ; then %s | tar -czf %s_%d.tar.gz --null -T -  ; fi", atr_result, atr_result, atr_type_of_operation, atr_socket);

	// Execute the linux command using atr_execute_command function
	if (atr_execute_command(atr_linux_cmd) == 1)
	{
		// If function returns 1, there was an error
		exit(1);
	}
	else if (atr_execute_command(atr_linux_cmd) == 0)
	{
		// If the function returns 0, the command was executed successfully
		// Store the file name of archive file in atr_fileName
		sprintf(atr_fileName, "%s_%d.tar.gz", atr_type_of_operation, atr_socket);

		// checks if the file was successfully created using the atr_check_If_File_Exists function. If the file exists, it sets the atr_send_file_flag to 1; otherwise, it sets it to 0.
		if (atr_check_If_File_Exists(atr_fileName) == 1)
		{
			atr_send_file_flag = 1;
		}
		else
		{
			atr_send_file_flag = 0;
		}

		// Use atr_send_response_to_client to send the response to the client with the socketDescripto, atr_send_file_flag and the fileName
		atr_send_response_to_client(atr_socket, atr_send_file_flag, atr_fileName);
	}
} // end atr_tarfgetz()

/*
	atr_getdirf function is used to find and send specific files whose date of creation is <=date2 and >=date1 (date1<=date2) from a server to a client over a socket connection.
*/
void atr_getdirf(int atr_socket)
{
	char *atr_type_of_operation = "getdirf";
	char atr_linux_cmd[ATR_MAX_COMM_LEN];
	char atr_fileName[500];
	int atr_send_file_flag = 0; // Flag to suggest whether a file will be sent to the client

	// Create the respective linux command for the particular action and store it in atr_linux_cmd
	char *atr_result = malloc(ATR_MAX_COMM_LEN * sizeof(char));
	sprintf(atr_result, "find %s -type f -name '*.*' -newermt '%s 00:00:00' ! -newermt '%s 23:59:59' -print0", atr_home_path, atr_commands[1], atr_commands[2]);
	sprintf(atr_linux_cmd, "%s | if grep -q . ; then %s | tar -czf %s_%d.tar.gz --null -T - ; fi", atr_result, atr_result, atr_type_of_operation, atr_socket);

	// Execute the linux command using atr_execute_command function
	if (atr_execute_command(atr_linux_cmd) == 1)
	{
		// If function returns 1, there was an error
		exit(1);
	}
	else if (atr_execute_command(atr_linux_cmd) == 0)
	{
		// If the function returns 0, the command was executed successfully
		// Store the file name of archive file in atr_fileName
		sprintf(atr_fileName, "%s_%d.tar.gz", atr_type_of_operation, atr_socket);

		// checks if the file was successfully created using the atr_check_If_File_Exists function. If the file exists, it sets the atr_send_file_flag to 1; otherwise, it sets it to 0.
		if (atr_check_If_File_Exists(atr_fileName) == 1)
		{
			atr_send_file_flag = 1;
		}
		else
		{
			atr_send_file_flag = 0;
		}

		// Use atr_send_response_to_client to send the response to the client with the socketDescripto, atr_send_file_flag and the fileName
		atr_send_response_to_client(atr_socket, atr_send_file_flag, atr_fileName);
	}
} // end atr_getdirf()

/*
	atr_fgetS function is used to find specific files in a directory, compress them into a tar.gz file, and then send a response to a client indicating whether the file was created successfully or not
*/
void atr_fgetS(int atr_socket)
{
	char *atr_type_of_operation = "fgets";
	char atr_linux_cmd[ATR_MAX_COMM_LEN], atr_fileName[500], atr_files[500];
	int atr_send_file_flag = 0; // Flag to suggest whether a file will be sent to the client

	// Create the respective linux command for the particular action and store it in atr_linux_cmd
	if (strcmp(atr_type_of_operation, "fgets") == 0)
	{
		sprintf(atr_linux_cmd, "find %s -type f '('", atr_home_path);
		if (strcmp(atr_commands[atr_num_of_arguments - 1], "-u") == 0)
		{
			// If the last argument is -u
			for (int loop_cntr = 1; loop_cntr < atr_num_of_arguments - 1; loop_cntr++)
			{
				if (loop_cntr != atr_num_of_arguments - 2)
				{
					sprintf(atr_files, " -name '%s' -o", atr_commands[loop_cntr]);
				}
				else
				{
					sprintf(atr_files, " -name '%s'", atr_commands[loop_cntr]);
				}
				strcat(atr_linux_cmd, atr_files);
			}
		}
		else
		{
			// If the last argument is not -u
			for (int loop_cntr = 1; loop_cntr < atr_num_of_arguments; loop_cntr++)
			{
				if (loop_cntr != atr_num_of_arguments - 1)
				{
					sprintf(atr_files, " -name '%s' -o", atr_commands[loop_cntr]);
				}
				else
				{
					sprintf(atr_files, " -name '%s'", atr_commands[loop_cntr]);
				}
				strcat(atr_linux_cmd, atr_files);
			}
		}
		sprintf(atr_files, " ')' -print0 ");
		strcat(atr_linux_cmd, atr_files);
		sprintf(atr_files, " | if grep -q . ; then %s |", atr_linux_cmd);
		strcat(atr_linux_cmd, atr_files);
		sprintf(atr_files, " tar -czf %s_%d.tar.gz --null -T - ; fi", atr_type_of_operation, atr_socket);
		strcat(atr_linux_cmd, atr_files);
	}
	// Execute the linux command using atr_execute_command function
	if (atr_execute_command(atr_linux_cmd) == 1)
	{
		// If function returns 1, there was an error
		exit(1);
	}
	else if (atr_execute_command(atr_linux_cmd) == 0)
	{
		// If the function returns 0, the command was executed successfully
		// Store the file name of archive file in atr_fileName
		sprintf(atr_fileName, "%s_%d.tar.gz", atr_type_of_operation, atr_socket);

		// checks if the file was successfully created using the atr_check_If_File_Exists function. If the file exists, it sets the atr_send_file_flag to 1; otherwise, it sets it to 0.
		if (atr_check_If_File_Exists(atr_fileName) == 1)
		{
			atr_send_file_flag = 1;
		}
		else
		{
			atr_send_file_flag = 0;
		}

		// Use atr_send_response_to_client to send the response to the client with the socketDescripto, atr_send_file_flag and the fileName
		atr_send_response_to_client(atr_socket, atr_send_file_flag, atr_fileName);
	}
} // end atr_fgetS()

/*
	atr_targzf function is used to find and send specific files belonging to the file-types listed in the <extension-list>, from a server to a client over a socket connection.
*/
void atr_targzf(int atr_socket)
{
	char *atr_type_of_operation = "targzf";
	char atr_linux_cmd[ATR_MAX_COMM_LEN];
	char atr_fileName[500];
	char atr_files[500];
	int atr_send_file_flag = 0;

	// Create the respective linux command for the particular action and store it in atr_linux_cmd
	if (strcmp(atr_type_of_operation, "targzf") == 0)
	{
		sprintf(atr_linux_cmd, "find %s -type f '('", atr_home_path);
		if (strcmp(atr_commands[atr_num_of_arguments - 1], "-u") == 0)
		{
			// If the last argument is -u
			for (int loop_cntr = 1; loop_cntr < atr_num_of_arguments - 1; loop_cntr++)
			{
				if (loop_cntr != atr_num_of_arguments - 2)
				{
					sprintf(atr_files, " -name '*.%s' -o", atr_commands[loop_cntr]);
				}
				else
				{
					sprintf(atr_files, " -name '*.%s'", atr_commands[loop_cntr]);
				}
				strcat(atr_linux_cmd, atr_files);
			}
		}
		else
		{
			// If the last argument is not -u
			for (int loop_cntr = 1; loop_cntr < atr_num_of_arguments; loop_cntr++)
			{
				if (loop_cntr != atr_num_of_arguments - 1)
				{
					sprintf(atr_files, " -name '*.%s' -o", atr_commands[loop_cntr]);
				}
				else
				{
					sprintf(atr_files, " -name '*.%s'", atr_commands[loop_cntr]);
				}
				strcat(atr_linux_cmd, atr_files);
			}
		}
		sprintf(atr_files, " ')' -print0 ");
		strcat(atr_linux_cmd, atr_files);
		sprintf(atr_files, " | if grep -q . ; then %s |", atr_linux_cmd);
		strcat(atr_linux_cmd, atr_files);
		sprintf(atr_files, "tar -czf %s_%d.tar.gz --null -T - ; fi", atr_type_of_operation, atr_socket);
		strcat(atr_linux_cmd, atr_files);
	}
	// Execute the linux command using atr_execute_command function
	if (atr_execute_command(atr_linux_cmd) == 1)
	{
		// If function returns 1, there was an error
		exit(1);
	}
	else if (atr_execute_command(atr_linux_cmd) == 0)
	{
		// If the function returns 0, the command was executed successfully
		// Store the file name of archive file in atr_fileName
		sprintf(atr_fileName, "%s_%d.tar.gz", atr_type_of_operation, atr_socket);

		// checks if the file was successfully created using the atr_check_If_File_Exists function. If the file exists, it sets the atr_send_file_flag to 1; otherwise, it sets it to 0.
		if (atr_check_If_File_Exists(atr_fileName) == 1)
		{
			atr_send_file_flag = 1;
		}
		else
		{
			atr_send_file_flag = 0;
		}

		// Use atr_send_response_to_client to send the response to the client with the socketDescripto, atr_send_file_flag and the fileName
		atr_send_response_to_client(atr_socket, atr_send_file_flag, atr_fileName);
	}
} // end atr_targzf()

/*
	atr_server_controller finction decides on whether the particular client should be handled by the primary server or the mirror server.
	If atr_type_of_server = 0, the client will be handled by the mirror server.
	If atr_type_of_server = 1, the client will be handled by the primary server.
	The function returns atr_type_of_server
*/
int atr_server_controller()
{
	// Logic for the first 12 clients
	if (atr_total_num_of_clients <= 12)
	{
		// First 6 of the first 12 clients will be handled by primary server and then the next 6 clients by the mirror server.
		if (atr_total_num_of_clients % 6 == 0 && atr_total_num_of_clients != 0)
		{
			if (atr_type_of_sever == 0)
			{
				atr_type_of_sever = 1;
			}
			else
			{
				atr_type_of_sever = 0;
			}
		}
	}
	// For the clients that comes after the 12 clients, they are allocated alternatively to each server starting with the primary server.
	else
	{
		if (atr_total_num_of_clients % 2 == 0)
		{
			atr_type_of_sever = 1;
		}
		else
		{
			atr_type_of_sever = 0;
		}
	}
	return atr_type_of_sever;

} // end atr_server_controller()

/*
	processclient Function handles the connections for each client that connects to the server.
	It takes three parameters: atr_serverSide_socket, atr_socket, and atr_client_addr, which represent the server-side socket, client socket, and client address respectively.
*/
void processclient(int atr_serverSide_socket, int atr_socket, struct sockaddr_in atr_client_addr)
{
	char atr_msg_buffer[1100];
	// To create a child for each client to process its request
	pid_t atr_proc_id = fork();
	if (atr_proc_id == 0)
	{
		// Child Process
		// Close the Server Side Socket atr_serverSide_socket
		close(atr_serverSide_socket);

		// Loop that handles the client communication
		while (1)
		{
			// To flush or clear the atr_msg_buffer
			memset(atr_msg_buffer, '\0', sizeof(atr_msg_buffer));
			// Recieve the client request using recv() from each client using the socket descriptor atr_socket
			recv(atr_socket, atr_msg_buffer, ATR_BUFF_SIZE, 0);
			// Check if the request recieved is empty, anc continue if empty.
			if (strlen(atr_msg_buffer) == 0)
			{
				continue;
			}

			// For the client to exit, enter 'quit'
			if (strcmp(atr_msg_buffer, "quit") == 0)
			{
				// If the request is quit, close the server side socket and exit
				printf("Client No: %d, Server disconnected from client %s:%d\n", atr_total_num_of_clients, inet_ntoa(atr_client_addr.sin_addr), ntohs(atr_client_addr.sin_port));
				close(atr_serverSide_socket);
				exit(0);
				break;
			}
			else
			{
				// If the request is valid and not quit, we split the request message into tokens using the function atr_split_string
				atr_split_string(atr_msg_buffer);
				char *atr_client_request = malloc(strlen(atr_commands[0]) + 1);
				strcpy(atr_client_request, atr_commands[0]);
				atr_client_request[strlen(atr_client_request)] = '\0';
				char *atr_message = malloc(ATR_MAX_COMM_LEN * sizeof(char));

				/*
				Based on the command, the function calls different functions (atr_filesrch(), atr_tarfgetz(), atr_getdirf(), atr_fgetS(), atr_targzf()) to handle the client request.
				*/
				if (strcmp(atr_client_request, "filesrch") == 0)
				{
					atr_filesrch(atr_socket);
				}
				else if (strcmp(atr_client_request, "tarfgetz") == 0)
				{
					atr_tarfgetz(atr_socket);
				}
				else if (strcmp(atr_client_request, "getdirf") == 0)
				{
					atr_getdirf(atr_socket);
				}
				else if (strcmp(atr_client_request, "fgets") == 0)
				{
					atr_fgetS(atr_socket);
				}
				else if (strcmp(atr_client_request, "targzf") == 0)
				{
					atr_targzf(atr_socket);
				}
				// To clear or flush the atr_message buffer memory
				memset(atr_message, 0, sizeof(char) * sizeof(atr_message));
			}
		}
	}
} // end processclient()

/*
	atr_create_server function for creating and starting a server-side socket. It returns the socket descriptor upon successful creation process.
*/
int atr_create_server()
{
	int atr_serverSide_socket, atr_option = 1;
	struct sockaddr_in serverAddr;

	// Create the socket using socket()
	atr_serverSide_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (atr_serverSide_socket < 0)
	{
		printf("--[FAILED] Socket not created\n");
		exit(1);
	}
	printf("++[SUCCESS] Socket is created for the server\n");

	memset(&serverAddr, '\0', sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(ATR_PORT_NUM);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	// Set the Socket Options for the created socket atr_serverSide_socket
	if (setsockopt(atr_serverSide_socket, SOL_SOCKET, SO_REUSEADDR, &atr_option, sizeof(atr_option)))
	{
		printf("--[FAILED] Socket Failed at setsockopt()\n");
		exit(1);
	}
	// Bind the socket atr_serverSide_socket to defined options
	if (bind(atr_serverSide_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		printf("--[FAILED] Socket Failed at Binding\n");
		exit(1);
	}
	printf("++[SUCCESS] Socket binded to the port %d\n", ATR_PORT_NUM);

	// Listen for the clients using listen()
	if (listen(atr_serverSide_socket, 10) == 0)
	{
		printf("++[SUCCESS] Socket Listening Successfully..\n");
	}
	else
	{
		printf("--[FAILED] Socket failed at listen()\n");
		exit(1);
	}
	return atr_serverSide_socket;
} // end atr_create_server()

// This is the main function
int main()
{
	atr_home_path = getenv("HOME"); // Store the home directory
	int atr_serverSide_socket, atr_socket;
	struct sockaddr_in atr_client_addr;
	socklen_t addr_size;

	atr_serverSide_socket = atr_create_server();

	// For each client requsting for a connection, it goes in this infinite loop and waits until accept() is successful
	while (1)
	{
		// To accept the connection by each client to the server
		atr_socket = accept(atr_serverSide_socket, (struct sockaddr *)&atr_client_addr, &addr_size);
		if (atr_socket < 0)
		{
			// Failed at accept()
			exit(1);
		}
		printf("Client No-%d : Connection Established From %s:%d", atr_total_num_of_clients + 1, inet_ntoa(atr_client_addr.sin_addr), ntohs(atr_client_addr.sin_port));

		// Use the atr_server_controller to check which server should handle each individual clients
		int selectedServer = atr_server_controller();
		if (selectedServer)
		{
			// For the primary server
			atr_num_of_P_Clients++;
			// Send a "P" to the Client to indicate that it should maintain its connection to the Primary Server.
			write(atr_socket, "P", 1);
			atr_total_num_of_clients++;
			printf("\n");
		}
		else
		{
			// For the Mirror Server
			atr_num_of_M_Clients++;
			// Send a "M" to the client to instruct it to be redirected to the Mirror Server.
			write(atr_socket, "M", 1);
			close(atr_socket);
			atr_total_num_of_clients++;
			printf(" -> Redirect to the Mirror server\n");
			// Avoid processing the client because the Mirror Server will handle it.
			continue;
		}

		// processclient to handle the requests of each client
		processclient(atr_serverSide_socket, atr_socket, atr_client_addr);
	}
	// To close the socket atr_socket
	close(atr_socket);
	return 0;
} // end Main function