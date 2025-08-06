#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
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
        // בדיקה אם המשתמש הקליד EXIT (מתעלם מרווחים אחריה)
        if (strncmp(buf, "EXIT", 4) == 0) {
            printf("Exiting molecule_requester. Goodbye!\n");
            break;
        }
        ssize_t slen = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        if (slen < 0) { perror("sendto"); continue; }

        // המתנה לתשובה מהשרת עם timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        struct timeval tv;
        tv.tv_sec = 3; // 3 שניות timeout
        tv.tv_usec = 0;
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