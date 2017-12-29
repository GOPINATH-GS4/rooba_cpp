//
// Created by GOPI on 12/28/17.
//
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <csignal>
#include <math.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "Roomba.h"

Roomba *roomba;
void processRequest(int sockfd, void (*f)(char *));
void setSigChild();


void eventHandler(char *s) {

    int velocity = -1;
    int angle = -1;
    int distance = -1;
    useconds_t seconds = -1;
    bool isDistance;
    std::string command;
    std::string message(s);
    std::string delimiter = ":";

    size_t pos = 0;
    std::string token;
    int i = 0;
    while ((pos = message.find(delimiter)) != std::string::npos) {
        token = message.substr(0, pos);
        switch(i) {
            case 0:
                command = token;
                break;
            case 1:
                velocity = atoi(token.c_str());
                if (velocity < -500 || velocity > 500)
                    velocity = 0;
                std::cout << "Velocity " << velocity << std::endl;
                break;
            case 2:
                angle =  atoi(token.c_str());
                if (angle < -2000 || angle > 2000)
                    angle = 0;
                std::cout << "Angle " << angle << std::endl;
                break;
            case 3:
                isDistance = token == "D" ? true : false;
                std::cout << "isDistance " << isDistance << std::endl;
                break;
            case 4:
                if (isDistance)
                    distance = atoi(token.c_str());
                else
                    seconds = atoi(token.c_str());

                if (isDistance) {
                    seconds = (useconds_t) ((distance * 10) / abs(velocity) * pow(10,6));
                }
                std::cout << "distance  " << distance << std::endl;
                std::cout << "seconds  " << seconds << std::endl;
                break;
            default:
                break;
        }
        i++;
        std::cout << token << std::endl;
        message.erase(0, pos + delimiter.length());
    }

    if (command.compare("DRIVE") == 0) {
        roomba->drive(velocity, angle);
        usleep(seconds);
        roomba->drive(0,0);
    }
    else if (command.compare("SPIN") == 0) {
        roomba->spin(Roomba::COUNTER_CLOCKWISE, velocity);
        usleep(seconds);
        roomba->spin(Roomba::COUNTER_CLOCKWISE, 0);
    }

    std::cout << "Command complete" << std::endl;
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
    roomba = new Roomba((char *) "/dev/ttyUSB0", B115200);
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