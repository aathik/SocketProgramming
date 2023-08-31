/*
	Team Members:
	1. Aathik Thayyil Radhakrishnan - 110094762
	2. Nissy Saju - 110097704

	Section 3

	client.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>

// Here, the code initialises arrays to hold user commands and their arguments as well as default server IP addresses.

char *IP_ADDRESS_MAIN = "127.0.0.1", *IP_ADDRESS_MIRR = "127.0.0.1", *cmds[64];
int total_args = 0;

/*
This function effectively removes both the leading and following spaces from the user_string,
ensuring that the modified string is null-terminated properly.
*/

void removeOuterSpaces(char *user_string)
{
	int x = 0, y = 0;
	while (user_string[x] == ' ')
		x++;

	while (user_string[x] != '\0')
	{
		user_string[y] = user_string[x];
		x++;
		y++;
	}

	user_string[y] = '\0';
	y--;

	while (user_string[y] == ' ')
		y--;
	user_string[y + 1] = '\0';
}

/*
SplitIntoTokens is a function that tokenizes a specified input string, user_string,
depending on space delimiters. It divides the input string into individual "tokens"
and stores them in a cmds array. Additionally, it uses the variable total_args to
keep track of the total amount of tokens processed.
*/

void splitIntoTokens(char *user_string)
{

	total_args = 0;
	char *segment = strtok(user_string, " ");
	while (segment != NULL)
	{
		cmds[total_args] = segment;
		segment = strtok(NULL, " ");
		total_args++;
	}
}

/*numCheck is a function that determines whether a provided input string, user_string,
represents a valid numerical value made up of digits. It iteratesthrough the input text,
determining whether all characters are numbers.
*/

int numCheck(char *user_string)
{

	for (int x = 0; user_string[x] != '\0'; x++)
	{
		if (!isdigit(user_string[x]))
			return 0;
	}
	return 1;
}

/*
This function accepts a date string in the form "yyyy-mm-dd" and verifies that it meets a number of criteria,
including being valid for the day, month, and year. It also takes the different days of February due to leap
years into account. If the date is valid, the method returns 1, otherwise it returns 0.
*/

int checkDateValidity(char *user_string)
{
	int d, m, yr;

	if (sscanf(user_string, "%d-%d-%d", &yr, &m, &d) != 3)
		return 0;
	if (d < 1 || m < 1 || m > 12 || yr < 0)
		return 0;

	int month_total_days = 31;
	if (m == 4 || m == 6 || m == 9 || m == 11)
		month_total_days = 30;
	else if (m == 2)
	{

		if (yr % 4 == 0 && (yr % 100 != 0 || yr % 400 == 0))
			month_total_days = 29;
		else
			month_total_days = 28;
	}

	if (d > month_total_days)
		return 0;
	return 1;
}

/*
This function compares two date strings in the format "yyyy-mm-dd" by parsing the year, month,
and day components from both strings and performing comparisons. If the first date is greater
than the second date, the function returns 0; otherwise, it returns 1.
*/

int compareDateStrings(char *first_date, char *second_date)
{
	int first_d, first_m, first_yr, second_d, second_m, second_yr;

	sscanf(first_date, "%d-%d-%d", &first_yr, &first_m, &first_d);
	sscanf(second_date, "%d-%d-%d", &second_yr, &second_m, &second_d);

	if (first_yr > second_yr)
		return 0;
	else if (first_yr == second_yr && first_m > second_m)
		return 0;
	else if (first_yr == second_yr && first_m == second_m && first_d > second_d)
		return 0;
	else
		return 1;
}

/*
This function validates and verifies the authenticity of user commands depending on the
conditions specified for each command. It guarantees that the command structure, arguments,
and conditions are obeyed correctly. The method returns 1 if the command is legitimate;
else, it returns 0. It covers numerous scenarios for various commands and their inputs,
as well as checking for numerical values and date correctness.
*/

