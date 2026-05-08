#ifndef FILES_H
#define FILES_H

typedef struct FILE {
    int fd;                 // File descriptor from the OS

    unsigned char *buffer;  // Pointer to buffer memory
    size_t buffer_size;     // Size of the buffer
    size_t buffer_pos;      // Current position inside buffer
    size_t buffer_len;      // Amount of valid data in buffer

    long position;          // Current file position

    int flags;              // Read/write/error/eof flags

    int eof;                // End-of-file reached?
    int error;              // Error happened?

    int mode;               // Open mode (read/write/append)

} FILE;

#define O_RDONLY 1
#define O_WRONLY 2
#define O_RDWR   3

#endif /* FILES_H */