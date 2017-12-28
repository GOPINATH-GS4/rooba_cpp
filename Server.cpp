//
// Created by GOPI on 12/28/17.
//
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <csignal>

#include "Roomba.h"

Roomba *roomba;
void processRequest(int sockfd, void (*f)(char *));
void setSigChild();
int velocity = 0;
int angle = 0;

void eventHandler(char *s) {
    char *ptr;
    std::cout << s << std::endl;

    ptr = strchr(s, ':');

    if (ptr == nullptr) return;
    *ptr = 0x00;
    ptr++;


    switch(s[0]) {
        case 'V' :
            std::cout << "Velocity " << ptr << std::endl;

            if (atoi(ptr) < -200 && atoi(ptr) > 200)
                velocity = 100;
            else
                velocity = atoi(ptr);
            break;

        case 'A' :

            std::cout << "Angle " << ptr << std::endl;
            if (atoi(ptr) < -200 && atoi(ptr) > 200)
                angle = 0;
            else
                angle = atoi(ptr);
            break;

        case 'D' :
            std::cout << "Drive Seconds " << ptr << std::endl;

            if (atoi(ptr) <= 0 && atoi(ptr) > 10) return;
            roomba->drive(velocity, angle);
            sleep(atoi(ptr));
            roomba->drive(0,0);
            break;
        case 'S' :
            std::cout << "Spin Seconds " << ptr << std::endl;

            if (atoi(ptr) <= 0 && atoi(ptr) > 10) return;
            roomba->spin(Roomba::CLOCKWISE, velocity);
            sleep(atoi(ptr));
            roomba->spin(Roomba::CLOCKWISE, 0);
            break;
    }
}

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cout << "Usage " << argv[0] << " port" << std::endl;
        return (1);
    }

    int client_fd, sockfd;
    socklen_t clilen;
    int port = atoi(argv[1]);
    struct sockaddr_in serv_addr, cli_addr;

    if ((port > 65535) || (port < 2000)) {
        std::cerr << "Please enter a port number between 2000 - 65535" << std::endl;
        return 0;
    }
   // roomba = new Roomba((char *) "/dev/ttyUSB0", B115200);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        return (1);
    }
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        return(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    setSigChild();
    while (1) {
        client_fd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (client_fd < 0) {
            perror("ERROR on accept");
            exit(1);
        }
        int pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            close(sockfd);
            processRequest(client_fd, eventHandler);
            close(client_fd);
            exit(0);
        }

    }

}

void processRequest(int sockfd, void (*f)(char *)) {

    fd_set readfds;
    struct timeval tv;
    char buffer[2048];
    int index, ret;

    FD_ZERO(&readfds);

    FD_SET(sockfd, &readfds);


    int n = sockfd + 1;

    tv.tv_sec = 5;
    tv.tv_usec = (double) .5;

    index = 0;
    ret = select(n, &readfds, NULL, NULL, &tv);

    if (ret == -1) {
        perror("select"); // error occurred in select()
    } else if (ret == 0) {
        printf("Timeout occurred!  No data after .5 seconds.\n");
        close(sockfd);
    } else {

        if (FD_ISSET(sockfd, &readfds)) {
            ssize_t no_read = 0;
            char c;
            while ((no_read = read(sockfd, &c, 1)) > 0) {
                buffer[index] = c;
                if (no_read == -1 || c == 10 || c == 13) break;
                index++;
            }
            index++;
            buffer[index] = 0x00;
            //if (index) printf("Writing... %s\n", buffer);
            f(buffer);

        }
    }
    close(sockfd);
}

void handleSigchld(int sig) {
    std::cout << "Handled signal " << sig << std::endl;
    int saved_errno = errno;
    setSigChild();
    while (waitpid((pid_t) (-1), 0, WNOHANG) > 0) {}
    errno = saved_errno;
}

void setSigChild() {

    struct sigaction sa{};

    sa.sa_handler = &handleSigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }
}