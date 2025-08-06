#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <getopt.h>
#include <sys/time.h>
#include <sys/select.h>

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
        fprintf(stderr, "Error: specify either -h <host> -p <port> (UDP) OR -f <UDS path> (UDS datagram)\n");
        exit(1);
    }

    int sockfd;
    if (uds_path) {
        // UDS Datagram
        sockfd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (sockfd < 0) { perror("socket"); exit(1); }
        struct sockaddr_un serv_addr, cli_addr;
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sun_family = AF_UNIX;
        strncpy(serv_addr.sun_path, uds_path, sizeof(serv_addr.sun_path)-1);

        // נדרש bind לכתובת ייחודית ללקוח
        char client_sock[256];
        snprintf(client_sock, sizeof(client_sock), "/tmp/molecule_req_%d.sock", getpid());
        memset(&cli_addr, 0, sizeof(cli_addr));
        cli_addr.sun_family = AF_UNIX;
        strncpy(cli_addr.sun_path, client_sock, sizeof(cli_addr.sun_path)-1);
        unlink(client_sock);
        if (bind(sockfd, (struct sockaddr*)&cli_addr, sizeof(cli_addr)) < 0) {
            perror("bind client UDS"); exit(1);
        }

        printf("Type UDP requests (e.g., DELIVER WATER 3), type EXIT to quit:\n");
        char buf[BUF_SIZE], reply[BUF_SIZE];
        while (1) {
            if (fgets(buf, sizeof(buf), stdin) == NULL)
                break;
            if (strncmp(buf, "EXIT", 4) == 0) {
                printf("Exiting molecule_requester. Goodbye!\n");
                break;
            }
            ssize_t slen = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if (slen < 0) { perror("sendto"); continue; }

            // Timeout
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            struct timeval tv = {3, 0};
            int retval = select(sockfd+1, &readfds, NULL, NULL, &tv);
            if (retval == -1) {
                perror("select");
                continue;
            } else if (retval == 0) {
                printf("No response from server (possibly due to SHUTDOWN). Exiting.\n");
                break;
            } else {
                struct sockaddr_un from_addr;
                socklen_t fromlen = sizeof(from_addr);
                ssize_t n = recvfrom(sockfd, reply, sizeof(reply)-1, 0, (struct sockaddr*)&from_addr, &fromlen);
                if (n < 0) { perror("recvfrom"); continue; }
                reply[n] = 0;
                printf("Server: %s", reply);
            }
        }
        close(sockfd);
        unlink(client_sock);
        return 0;
    } else {
        // UDP
        struct sockaddr_in serv_addr;
        struct hostent *he;
        if ((he = gethostbyname(host)) == NULL) {
            perror("gethostbyname"); exit(1);
        }
        if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            perror("socket"); exit(1);
        }
        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        memcpy(&serv_addr.sin_addr, he->h_addr, he->h_length);

        printf("Type UDP requests (e.g., DELIVER WATER 3), type EXIT to quit:\n");
        char buf[BUF_SIZE], reply[BUF_SIZE];
        while (1) {
            if (fgets(buf, sizeof(buf), stdin) == NULL)
                break;
            if (strncmp(buf, "EXIT", 4) == 0) {
                printf("Exiting molecule_requester. Goodbye!\n");
                break;
            }
            ssize_t slen = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
            if (slen < 0) { perror("sendto"); continue; }
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            struct timeval tv = {3, 0};
            int retval = select(sockfd+1, &readfds, NULL, NULL, &tv);
            if (retval == -1) {
                perror("select");
                continue;
            } else if (retval == 0) {
                printf("No response from server (possibly due to SHUTDOWN). Exiting.\n");
                break;
            } else {
                struct sockaddr_in from_addr;
                socklen_t fromlen = sizeof(from_addr);
                ssize_t n = recvfrom(sockfd, reply, sizeof(reply)-1, 0, (struct sockaddr*)&from_addr, &fromlen);
                if (n < 0) { perror("recvfrom"); continue; }
                reply[n] = 0;
                printf("Server: %s", reply);
            }
        }
        close(sockfd);
        return 0;
    }
}