int checkCmdValidity()
{

	// Check if the first argument of the command matches any of the valid commands
	if (strcmp(cmds[0], "filesrch") == 0 || strcmp(cmds[0], "tarfgetz") == 0 || strcmp(cmds[0], "getdirf") == 0 ||
		strcmp(cmds[0], "fgets") == 0 || strcmp(cmds[0], "targzf") == 0 || strcmp(cmds[0], "quit") == 0)
	{

		if (strcmp(cmds[0], "filesrch") == 0)
		{

			// If the command is "filesrch", ensure that the total number of arguments is 2
			if (total_args != 2)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: filesrch <filename>\n\n");
				return 0;
			}
			return 1;
		}

		else if (strcmp(cmds[0], "tarfgetz") == 0)
		{

			// If the command is "tarfgetz", ensure that the total number of arguments is 3 or 4
			if (total_args < 3 || total_args > 4)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: tarfgetz <size1> <size2> <-u>\n\n");
				return 0;
			}

			// Check if the size arguments are valid numbers
			else if (numCheck(cmds[1]) == 0 || numCheck(cmds[2]) == 0)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: tarfgetz <size1> <size2> <-u>\n");
				printf("Interger size required!!!\n\n");
				return 0;
			}

			// Check if size1 is less than size2
			else if (atoi(cmds[1]) > atoi(cmds[2]))
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: tarfgetz <size1> <size2> <-u>\n");
				printf("Required: size1 <= size2 !!!\n\n");
				return 0;
			}

			// If the total_args is 4 and the last argument is not "-u", print usage
			else if (total_args == 4 && strcmp(cmds[total_args - 1], "-u") != 0)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: tarfgetz <size1> <size2> <-u>\n\n");
				return 0;
			}

			return 1;
		}

		else if (strcmp(cmds[0], "getdirf") == 0)
		{

			// If the command is "getdirf", ensure that the total number of arguments is 3 or 4
			if (total_args < 3 || total_args > 4)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: getdirf <date1> <date2> <-u>\n\n");
				return 0;
			}

			// Check if the date arguments are valid
			else if (checkDateValidity(cmds[1]) == 0 || checkDateValidity(cmds[2]) == 0)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: getdirf <date1> <date2> <-u>\n");
				printf("Provide a legit date!!!\n\n");
				return 0;
			}

			// Check if date1 is older than date2
			else if (compareDateStrings(cmds[1], cmds[2]) == 0)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: getdirf <date1> <date2> <-u>\n");
				printf("Required: Date1 > Date2 !!!\n\n");
				return 0;
			}

			// If the total_args is 4 and the last argument is not "-u", print usage
			else if (total_args == 4 && strcmp(cmds[total_args - 1], "-u") != 0)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: tarfgetz <size1> <size2> <-u>\n\n");
				return 0;
			}

			return 1;
		}

		else if (strcmp(cmds[0], "fgets") == 0)
		{

			// If the command is "fgets", ensure that the total number of arguments is 2 to 5
			if (total_args < 2 || total_args > 5)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: fgets <file1> <file2> <file3> <file4>\n\n");
				return 0;
			}

			return 1;
		}

		else if (strcmp(cmds[0], "targzf") == 0)
		{

			// If the command is "targzf", ensure that the total number of arguments is 2 to 6
			if (total_args < 2 || total_args > 6)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: targzf <extension1> <extension2> <extension3> <extension4> <-u>\n\n");
				return 0;
			}

			// If the total_args is 6 and the last argument is not "-u", print usage
			else if (total_args == 6 && strcmp(cmds[total_args - 1], "-u") != 0)
			{
				printf("\nInvalid usage. Not the Correct format!!! \nCorrect format is: targzf <extension1> <extension2> <extension3> <extension4> <-u>\n\n");
				return 0;
			}

			return 1;
		}

		return 1; // Valid command other than the specified ones
	}

	else
	{
		printf("!!!! Not a valid command !!!!\n");
		return 0; // Invalid command
	}
}

/*
The response from the server is handled by this function. It determines if the answeris
data or a file and then processes it appropriately. It displays the data if the response
is non-file data. If the answer is a file, it receives and writes the file data to the
specified target file; otherwise, it extracts the tar.gz file if the "-u" flag is supplied.
The function also handles errors at various phases of data reception, writing, and processing.
*/

