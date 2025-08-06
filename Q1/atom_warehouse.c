#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>

#define MAX_ATOMS 1000000000000000000ULL
#define BUF_SIZE 1024

unsigned long long carbon = 0, oxygen = 0, hydrogen = 0;

void print_state() {
    printf("Warehouse State: CARBON=%llu OXYGEN=%llu HYDROGEN=%llu\n", carbon, oxygen, hydrogen);
    fflush(stdout);
}

int handle_command(const char *buf) {
    unsigned long long num;
    if (sscanf(buf, "ADD CARBON %llu", &num) == 1) {
        if (carbon + num > MAX_ATOMS) num = MAX_ATOMS - carbon;
        carbon += num;
        print_state();
        return 0;
    } else if (sscanf(buf, "ADD OXYGEN %llu", &num) == 1) {
        if (oxygen + num > MAX_ATOMS) num = MAX_ATOMS - oxygen;
        oxygen += num;
        print_state();
        return 0;
    } else if (sscanf(buf, "ADD HYDROGEN %llu", &num) == 1) {
        if (hydrogen + num > MAX_ATOMS) num = MAX_ATOMS - hydrogen;
        hydrogen += num;
        print_state();
        return 0;
    } else if (strncmp(buf, "EXIT", 4) == 0) {
        printf("Client requested to close its connection.\n");
        fflush(stdout);
        return 1;
    } else {
        fprintf(stderr, "Invalid command: %s", buf);
        fflush(stderr);
        return -1;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[1]);
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(listen_fd); exit(1);
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen"); close(listen_fd); exit(1);
    }

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    fd_set master, read_fds;
    int fdmax = (listen_fd > STDIN_FILENO) ? listen_fd : STDIN_FILENO;
    FD_ZERO(&master);
    FD_SET(listen_fd, &master);
    FD_SET(STDIN_FILENO, &master);

    printf("Warehouse server listening on port %d\n", port);
    printf("To close all connections type SHUTDOWN.\n");

    int running = 1;
    while (running) {
        read_fds = master;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }
        for (int i = 0; i <= fdmax; ++i) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listen_fd) {
                    
                    struct sockaddr_in client_addr;
                    socklen_t addrlen = sizeof(client_addr);
                    int newfd = accept(listen_fd, (struct sockaddr*)&client_addr, &addrlen);
                    if (newfd == -1) { perror("accept"); continue; }
                    FD_SET(newfd, &master);
                    if (newfd > fdmax) fdmax = newfd;
                } else if (i == STDIN_FILENO) {
                    
                    char input_buf[BUF_SIZE];
                    if (fgets(input_buf, sizeof(input_buf), stdin)) {
                        if (strncmp(input_buf, "SHUTDOWN", 8) == 0) {
                            printf("Server shutting down by terminal command.\n");
                            
                            for (int j = 0; j <= fdmax; ++j) {
                                if (FD_ISSET(j, &master) && j != listen_fd && j != STDIN_FILENO) {
                                    close(j);
                                    FD_CLR(j, &master);
                                }
                            }
                            close(listen_fd);
                            running = 0;
                            break;
                        }
                    }
                } else {
                    
                    char buf[BUF_SIZE];
                    ssize_t nbytes = recv(i, buf, sizeof(buf)-1, 0);
                    if (nbytes <= 0) {
                        close(i);
                        FD_CLR(i, &master);
                    } else {
                        buf[nbytes] = '\0';
                        int res = handle_command(buf);
                        if (res == 1) { 
                            close(i);
                            FD_CLR(i, &master);
                        }
                    }
                }
            }
        }
    }
    printf("Server exited.\n");
    return 0;
}