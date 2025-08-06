#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
        exit(1);
    }
    char *server = argv[1];
    int port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *he;

    if ((he = gethostbyname(server)) == NULL) {
        perror("gethostbyname"); exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket"); exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    memcpy(&serv_addr.sin_addr, he->h_addr, he->h_length);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect"); close(sockfd); exit(1);
    }

    printf("Connected to warehouse server. Type commands (e.g., ADD CARBON 5), or EXIT to quit:\n");

    fd_set master;
    int fdmax = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;
    char buf[BUF_SIZE];

    int running = 1;
    while (running) {
        FD_ZERO(&master);
        FD_SET(STDIN_FILENO, &master);
        FD_SET(sockfd, &master);

        if (select(fdmax+1, &master, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        // בדיקת קלט מהשרת
        if (FD_ISSET(sockfd, &master)) {
            int n = recv(sockfd, buf, sizeof(buf)-1, 0);
            if (n == 0) {
                printf("Connection closed by server (possibly due to SHUTDOWN).\n");
                break;
            } else if (n < 0) {
                perror("recv");
                break;
            } else {
                buf[n] = '\0';
            }
        }

        // בדיקת קלט מהמשתמש
        if (FD_ISSET(STDIN_FILENO, &master)) {
            if (fgets(buf, sizeof(buf), stdin) == NULL) {
                // EOF מקלט
                printf("Input closed.\n");
                break;
            }
            send(sockfd, buf, strlen(buf), 0);
            if (strncmp(buf, "EXIT", 4) == 0) {
                printf("Closing connection by user request.\n");
                break;
            }
        }
    }

    close(sockfd);
    return 0;
}