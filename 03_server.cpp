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

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char *buf, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, buf, n);
        if(rv <= 0){
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t write_all (int fd, const char *buf, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buf, n);
        if(rv <= 0){
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }

    return 0;
}


static int32_t one_request(int connfd){
    char rbuf[4+k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, rbuf, 4);
    errno = 0;
    if(err ){
        msg(err == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4);
    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    err = read_full(connfd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    fprintf(stderr, "client says: %.*s\n", len, &rbuf[4]);

    // reply using the same protocol
    const char reply[]= "world";
    char wbuf[4+sizeof(reply)];
    len =(uint32_t)strlen(reply);
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);

    return write_all(connfd, wbuf, 4+ len);
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

        while(true){
            int32_t err = one_request(connfd);
            if(err){
                break;
            }
        }
        close(connfd); // close connection after handling
    }

    return 0;
};

