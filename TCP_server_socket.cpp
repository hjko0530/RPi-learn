/*
** server.c – 展示一個stream socket server
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#define PORT "3490" // 提供給使用者連線的 port
#define BACKLOG 10  // 有多少個特定的連線佇列（pending connections queue）

void sigchld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

// 取得sockaddr，IPv4或IPv6：
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd; // 在 sock_fd 進行 listen，new_fd 是新的連線
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // 連線者的位址資訊
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char *msg = "Helo world";
    char buf[100];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // 使用我的 IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // 以迴圈找出全部的結果，並綁定（bind）到第一個能用的結果
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // 全部都用這個 structure

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // 收拾全部死掉的 processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while (1)
    { // 主要的 accept() 迴圈
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1)
        {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork())
        {                  // 這個是 child process
            close(sockfd); // child 不需要 listener

            if (send(new_fd, msg, strlen(msg), 0) == -1)
                perror("send");
            if (recv(new_fd, buf, sizeof(buf), 0) == -1)
                perror("recv");
            printf("server: received '%s'\n", buf);
            close(new_fd);

            exit(0);
        }

        close(new_fd); // parent 不需要這個
    }

    return 0;
}