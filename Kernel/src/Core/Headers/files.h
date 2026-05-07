#ifndef FILES_H
#define FILES_H

typedef struct _iobuf {
    char* _ptr;
    int   _cnt;
    char* _base;
    int   _flag;
    int   _file;
    int   _charbuf;
    int   _bufsiz;
    char* _tmpfname;
} FILE;

#endif /* FILES_H */