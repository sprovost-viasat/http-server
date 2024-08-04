#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include "server.h"
#include "interface.h"
#include "queryparams.h"


typedef struct inote{

    char to[32];
    char from[32];
    char note[256];
    unsigned int id;
} inote_t; 

inote_t notes[3] = {
    {"Chris", "Eric", "Hey thanks for letting me use your Pi zero!!", 0},
    {"Tommy", "Eric", "I appreciate the golf lesson today (8/3/24)!", 1},
    {"Eric", "Eric", "Lock in and finish this program by the end of day", 2},
};

// method and URL strings are passed by reference
void process_request_line(char *request_line, char **method, char **URL)
{
    printf("processing request line: %s\n", request_line);
    char del[1] = " ";
    *method = strtok(request_line, del);     /*Tokenize the request line on the basis of space, and extract the first word*/
    *URL = strtok(NULL, del);                /*Extract the URL*/
    // printf("Method = %s\n", *method);
    // printf("URL = %s\n", *URL);
}

void query_database(unsigned int *ids, const size_t *num_ids, QueryParamNode_t **head)
{
    //  Save param and the value accordingly

    unsigned int id_key = UINT_MAX;
    char *to_key = NULL;
    char *from_key = NULL;
    
    // Populate datatypes from query params
    QueryParamNode_t *curr = *head;
    while (curr)
    {
        if (strcmp(curr->data.key, "id") == 0)
        {
            id_key = atoi(curr->data.value);
        }
        else if (strcmp(curr->data.key, "to") == 0)
        {
            to_key = strdup(curr->data.value);
        }
        else if (strcmp(curr->data.key, "from") == 0)
        {
            from_key = strdup(curr->data.value);
        }

        if (curr->next == NULL) break;
        curr = curr->next;
    }

    unsigned int ids_ind = 0;

    for (unsigned int i = 0; i < *num_ids; i++)
    {
        bool use = true;
        if (to_key)
        {
            use &= (strcmp(notes[i].to, to_key) == 0);
        }
        if (from_key)
        {
            use &= (strcmp(notes[i].from, from_key) == 0);
        }
        if (id_key != UINT_MAX)
        {
            use &= (notes[i].id == id_key);
        }
        
        if (use)
        {
            printf("Adding note %u to list to share\n", notes[i].id);
            ids[ids_ind] = notes[i].id;
            ids_ind++;
        }
    }
}

// Construct the thext response body
// returns the length of the response data
unsigned int construct_html_table(char** response, const size_t ARRSIZE, unsigned int *selected_ids)
{
    // Construct table
    strcpy(*response,
        "<html>"
        "<head>"
            "<title>HTML Response</title>"
            "<style>"
            "table, th, td {"
                "border: 1px solid black;}"
             "</style>"
        "</head>"
        "<body>"
        "<table>");

    strcat(*response, "<tr><td>To</td><td>From</td><td>Note</td></tr>");

    for (unsigned int i = 0; i < ARRSIZE; i++)
    {
        if (selected_ids[i] == UINT_MAX) break;
        printf("obtained id: %u\n", selected_ids[i]);

        strcat(*response, "<tr>");

        strcat(*response, "<td>");
        strcat(*response, notes[selected_ids[i]].to);
        strcat(*response, "</td><td>");
        strcat(*response, notes[selected_ids[i]].from);
        strcat(*response, "</td><td>");
        strcat(*response, notes[selected_ids[i]].note);
        strcat(*response, "</td>");

        strcat(*response, "</tr>");
    }

    strcat(*response , 
            "</table>"
            "</body>"
            "</html>");

    return strlen(*response);
}

// Construct the text header + concatenate the response body
// returns the length of the header and response data
unsigned int construct_html_header(char** header, char** response)
{
    strcpy(*header, "HTTP/1.1 200 OK\n");      
    strcat(*header, "Server: Roommate Notes HTTP Server\n");
    strcat(*header, "Content-Length: "  ); 
    strcat(*header, "Connection: close\n"   );
    //strcat(header, itoa(content_len_str)); 
    strcat(*header, "2048");
    strcat(*header, "\n");
    strcat(*header, "Content-Type: text/html; charset=UTF-8\n");
    strcat(*header, "\n");

    strcat(*header, *response);

    return strlen(*header);
}

char * process_GET_request(char *URL, unsigned int *response_len)
{    
    char *strid = NULL, *to = NULL, *from = NULL;

    // return a ;inked list of key, value pairs parsed from query string
    QueryParamNode_t *head = parse_query_string(URL);

    const size_t ARRSIZE = sizeof(notes) / sizeof(inote_t);
    unsigned int selected_ids[ARRSIZE];

    // Initialize all selected_ids to UINT_MAX (aka NULL)
    memset(selected_ids, UINT_MAX, sizeof(selected_ids));

    query_database(selected_ids, &ARRSIZE, &head);

    // Free linked list nodes
    QueryParamNode_t *cur = head;
    while (cur)
    {
        cur = cur->next;
        free(head);
        head = cur;
    }
    
    /*We have got the notes of interest here*/
    char *response = calloc(1, 1024);

    unsigned int content_len_str = construct_html_table(&response, ARRSIZE, selected_ids);

    /*create HTML hdr returned by server*/
    char *header  = calloc(1, 248 + content_len_str);
    content_len_str = construct_html_header(&header, &response);

    *response_len = content_len_str;
    free(response);
    
    return header;
}

char * process_POST_request(char *URL, unsigned int *response_len)
{
    return NULL;
}
