#include<stdint.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<stdio.h>
#include<errno.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/ip.h>

// Helper function to print an error message and exit
static void die(const char *msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    exit(1);
};


int main (){

    // Create a TCP socket(IPv4)
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        die("socket()");
    }; 

    // Set up the server address (localhost:1111)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1111); // Convert to network byte order
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1
    
    // Connect to ther server
    int rv = connect(fd,(const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        die("connect");
    };

    // Send a message to the server
    char msg[] = "hello";
    write(fd, msg, strlen(msg));

    // Read the server's response
    char rbuf[64] = {};
    ssize_t n = read(fd,rbuf, sizeof(rbuf)-1);
    if(n < 0){
        die("read");
    };

    // Print the server response
    printf("server says: %s\n", rbuf);
    
    // Close the socket
    close(fd);
    return 0;
};