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

static void msg(const char *msg){
    fprintf(stderr, "%s\n", msg);
}


// Helper function to print an error message and exit
static void die(const char *msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    exit(1);
};


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

static int32_t write_all(int fd, const char *buf, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, buf, n);
        if(rv <= 0){
            return -1 ; // error
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

const size_t k_max_msg = 4096;

static int32_t query(int fd, const char *text){
    uint32_t len = (uint32_t)strlen(text);
    if(len > k_max_msg){
        return -1;
    }

    char wbuf[4+k_max_msg];
    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], text, len);
    if(int32_t err = write_all(fd, wbuf, 4+len)){
        return err;
    }

    // 4 bytes header

    char rbuf[4+k_max_msg +1];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    memcpy(&len, rbuf, 4);
    if(len > k_max_msg){
        msg("too long");
        return -1;
    }

    // reply body
    err = read_full(fd, &rbuf[4],len);

    if(err){
        msg("read() error");
        return err;
    }

    // do somthing
    printf("Server says: %.*s\n", len, &rbuf[4]);
    return 0;
}

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

    // multiple request
    int32_t err = query(fd, "hello1");
    if(err){
        goto L_DONE;
    }

    err = query(fd, "hello2");
    if(err){
        goto L_DONE;
    }
    err = query(fd, "hello3");
    if(err){
        goto L_DONE;
    }

    L_DONE :
        close(fd);
        return 0;
};