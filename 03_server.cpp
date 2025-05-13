#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>

// Print a simple message to stderr (for informational logs)
static void msg(const char *msg){
    fprintf(stderr, "%s\n", msg);
}

// Print an error message and abort the program (used for critical failures)
static void die(const char *msg){
    int err = errno;  // Save the current value of errno (error code)
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort(); // Terminate the program
}

const size_t k_max_msg = 4096; // Define the maximum message length the server will handle

// Read exactly n bytes from the file descriptor fd into buf
static int32_t read_full(int fd, char *buf, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, buf, n);
        if(rv <= 0){
            return -1; // Error or client closed the connection
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

// Write exactly n bytes from buf to the file descriptor fd
static int32_t write_all(int fd, const char *buf, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buf, n);
        if(rv <= 0){
            return -1; // Write error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

// Handle a single client request
static int32_t one_request(int connfd){
    char rbuf[4 + k_max_msg]; // Buffer for receiving data
    errno = 0;

    // First read 4 bytes which indicate the message length
    int32_t err = read_full(connfd, rbuf, 4);
    if(err){
        msg(err == 0 ? "EOF" : "read() error");
        return err;
    }

    // Extract message length
    uint32_t len = 0;
    memcpy(&len, rbuf, 4);
    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    // Read the actual message body
    err = read_full(connfd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    // Print client's message to stderr
    fprintf(stderr, "client says: %.*s\n", len, &rbuf[4]);

    // Prepare a reply using the same protocol (4-byte length + message)
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);         // 4-byte length prefix
    memcpy(&wbuf[4], reply, len);  // message body

    // Send the reply to the client
    return write_all(connfd, wbuf, 4 + len);
}

int main(){
    // Create a TCP socket using IPv4
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        die("socket()");
    }

    // Allow address reuse (useful for restarting the server quickly)
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // Set up server address to listen on all network interfaces (0.0.0.0:1111)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1111); // Convert port to network byte order
    addr.sin_addr.s_addr = ntohl(0); // 0.0.0.0

    // Bind the socket to the address and port
    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        die("bind()");
    }

    // Listen for incoming connections with max backlog
    rv = listen(fd, SOMAXCONN);
    if(rv){
        die("listen()");
    }

    msg("Server is listening on port 1111...");

    // Accept and process client connections in an infinite loop
    while(true){
        struct sockaddr_in client_addr = {};
        socklen_t addrlen = sizeof(client_addr);

        // Accept a new client connection
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
        if(connfd < 0){
            continue; // Failed to accept, try again
        }

        // Handle multiple requests from the same client
        while(true){
            int32_t err = one_request(connfd);
            if(err){
                break; // Exit loop on error or client disconnect
            }
        }

        // Close client connection
        close(connfd);
    }

    return 0;
}