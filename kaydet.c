#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// write data to file (open append mode)
void writeFile(char *str);

int main(int argc, char *argv[])
{
    char str[100] = {'\0'}; // string

    read(3, str, 100); // read pipe
    writeFile(str);    // write string to file

    return 0;
}

void writeFile(char *str)
{
    FILE *file;
    file = fopen("sonuc.txt", "a");
    if (file == NULL)
    {
        printf("sonuc.c file failed to open.\n");
    }
    else
    {
        fputs(str, file); // writing in the file
        fclose(file); // closing the file
    }
}
