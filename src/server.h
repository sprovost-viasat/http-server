#pragma once
#include "queryparams.h"

typedef struct inote{
    char to[32];
    char from[32];
    char note[256];
    unsigned int id;
} inote_t; 


// method and URL strings are passed by reference
void process_request_line(char *request_line, char **method, char **URL);
char * process_GET_request(const char *URL, unsigned int *response_len);
char * process_POST_request(char *buffer, unsigned int *response_len);
