#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "buffer.h"
#include "requests.h"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

char loginCookie[500];
char jwtToken[500];

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
        
        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;
            
            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);
            
            if (content_length_start < 0) {
                continue;           
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;
    
    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

void GetLoginCookie(char* response)
{
    char* cookie = strstr(response, "Set-Cookie");
    long poz1 = strcspn(cookie, " ");
    long poz2 = strcspn(cookie, "\n");
    strncpy(loginCookie, cookie + poz1 + 1, poz2 - poz1);
    loginCookie[poz2 - poz1] = '\0';
}

void GetJwtToken(char* response)
{
    strcpy(jwtToken, response + 10);
    long len = strlen(jwtToken);
    jwtToken[len -2] = '\0';
}

void Registration(int sockfd)
{
    char username[100]; 
    char password[100];
    char status[100];
    char *message, *response;
    char** payload = (char**)malloc(sizeof(char*));
    payload[0] = (char*)malloc(sizeof(char) * LINELEN);

    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    sprintf(payload[0], "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

    message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/register", "application/json",\
     payload, 1, NULL, 0, 1);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
        printf("%s\n", response);
}

void Login(int sockfd)
{
    char username[100]; 
    char password[100];
    char status[100];
    char *message, *response, *json;
    char** payload = (char**)malloc(sizeof(char*));
    payload[0] = (char*)malloc(sizeof(char) * LINELEN);

    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    sprintf(payload[0], "{\"username\":\"%s\",\"password\":\"%s\"}", username, password);

     message = compute_post_request("34.241.4.235", "/api/v1/tema/auth/login", "application/json",\
     payload, 1, NULL, 0, 1);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    json = basic_extract_json_response(response);
    if(json != NULL)
        printf("%s\n", json);
    else
        GetLoginCookie(response);
}

void EnterLibrary(int sockfd)
{
    char status[100];
    char *message, *response;
    char** cookies = (char**)malloc(sizeof(char*));
    cookies[0] = (char*)malloc(sizeof(char) * LINELEN);

    strcpy(cookies[0], loginCookie);

    message = compute_get_request("34.241.4.235", "/api/v1/tema/library/access", NULL, cookies, 1, 0);
    send_to_server(sockfd, message);
    
    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
    {
        if(strncmp(response, "{\"error\"", 8) == 0)
            printf("%s\n", response);
        else
            GetJwtToken(response);
    }
}

void GetBooks(int sockfd)
{
    char status[100];
    char line [LINELEN];
    char *message, *response;

    message = compute_get_request("34.241.4.235", "/api/v1/tema/library/books", NULL, NULL, 0, 0);
    message[strlen(message) - 2] = '\0';
    sprintf(line, "Authorization: Bearer %s", jwtToken);
    compute_message(message, line);
    compute_message(message, "");
    send_to_server(sockfd, message);
    
    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
        printf("%s\n", response);
}

void GetBook(int sockfd)
{
    char status[100];
    char line[LINELEN];
    char id[100];
    char URL[200];
    char *message, *response;

    printf("id=");
    scanf("%s", id);

    sprintf(URL, "/api/v1/tema/library/books/%s", id);

    message = compute_get_request("34.241.4.235", URL, NULL, NULL, 0, 0);
    message[strlen(message) - 2] = '\0';
    sprintf(line, "Authorization: Bearer %s", jwtToken);
    compute_message(message, line);
    compute_message(message, "");
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
        printf("%s\n", response);
}

void AddBook(int sockfd)
{
    char status[100];
    char line[LINELEN];
    char title[100], author[100], genre[100], publisher[100], page_count[100];
    char *message, *response;
    char** payload = (char**)malloc(sizeof(char*));
    payload[0] = (char*)malloc(sizeof(char) * LINELEN);

    printf("title=");
    scanf("%s", title);
    printf("author=");
    scanf("%s", author);
    printf("genre=");
    scanf("%s", genre);
    printf("publisher=");
    scanf("%s", publisher);
    printf("page_count=");
    scanf("%s", page_count);

    sprintf(payload[0], "{\"title\":\"%s\",\"author\":\"%s\",\"genre\":\"%s\",\"publisher\":\"%s\",\"page_count\":\"%s\"}",\
     title, author, genre, publisher, page_count);
    
    message = compute_post_request("34.241.4.235", "/api/v1/tema/library/books", "application/json",\
     payload, 1, NULL, 0, 0);
    message[strlen(message) - 2] = '\0';
    sprintf(line, "Authorization: Bearer %s", jwtToken);
    compute_message(message, line);
    compute_message(message, "");
    strcat(message, payload[0]);
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
        printf("%s\n", response);
}

void DeleteBook(int sockfd)
{
    char status[100];
    char line[LINELEN];
    char id[100];
    char URL[200];
    char *message, *response;

    printf("id=");
    scanf("%s", id);

    sprintf(URL, "/api/v1/tema/library/books/%s", id);

    message = compute_get_request("34.241.4.235", URL, NULL, NULL, 0, 1);
    message[strlen(message) - 2] = '\0';
    sprintf(line, "Authorization: Bearer %s", jwtToken);
    compute_message(message, line);
    compute_message(message, "");
    send_to_server(sockfd, message);

    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
        printf("%s\n", response);
}

void Logout(int sockfd)
{
    char status[100];
    char *message, *response;
    char** cookies = (char**)malloc(sizeof(char*));
    cookies[0] = (char*)malloc(sizeof(char) * LINELEN);

    strcpy(cookies[0], loginCookie);

    message = compute_get_request("34.241.4.235", "/api/v1/tema/auth/logout", NULL, cookies, 1, 0);
    send_to_server(sockfd, message);

    memset(loginCookie, 0, 500);
    memset(jwtToken, 0, 500);
    
    response = receive_from_server(sockfd);
    long len = strcspn(response, "\n");
    strncpy(status, response, len+1);
    status[len + 1] = '\0';
    printf("%s\n", status);

    response = basic_extract_json_response(response);
    if(response != NULL)
        printf("%s\n", response);
}

