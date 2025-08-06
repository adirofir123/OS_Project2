#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <getopt.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    char *host = NULL, *uds_path = NULL;
    int port = -1;
    int opt;
    while ((opt = getopt(argc, argv, "h:p:f:")) != -1) {
        switch(opt) {
            case 'h':
                host = strdup(optarg); break;
            case 'p':
                port = atoi(optarg); break;
            case 'f':
                uds_path = strdup(optarg); break;
        }
    }
    if (((host || port > 0) && uds_path) || (!uds_path && (host == NULL || port <= 0))) {
        fprintf(stderr, "Error: specify either -h <host> -p <port> (TCP) OR -f <UDS path> (UDS stream)\n");
        exit(1);
    }

    int sockfd;
    if (uds_path) {
        // UDS STREAM
        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
        struct sockaddr_un serv_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, uds_path, sizeof(serv_addr.sun_path)-1);
        if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            perror("connect UDS STREAM"); exit(1);
        }
        printf("Connected via UDS stream. Type atom commands (e.g., ADD CARBON 5), type EXIT to quit:\n");
    } else {
        // TCP
        struct sockaddr_in serv_addr;
        struct hostent *he;
        if ((he = gethostbyname(host)) == NULL) {
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
            perror("connect"); exit(1);
        }
        printf("Connected via TCP. Type atom commands (e.g., ADD CARBON 5), type EXIT to quit:\n");
    }

    char buf[BUF_SIZE];
    while (fgets(buf, sizeof(buf), stdin)) {
        write(sockfd, buf, strlen(buf));
        if (strncmp(buf, "EXIT", 4) == 0) break;
    }
    close(sockfd);
    printf("Exiting supplier_atom. Goodbye!\n");
    return 0;
}