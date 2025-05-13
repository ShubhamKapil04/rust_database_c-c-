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

// Prints a simple message to stderr
static void msg(const char *msg){
    fprintf(stderr, "%s\n", msg);
}

// Prints an error message and exits the program
static void die(const char *msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    exit(1);
};

// Reads exactly 'n' bytes from a file descriptor into a buffer
// Useful for reading complete messages
static int32_t read_full(int fd, char *buf, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, buf, n);
        if(rv <= 0){
            return -1; // error or unexpected EOF
        }

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

// Writes exactly 'n' bytes from a buffer to a file descriptor
static int32_t write_all(int fd, const char *buf, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buf, n);
        if(rv <= 0){
            return -1; // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

const size_t k_max_msg = 4096; // Maximum size of a message

// Sends a message to the server and reads the reply
static int32_t query(int fd, const char *text){
    uint32_t len = (uint32_t)strlen(text); // Get length of input message
    if(len > k_max_msg){
        return -1;
    }

    // Prepare a buffer with 4-byte length prefix followed by the message
    char wbuf[4 + k_max_msg];
    memcpy(wbuf, &len, 4);          // Copy length into first 4 bytes
    memcpy(&wbuf[4], text, len);   // Copy actual message after the length

    // Send message to server
    if(int32_t err = write_all(fd, wbuf, 4 + len)){
        return err;
    }

    // Prepare a buffer to receive the reply
    char rbuf[4 + k_max_msg + 1];  // +1 for safety
    errno = 0;

    // Read the 4-byte length prefix of the reply
    int32_t err = read_full(fd, rbuf, 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    // Get length of the incoming message
    memcpy(&len, rbuf, 4);
    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    // Read the actual message from the server
    err = read_full(fd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    // Print the reply from the server
    printf("Server says: %.*s\n", len, &rbuf[4]);
    return 0;
}

int main (){
    // Create a TCP socket using IPv4
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        die("socket()");
    } 

    // Set up the server address (127.0.0.1:1111)
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1111);             // Convert port to network byte order
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // Use loopback address (127.0.0.1)

    // Connect to the server
    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if(rv){
        die("connect");
    }

    // Send multiple queries to the server
    int32_t err = query(fd, "hello1");
    if(err){
        goto L_DONE; // Exit if there's an error
    }

    err = query(fd, "hello2");
    if(err){
        goto L_DONE;
    }

    err = query(fd, "hello3");
    if(err){
        goto L_DONE;
    }

L_DONE:
    // Close the socket and exit
    close(fd);
    return 0;
};