#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CAHARACTER 100 // max number of letters to be supported
#define MAX_LIST 10        // max number of commands to be supported
#define QUERY_NUM 8

// message type message or error
typedef enum
{
    MESSAGE,
    ERROR
} MESSAGE_TYPE;

// data struct store value read from file
typedef struct
{
    char name[100];
    char number[10];
} data_t;

// send message to program
typedef struct
{
    char message[MAX_CAHARACTER];
    MESSAGE_TYPE message_type;
} message_t;

// parse string to words by using delimiter (delim)
// index is start point
// strings assing to parsed
void split(char *str, char parsed[MAX_LIST][MAX_CAHARACTER], char *delim, int index);

// remove character '\n' from string
// the reason I use this function is that when user enter a input, input function put '\n'
// character at the end of the string
void remove_newline_char(char *str);

// clear string
void clear_data(char *str);

// clear string array
void clear_str_arr(char arr[MAX_LIST][MAX_CAHARACTER]);

// check that whether a string is a number
// if the string is not a number, return 0. Otherwise return 1
int isNumber(char *str);

// control whether string is empty
// if string is empty, return 1
// otherwise return 0
int isEmpty(char *str);

// execute query
// data is temporarily storage space and queried values assing to data
// value returned from query store msg
void execQuery(char query[MAX_LIST][MAX_CAHARACTER], data_t *data, message_t *msg);

// check query and if there is an error, return error codes
// -1 -> missing or bad code
// -2 -> not found file
// 0 -> no error
int queryError(char query[MAX_LIST][MAX_CAHARACTER]);

// length of string array -> return number of string
int len_str_arr(char str[MAX_LIST][MAX_CAHARACTER]);

// select data from files
// it may be selected number or name or both
// queried_data is returned data from database
// data is temporarily storage space
// if there is no selected value, return 0. Otherwise return 1
int select_data(char query[MAX_LIST][MAX_CAHARACTER], char *queried_data, data_t *data);

// check file exists
// if file exists return 1, otherwise return 0
int file_exists(char *file);

int main()
{
    char inputString[MAX_CAHARACTER] = {'\0'};          // take input from user
    char parsedArgs[MAX_LIST][MAX_CAHARACTER] = {'\0'}; // arguments of parsed string

    data_t *data = calloc(1, sizeof(data_t)); // store values returned from database 
    message_t *msg = calloc(1, sizeof(message_t)); // message for sending to program

    int fd;
    // FIFO file path
    char myfifo[] = "/tmp/myfifo";

    // creating the named file(FIFO)
    // mkfifo(<pathname>,<permission>)
    mkfifo(myfifo, 0666);

    while (1)
    {
        fd = open(myfifo, O_RDONLY);
        read(fd, inputString, MAX_CAHARACTER); // read data from program
        close(fd);

        printf("Gelen komut: %s\n", inputString); // print data

        split(inputString, parsedArgs, " ", 0); // split data, start index 0
        int len = len_str_arr(parsedArgs); // lentght of words
        split(parsedArgs[len - 1], parsedArgs, "=", len - 1);   // the las element is split by '=' and it assing to the last elemtent of parsed array

        // execute commands and returned values assing to msg
        execQuery(parsedArgs, data, msg);

        fd = open(myfifo, O_WRONLY);
        write(fd, msg, sizeof(message_t));  // write message to program
        close(fd);

        // clear strings
        clear_data(msg->message);
        clear_str_arr(parsedArgs);
    }

    // free heap memory
    free(data);
    free(msg);

    return 0;
}

