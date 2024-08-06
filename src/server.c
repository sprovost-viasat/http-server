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

#define DATABASESIZE 20


unsigned int NOTES_SIZE = 3;
inote_t notes[DATABASESIZE] = {
    {"Chris", "Eric", "Hey thanks for letting me use your Pi zero!!", 0},
    {"Tommy", "Eric", "I appreciate the golf lesson today (8/3/24)!", 1},
    {"Eric", "Eric", "Lock in and finish this program by the end of day", 2},
};

void print_notes()
{
    for (int i = 0; i < NOTES_SIZE; i++)
    {
        printf("%u. to: %s, from: %s; %s\n",
            notes[i].id,
            notes[i].to,
            notes[i].from,
            notes[i].note
        );
    }
}

// method and URL strings are passed by reference
void process_request_line(char *request_line, char **method, char **URL)
{
    printf("processing request line: %s\n", request_line);
    char del[] = " ";
    *method = strtok(request_line, del);     /*Tokenize the request line on the basis of space, and extract the first word*/
    *URL = strtok(NULL, del);                /*Extract the URL*/
}

void query_database(unsigned int *ids, const unsigned int *num_ids, QueryParamNode_t **head)
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
            // printf("Adding note %u to list to share\n", notes[i].id);
            ids[ids_ind] = notes[i].id;
            ids_ind++;
        }
    }
}

// note id == UINT_MAX if failed to create
inote_t create_note(QueryParamNode_t **head)
{
    //  Save param and the value accordingly

    unsigned int id_key = UINT_MAX;
    char *to_key = NULL;
    char *from_key = NULL;
    char *note_key = NULL;

    unsigned int count = 0;
    
    // Populate datatypes from query params
    QueryParamNode_t *curr = *head;
    while (curr)
    {
        if (strcmp(curr->data.key, "note") == 0)
        {
            note_key = strdup(curr->data.value);
            count++;
        }
        else if (strcmp(curr->data.key, "to") == 0)
        {
            to_key = strdup(curr->data.value);
            count++;
        }
        else if (strcmp(curr->data.key, "from") == 0)
        {
            from_key = strdup(curr->data.value);
            count++;
        }

        if (curr->next == NULL) break;
        curr = curr->next;
    }

    if (count < 3)
    {
        printf("Error adding item to database. Missing a required value.\n");
        inote_t ret = {
            .from = "",
            .to = "",
            .note = "",
            .id = UINT_MAX
        };
        return ret;
    }

    inote_t ret = {
        .id = NOTES_SIZE
    };
    strcpy(ret.from, from_key);
    strcpy(ret.to, to_key);
    strcpy(ret.note, note_key);
    return ret;
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

    // calculate and append content length of response
    unsigned int content_length = strlen(*response);
    char content_length_str[16];
    sprintf(content_length_str, "%u\n", content_length);
    strcat(*header, "Content-Length: ");
    strcat(*header, content_length_str);

    strcat(*header, "Connection: close\n");
    // strcat(header, itoa(content_len_str)); 
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

    // return a linked list of key, value pairs parsed from query string
    QueryParamNode_t *head = parse_query_string(URL);

    unsigned int selected_ids[NOTES_SIZE];

    // Initialize all selected_ids to UINT_MAX (aka NULL)
    memset(selected_ids, UINT_MAX, sizeof(selected_ids));

    query_database(selected_ids, &NOTES_SIZE, &head);

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

    unsigned int content_len_str = construct_html_table(&response, NOTES_SIZE, selected_ids);

    /*create HTML hdr returned by server*/
    char *header  = calloc(1, 248 + content_len_str);
    content_len_str = construct_html_header(&header, &response);

    *response_len = content_len_str;
    free(response);
    
    return header;
}



char * process_POST_request(char *buffer, unsigned int *response_len)
{
    char* data = extract_buffer_body_data(&buffer, strlen(buffer));

    if (!data)
    {
        return "Failed to parse data in POST body.\n";
    }

    // return a linked list of key, value pairs parsed from query string
    QueryParamNode_t *head = parse_query_string(data);

    free(data);

    const size_t ARRSIZE = sizeof(notes) / sizeof(inote_t);
    unsigned int selected_ids[ARRSIZE];

    // Initialize all selected_ids to UINT_MAX (aka NULL)
    memset(selected_ids, UINT_MAX, sizeof(selected_ids));

    inote_t new_note = create_note(&head);

    if (new_note.id == UINT_MAX)
    {
       return "Failed to create new note.\n"; 
    }

    notes[NOTES_SIZE] = new_note;
    NOTES_SIZE++;

    // Free linked list nodes
    QueryParamNode_t *cur = head;
    while (cur)
    {
        cur = cur->next;
        free(head);
        head = cur;
    }

    char *response = calloc(1, 248);
    strcpy(response, "Added note to database with id ");
    char id_str[4];
    sprintf(id_str, "%u", new_note.id);
    strcat(response, id_str);
    strcat(response, ".\n");
    char *header  = calloc(1, 248);
    unsigned int content_len_str = construct_html_header(&header, &response);
    *response_len = strlen(header);
    free(response);
    return header;
}
