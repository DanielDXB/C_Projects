#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    int sockfd;
    char command[LINELEN];

    while(1)
    {
        scanf("%s", command);

        if(!strcmp(command, "exit"))
            break;

        sockfd = open_connection("34.241.4.235", 8080, AF_INET, SOCK_STREAM, 0);

        if(!strcmp(command, "register"))
            Registration(sockfd);
           
        if(!strcmp(command, "login"))
            Login(sockfd);
         
        if(!strcmp(command, "enter_library"))
            EnterLibrary(sockfd);
           
        if(!strcmp(command, "get_books"))
            GetBooks(sockfd);
           
        if(!strcmp(command, "get_book"))
            GetBook(sockfd);
           
        if(!strcmp(command, "add_book"))
            AddBook(sockfd);
            
        if(!strcmp(command, "delete_book"))
            DeleteBook(sockfd);
            
        if(!strcmp(command, "logout"))
            Logout(sockfd);
            
        close(sockfd);
    }

    return 0;
}