void split(char *str, char parsed[MAX_LIST][MAX_CAHARACTER], char *delim, int index)
{
    // if sepereted string is not NULL, parsed[i] assigned
    char *p;
    while ((p = strsep(&str, delim)) != NULL)
    {
        if (isEmpty(p)) // if parsed string is empty
        {
            continue;
        }

        strcpy(parsed[index], p);

        index++;
    }

    free(p);
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

void clear_data(char *str)
{
    while (*str != '\0')
    {
        *str = '\0';
        str++;
    }
}

void clear_str_arr(char arr[MAX_LIST][MAX_CAHARACTER])
{
    int i = 0;
    while (strcmp(arr[i], "\0") != 0)
    {
        clear_data(arr[i]);
        i++;
    }
}

int isNumber(char *str)
{
    int i = 0;
    while (*str != '\0')
    {
        if (i == 0)
        {
            if (str[0] == '-')
            {
                str++;
            }
            i++;
        }
        if (*str < '0' || *str > '9')
        {
            return 0; // false
        }
        str++;
    }

    return 1; // true
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

void execQuery(char query[MAX_LIST][MAX_CAHARACTER], data_t *data, message_t *msg)
{
    FILE *file;
    char input[100] = {'\0'};
    int i = 0;
    int request = 0;

    int error = queryError(query); // check error

    if (error == -1)    // if error -1
    {
        msg->message_type = ERROR;  // message type is an error
        strcpy(msg->message, "Error: eksik ya da hatali komut girdiniz!");
        printf("%s\n", msg->message);
        return;
    }
    else if (error == -2)   // if error -2, file error
    {
        msg->message_type = ERROR;
        strcpy(msg->message, "Error: Dosya bulunamadi!");
        printf("%s\n", msg->message);
        return;
    }
    else    // otherwise no error, it is just message
    {
        msg->message_type = MESSAGE;
    }

    if ((file = fopen(query[3], "r")) == NULL)
    {
        printf("File could not be opened\n");
    }
    else
    {
        while (!feof(file))
        {
            fgets(input, 100, file);    // read one line from file

            sscanf(input, "%s %s\n", data->name, data->number); // format the string and assing data->name and data->number

            // select data from data base
            // if there is selected value, increase request
            if (select_data(query, msg->message, data))
            {
                request++;
            }
        }

        fclose(file);
    }

    // if there is no selected value, it is empty, so null
    if (request == 0)
    {
        printf("null\n");
        strcat(msg->message, "null");
    }
}

int len_str_arr(char str[MAX_LIST][MAX_CAHARACTER])
{
    int i = 0;
    while (strcmp(str[i], "\0") != 0)
    {
        i++;
    }
    return i;
}

int select_data(char query[MAX_LIST][MAX_CAHARACTER], char *queried_data, data_t *data)
{
    int founded = 0;
    // if a value is founded
    if ((strcmp(query[5], "name") == 0 && strcmp(query[6], data->name) == 0) || (strcmp(query[5], "number") == 0 && strcmp(query[6], data->number) == 0))
    {
        founded = 1;    // value is founded
        if (strcmp(query[1], "*") == 0) // * -> name and number
        {
            printf("%s %s\n", data->name, data->number);
            // copy founded value to queried_data
            strcat(queried_data, data->name);
            strcat(queried_data, " ");
            strcat(queried_data, data->number);
            strcat(queried_data, "\n");
        }
        else if (strcmp(query[1], "name") == 0)   // name
        {
            printf("%s\n", data->name);
            strcat(queried_data, data->name);
            strcat(queried_data, "\n");
        }   
        else if (strcmp(query[1], "number") == 0)   // number
        {
            printf("%s\n", data->number);
            strcat(queried_data, data->number);
            strcat(queried_data, "\n");
        }
    }

    return founded;
}

int queryError(char query[MAX_LIST][MAX_CAHARACTER])
{
    if (len_str_arr(query) != 7)  // if number of strings is not 7
    {
        return -1;
    }
    if (strcmp("select", query[0]) != 0)
    {
        return -1;
    }

    if (strcmp("*", query[1]) != 0 && strcmp("name", query[1]) != 0 && strcmp("number", query[1]) != 0)
    {
        return -1;
    }

    if (strcmp("from", query[2]) != 0)
    {
        return -1;
    }

    if (!file_exists(query[3])) // file does not exist
    {
        return -2;
    }

    if (strcmp("where", query[4]) != 0)
    {
        return -1;
    }

    if (strcmp("name", query[5]) != 0 && strcmp("number", query[5]) != 0)
    {
        return -1;
    }

    if (isEmpty(query[6]))  // it is empty
    {
        return -1;
    }

    return 0;
}

int file_exists(char *file)
{
    FILE *f;
    f = fopen(file, "r");
    if (f == NULL)
    {
        return 0;   // file not founded
    }
    else
    {
        fclose(f);
        return 1;   // file founded
    }
}
