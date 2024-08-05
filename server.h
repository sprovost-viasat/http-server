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
void string_space_trim(char *string);
inote_t create_note(QueryParamNode_t **head);
void query_database(unsigned int *ids, const unsigned int *num_ids, QueryParamNode_t **head);
void print_notes();
unsigned int construct_html_header(char** header, char** response);
unsigned int construct_html_table(char** response, const size_t ARRSIZE, unsigned int *selected_ids);
char * process_GET_request(char *URL, unsigned int *response_len);
char * process_POST_request(char *buffer, unsigned int *response_len);
