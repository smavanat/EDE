#include "../include/file_utils.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * Reads a file in its entirety
 * @param path the filepath
 * @param buf a buffer to add the contents of the fiel tp
 * @param addNull whether the buffer should be null terminated or not
 * @return the filesize if successful, -1 otherwise
 */
int read_to_end(char const *path, char **buf, bool addNull) {
    FILE *fp; //Struct to store our file data
    size_t fsz; //The file size
    long offEnd; //Byte offset to the end of the file
    int rc; //Return code of the fseek function

    //Open the file
    fp = fopen(path, "rb");
    if(NULL == fp) {
        return -1;
    }

    //Seek to the end of the file
    rc = fseek(fp, 0L, SEEK_END);
    if(0 != rc) {
        return -1;
    }

    //Byte offset to the end of the file
    if(0 > (offEnd = ftell(fp))) {
        return -1;
    }
    fsz = (size_t)offEnd;

    //Allocate a buffer to hold the whole file
    *buf = malloc(fsz + (int)addNull);
    if(NULL == *buf) {
        return -1;
    }

    //Rewind file pointer to the start of the file:
    rewind(fp);

    //Place the file into a buffer
    if(fsz != fread(*buf, 1, fsz, fp)) {
        free(buf);
        return -1;
    }

    //Close the file
    if(EOF == fclose(fp)) {
        free(*buf);
        return -1;
    }

    //Add null terminator
    if(addNull) {
        (*buf)[fsz] = '\0';
    }

    return fsz;
}