void processResponseFromServer(int skt_client_comm, char *target_fname)
{
	char nis_data[1024], flag_nis_data[1024];
	off_t read_byte_count = 0, data_packet_size;

	printf("--[WAIT] Waiting for response from client\n");

	// Receive the flag data from the server
	recv(skt_client_comm, &flag_nis_data, sizeof(flag_nis_data), 0);
	char msg_flag[10];
	strncpy(msg_flag, flag_nis_data, 10);

	// Flags to indicate if data is a file or not
	char data_is_file_flag[10] = "FILETOCLNT", non_file_flag[10] = "ERRORMESSG";

	// Check if the received data is a non-file response
	if (strncmp(msg_flag, non_file_flag, 10) == 0)
	{

		printf("++[SUCCESS] Response Received from client \n\n");
		sleep(1);
		send(skt_client_comm, "flagReceived", 12, 0);

		int bytes_recieved = recv(skt_client_comm, &nis_data, sizeof(nis_data), 0);
		nis_data[bytes_recieved] = '\0';
		printf("--%s\n", nis_data);
		return;
	}

	// Check if the received data is a file response
	else if (strncmp(msg_flag, data_is_file_flag, 10) == 0)
	{

		printf("++[SUCCESS] Response Received \n\n");
		sleep(1);
		send(skt_client_comm, "flagReceived", 12, 0);

		// Open the file for writing
		int file_descriptor = open(target_fname, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

		if (file_descriptor == -1)
		{
			printf("--[FAILED] The file could not be opened \n");
			close(file_descriptor);
		}
		else
		{

			// Receive the size of the data packet
			recv(skt_client_comm, &data_packet_size, sizeof(data_packet_size), 0);

			// Loop to receive and write data to the file
			while (read_byte_count < data_packet_size)
			{

				ssize_t received_byte_count = recv(skt_client_comm, nis_data, 1024, 0);
				if (received_byte_count == -1)
				{
					printf("--[FAILED] Data Reception Error Occured ");
					break;
				}

				ssize_t output_byte_count = write(file_descriptor, nis_data, received_byte_count);
				if (output_byte_count == -1)
				{
					printf("--[FAILED] Data Writing Error Occured ");
					break;
				}

				if (output_byte_count < received_byte_count)
				{
					printf("--[FAILED] Error Occured while writing/outputing data to a file \n");
					break;
				}

				read_byte_count = read_byte_count + received_byte_count;
			}

			close(file_descriptor);

			// If the "-u" flag is provided, extract the tar.gz file
			if (strcmp(cmds[total_args - 1], "-u") == 0)
			{
				char user_string[200];
				sprintf(user_string, "tar -xzf %s", target_fname);
				system(user_string);
			}
		}
	}
}

/*
This function opens a connection to a server based on its categorization. It checks first
to see if the category is "M" (Mirror Server) or "P" (Primary Server). It generates a socket,
configures the server address, and starts a connection for each category. If neither "M" nor
"P" is selected, an error message is displayed. The socket descriptorfor communication with
the selected server is returned by the function.
*/

int initializeServerConnection(char *category_of_srvr)
{
	int skt_client_comm, return_val;
	struct sockaddr_in address_mirr_srvr;
	struct sockaddr_in address_main_srvr;

	// Check if the server category is "M" (Mirror Server)
	if (strcmp(category_of_srvr, "M") == 0)
	{

		// Create a socket for communication
		skt_client_comm = socket(AF_INET, SOCK_STREAM, 0);
		if (skt_client_comm < 0)
		{
			printf("--[FAILED] A connection error has occurred \n");
			exit(1);
		}

		// Configure the address for the Mirror Server
		memset(&address_mirr_srvr, '\0', sizeof(address_mirr_srvr));
		address_mirr_srvr.sin_family = AF_INET;
		address_mirr_srvr.sin_port = htons(7777);
		address_mirr_srvr.sin_addr.s_addr = inet_addr(IP_ADDRESS_MIRR);

		// Establish a connection to the Mirror Server
		return_val = connect(skt_client_comm, (struct sockaddr *)&address_mirr_srvr, sizeof(address_mirr_srvr));
		if (return_val < 0)
		{
			printf("--[FAILED] A connection error has occurred @^@^@ Could not establish a connection with Mirror Server\n");
			exit(1);
		}
		printf("++[SUCCESS] Connection to Mirror Server has been established succesfully\n");
	}

	// Check if the server category is "P" (Primary Server)
	else if (strcmp(category_of_srvr, "P") == 0)
	{

		// Create a socket for communication
		skt_client_comm = socket(AF_INET, SOCK_STREAM, 0);
		if (skt_client_comm < 0)
		{
			printf("--[FAILED] A connection error has occurred \n");
			exit(1);
		}

		printf("++[SUCCESS] Successful creation of client socket\n");

		// Configure the address for the Primary Server
		memset(&address_main_srvr, '\0', sizeof(address_main_srvr));
		address_main_srvr.sin_family = AF_INET;
		address_main_srvr.sin_port = htons(5555);
		address_main_srvr.sin_addr.s_addr = inet_addr(IP_ADDRESS_MAIN);

		// Establish a connection to the Primary Server
		return_val = connect(skt_client_comm, (struct sockaddr *)&address_main_srvr, sizeof(address_main_srvr));
		if (return_val < 0)
		{
			printf("--[FAILED] A connection error has occurred @^@^@ Could not establish a connection with Primary Server\n");
			exit(1);
		}
	}

	// Invalid server category
	else
	{
		printf("--[FAILED] The Server Category does not exist !!!!\n");
		exit(1);
	}
	return skt_client_comm; // Return the socket descriptor for communication
}

/*
The main function establishes a connection with the server, processes user commands,
and manages server responses. It checks command-line arguments, connects to the server
(Primary or Mirror), and enters a loop to interact with the user. The loop continues
until the "quit" command is entered.
*/

int main(int input_count, char *input_value[])
{
	// Check for correct number of command-line arguments
	if (input_count != 3 && input_count != 1)
	{
		printf("Please provide %s with <Server IP> <Mirror IP>\n", input_value[0]);
		exit(1);
	}

	// If two IP addresses are provided, assign them to respective variables
	else if (input_count == 3)
	{

		IP_ADDRESS_MAIN = input_value[1];
		IP_ADDRESS_MIRR = input_value[2];
	}

	int skt_client_comm;
	char data_nis_buffr[1024], user_string[1024], nis_data[1024];

	// Initialize a connection with the Primary Server
	skt_client_comm = initializeServerConnection("P");

	// Write 'c' to the server to initiate the communication
	write(skt_client_comm, "c", 1);
	int m = read(skt_client_comm, nis_data, 1);
	nis_data[m] = '\0';

	// Check whether the server is Primary or Mirror
	if (strcmp(nis_data, "P") == 0)
	{
		printf("++[SUCCESS] Successfully established connection to the PRIMARY SERVER !!!!\n");
	}
	else if (strcmp(nis_data, "M") == 0)
	{
		close(skt_client_comm);
		skt_client_comm = initializeServerConnection("M");
	}
	else
	{
		printf("--[FAILED] ISSUE IN PRIMARY SERVER OCCURED: Command not recognised in response !!!!\n");
		exit(1);
	}

	while (1)
	{
		memset(nis_data, 0, 1024);

		printf("C$ ");
		scanf(" %[^\n]s", &data_nis_buffr[0]);
		strcpy(user_string, data_nis_buffr);

		removeOuterSpaces(user_string);
		splitIntoTokens(user_string);

		// Validate the user's command
		int validation_status = checkCmdValidity();

		// If command is valid, send it to the server
		if (validation_status == 1)
			send(skt_client_comm, data_nis_buffr, strlen(data_nis_buffr), 0);
		else
			continue;

		// Check for the "quit" command
		if (strcmp(data_nis_buffr, "quit") == 0)
		{
			close(skt_client_comm);
			printf("^^^ QUIT COMMAND GIVEN: SERVER HAS BEEN SUCCESSFULLY DISCONNECTED ^^^\n");
			exit(1);
		}

		// Process responses from the server based on different commands
		if (strcmp(cmds[0], "tarfgetz") == 0)
			processResponseFromServer(skt_client_comm, "temp.tar.gz");
		else if (strcmp(cmds[0], "getdirf") == 0)
			processResponseFromServer(skt_client_comm, "temp.tar.gz");
		else if (strcmp(cmds[0], "fgets") == 0)
			processResponseFromServer(skt_client_comm, "temp.tar.gz");
		else if (strcmp(cmds[0], "targzf") == 0)
			processResponseFromServer(skt_client_comm, "temp.tar.gz");
		else
		{

			if (recv(skt_client_comm, nis_data, 1024, 0) < 0)
				printf("--[FAILED] DATA RECPTION ERROR HAS BEEN OCCURED !!!!\n");
			else
				printf("%s\n", nis_data);
		}
	}
	return 0;
}