#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "queryparams.h"

// remove all chars up to and includeing the ?
// if cannot find ? then remove nothing
char* only_query_params(char *query_string)
{
    char *question_mark = strchr(query_string, '?');

    if (question_mark) {
        // Move the beginning of the string to the character after '?'
        memmove(query_string, question_mark + 1, strlen(question_mark + 1) + 1);
        printf("Removed all chars up to and including '?':  %s\n", query_string);
    }
    
    return query_string;
}

QueryParamNode_t * parse_query_string(const char *query_string)
{
    QueryParamNode_t *head = NULL, *tail = NULL;

    char *key, *value;
    char *cquery_string = strdup(query_string);
    cquery_string = only_query_params(cquery_string);

    printf("Parsing: %s\n", cquery_string);

    int pc=0;
    char *tok;
    char *otok;
    for (tok=strtok(cquery_string,"&"); tok!=NULL; tok=strtok(tok,"&"))
    {
        pc++;
        otok=tok+strlen(tok)+1;
        key=strtok(tok,"=");
        // fprintf(stderr,"param%d: %s ",pc,key);
        value=strtok(NULL,"=");
        // fprintf(stderr,"value%d: %s\n",pc,value);
        tok=otok;

        if (!value || !key) {
            printf("Error with a ghost param! {%s:%s}\n", key, value);
            continue;
        }

        QueryParamNode_t *node = (QueryParamNode_t *)malloc(sizeof(QueryParamNode_t));
        if (node == NULL)
        {
            // Handle memory allocation failure
            printf("Failed to allocate memory for QueryParamNode_t node.\n");
            return head;
        }

        // save query param key, value to the new node
        strncpy(node->data.key, key, sizeof(node->data.key) - 1);
        node->data.key[sizeof(node->data.key) - 1] = '\0';
        strncpy(node->data.value, value, sizeof(node->data.value) - 1);
        node->data.value[sizeof(node->data.value) - 1] = '\0';
        node->next = NULL;

        printf("node key: %s, node val: %s\n", node->data.key, node->data.value);

        // add new node to the linked list
        if (head == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    };
    return head;
}

char * extract_data(char *buffer, size_t buffer_len, unsigned int content_length)
{
    // Ensure content_length is within the buffer bounds
    if (content_length > buffer_len) {
        // Handle error: content length is larger than buffer size
        return NULL;
    }

    // Calculate the starting index of the data
    char *data_start = buffer + (buffer_len - content_length);

    // Create a new string to hold the extracted data
    char *data = (char *)malloc(content_length + 1); // Allocate memory for the data and null terminator
    if (data == NULL) {
        // Handle memory allocation failure
        return NULL;
    }

    // Copy the data to the new string
    memcpy(data, data_start, content_length);
    data[content_length] = '\0'; // Null-terminate the string

    return data;
}

char * extract_buffer_body_data(char **buffer, size_t buffer_len)
{
    // unsigned int buffer_len = strlen(*buffer);
    printf("BUFFER IN extract_buffer_body_data: %s\n",*buffer);
    
    char *content_length_str = strstr(*buffer, "Content-Length: ");
    if (content_length_str == NULL) {
        // Handle error: Content-Length header not found
        printf("Parsing error: Content-Length header not found\n");
        return NULL;
    }

    content_length_str += 16; // Skip "Content-Length: "
    char *end = strchr(content_length_str, '\n');
    if (end == NULL) {
        // Handle error: Invalid Content-Length header
        printf("Parsing error: Cannot find EOL for Content-Length header\n");
        return NULL;
    }
    printf("Content-Length EOL found!\n");
    *end = '\0'; // add null-terminating to parse out int of content length

    unsigned int content_length = atoi(content_length_str);

    if (content_length == 0) return NULL;

    printf("content_length=%u\n", content_length);

    return extract_data(*buffer, buffer_len, content_length);
}

