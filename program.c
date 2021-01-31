#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define MAX_CAHARACTER 100 // max number of letters to be supported

// message type message or error
typedef enum
{
	MESSAGE,
	ERROR
} MESSAGE_TYPE;

// recaive message from database
typedef struct
{
	char message[MAX_CAHARACTER];
	MESSAGE_TYPE message_type;
} message_t;

// remove character '\n' from string
// the reason I use this function is that when user enter a input, input function put '\n'
// character at the end of the string
void remove_newline_char(char *str);
// execute kaydet.c program and saving returned values to sonuc.txt file
void save(char *arr);
// control whether string is empty
// if string is empty, return 1
// otherwise return 0
int isEmpty(char *str);

int main()
{
	int fd;
	// FIFO file path
	char myfifo[] = "/tmp/myfifo";

	// creating the named file(FIFO)
	// mkfifo(<pathname>, <permission>)
	mkfifo(myfifo, 0666);

	char query[100] = {'\0'}; // query string

	message_t *msg = calloc(1, sizeof(message_t)); // return message from database

	while (1)
	{
		printf("Sorgu giriniz: "); // want a query from user
		fgets(query, 100, stdin); // gets query from user
		remove_newline_char(query); // remove '\n' character from string

		// if query is empty, continue the while loop
		if (isEmpty(query))
		{
			continue;
		}

		// if query is exit, exit from program
		if (strcmp("exit", query) == 0)
		{
			exit(0);
		}

		// open FIFO for write only
		fd = open(myfifo, O_WRONLY);
		// write the input on FIFO
		// and close it
		write(fd, query, strlen(query) + 1); // strlen + 1 -> write string and string termination character '\0'
		close(fd); // close pipe

		// ppen FIFO for read only
		fd = open(myfifo, O_RDONLY);

		// read message from FIFO
		read(fd, msg, sizeof(message_t));
		close(fd);

		// print the message
		printf("%s\n", msg->message);

		// if message type is error, continue the while loop
		if (msg->message_type == ERROR)
		{
			continue;
		}

		// if returned message is null, there is no returned value, so continue the loop
		if (strcmp("null", msg->message) == 0)
		{
			continue;
		}

		// the loop continue until it is entered valid input
		while (1)
		{
			printf("Sorgu sonucu kaydedilsin mi? (e/h): "); // ask that whether returned values save to file (yes/no)
			fgets(query, 100, stdin);
			remove_newline_char(query);

			// if answer is e
			if (strcmp("e", query) == 0)
			{
				save(msg->message);  // saving message to sonuc.txt
				printf("basariyla kaydedildi\n");	// successfully saved
				break;
			}
			else if (strcmp("h", query) == 0) // if answer h, break the loop and continue main loop
			{
				break;
			}
			else	// otherwise, again ask the question to user
			{
				printf("Error: Gecersiz komut! Lutfen 'e' ya da 'h' giriniz!\n"); // print error message
			}
		}
	}

	free(msg); // free heap memory

	return 0;
}

void remove_newline_char(char *str)
{
	while (*str != '\0') // str is not null
	{
		if (*str == '\n') // str is newline character
		{
			*str = '\0'; // str assigned null character
			break;
		}
		str++; // increse str pointer
	}
}

void save(char *arr)
{
	int pipefd[2];

	// if there is an error
	if (pipe(pipefd) < 0)
	{
		printf("error: pipe basarisi\n"); // print error message
		exit(1); // terminate the program
	}

	int pid = fork();
	int c;
	if (pid == 0)
	{
		// write the string to pipe
		write(pipefd[1], arr, strlen(arr) + 1);
		// execute kaydet program
		char *args[1] = {NULL};
		c = execv("kaydet", args);
		// if there is an error
		printf("error: kayit basarisiz\n");
		close(pipefd[0]);
		close(pipefd[1]);
		//exit(0);
	}
	else
	{
		wait(&c); // wait child process
		// close the pipe
		close(pipefd[0]);
		close(pipefd[1]);
	}
}

int isEmpty(char *str)
{
	// str value is not null
	while (*str != '\0')
	{
		// if str value is not empty
		if (*str != ' ')
		{
			return 0; // False: it is not empty
		}
		str++; // increse str pointer
	}
	return 1; // True: str is empty
}
