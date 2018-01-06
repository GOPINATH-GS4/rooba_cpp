//
// Created by GOPI on 12/28/17.
//
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <csignal>

int main(int argc, char **argv) {

    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char *host;
    int port;
    char *message;

    if (argc < 4){
        std::cout << "Usage " << argv[0] << " host port message" << std::endl;
        return 1;
    }

    host = argv[1];
    port = atoi(argv[2]);
    message = argv[3];
    if ((port > 65535) || (port < 2000)) {
        std::cerr << "Please enter a port number between 2000 - 65535" << std::endl;
        return 0;
    }


    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(host);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }
    int flags = fcntl(sockfd, F_GETFL, 0);

    flags = flags | O_NONBLOCK;
    fcntl(sockfd, F_SETFL, flags);

    if (write(sockfd, message, strlen(message)) < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
    char buffer[1024];
    memset(buffer, 0x00, sizeof(buffer));
    read(sockfd, buffer, sizeof(buffer));
    close(sockfd);
    return 0;
}