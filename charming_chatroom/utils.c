/**
 * Charming Chatroom
 * CS 241 - Spring 2020
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here
    int32_t msg_size = htonl(size);
    ssize_t written_bytes = 
        write_all_to_socket(socket, (char *)&msg_size, MESSAGE_SIZE_DIGITS);
    return written_bytes;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    ssize_t retval;
    ssize_t nread = 0;
    while ( nread < (ssize_t) count) {
        retval = read(socket, buffer + nread, count - nread);
        if (retval == 0) 
            return 0;
        else if (retval > 0) 
            nread += retval;
        else if ( retval == -1 && (errno == EINTR || errno == EAGAIN) )
            continue;
        else if (retval == -1) 
            return -1;
    }
    return nread;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    ssize_t retval;
    ssize_t nwritten = 0;
    while ( nwritten < (ssize_t) count ) {
        retval = write(socket, buffer + nwritten, count - nwritten);
        if (retval == 0) 
            return 0;
        else if (retval > 0) 
            nwritten += retval;
        else if ( retval == -1 && (errno == EINTR || errno == EAGAIN) )
            continue;
        else if (retval == -1)
            return -1;
    }
    return nwritten;
}
