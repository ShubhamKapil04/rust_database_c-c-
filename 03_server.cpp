#include<stdint.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/ip.h>

// Print a simple message to stderr
static void msg(const char *msg){
    fprintf(stderr, "%s\n", msg);
}

// Print an error message and abort the program
static void die(const char *msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

// Handle a single client connection
static void do_somthing(int connfd){
    char rbuf[64] = {};

    // Read message from the client
    ssize_t n = read(connfd,rbuf, sizeof(rbuf) - 1);
    if(n < 0){
        msg("read() error");
        return;
    }

    fprintf(stderr, "client says: %s\n", rbuf);

    char wbuf[] = "world";
    write(connfd, wbuf, strlen(wbuf));
}

int main(){

    // Create a TCP socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        die("socket()");
    }

    // Bind the socket to 0.0.0.0:1111
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    //bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1111); // Convert to nextwork byte order
    addr.sin_addr.s_addr = ntohl(0); // 0.0.0.0(accept connections or any interface)
    int rv = bind(fd,(const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        die("bind()");
    };

    // Start listening for incoming connections
    rv = listen(fd, SOMAXCONN);
    if(rv){
        die("listen()");
    };

    msg("Server is listing on port 1111 .....");

    // Accept and handle incoming connections in a loop
    while(true){

        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);

        // Accept a new connection
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);

        if(connfd < 0){
            continue; // Try the next connection
        };

        do_somthing(connfd);
        close(connfd); // close connection after handling
    }

    return 0;
};

