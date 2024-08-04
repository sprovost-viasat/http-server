#pragma once


typedef struct QueryParam {
    char key[32];
    char value[256];
} QueryParam_t;

// to make a linked list of dynamic size based on the number of query params
typedef struct QueryParamNode {
    QueryParam_t data;
    struct QueryParamNode *next;
} QueryParamNode_t;

char * get_note(unsigned int id);
QueryParamNode_t * parse_query_string(const char *query_string);
