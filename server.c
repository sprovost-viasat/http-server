#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "server.h"
#include "interface.h"

#include <limits.h>

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

/*string helping functions*/

/*Remove the space from both sides of the string*/
void string_space_trim(char *string)
{

    if(!string)
        return;

    char* ptr = string;
    int len = strlen(ptr);

    if(!len){
        return;
    }

    if(!isspace(ptr[0]) && !isspace(ptr[len-1])){
        return;
    }

    while(len-1 > 0 && isspace(ptr[len-1])){
        ptr[--len] = 0;
    }

    while(*ptr && isspace(*ptr)){
        ++ptr, --len;
    }

    memmove(string, ptr, len + 1);
}

typedef struct QueryParam {
    char key[32];
    char value[32];
} QueryParam_t;

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


unsigned int convert_char_to_uint(const char* str) {
    char* endptr;
    unsigned long value = strtoul(str, &endptr, 10); // Base 10 for decimal

    // Check for conversion errors
    if (*endptr != '\0') {
        // Handle error: invalid character found
        return 0; // Or handle the error differently
    }

    // Ensure the value fits in an unsigned int
    if (value > UINT_MAX) {
        // Handle overflow: value too large
        return UINT_MAX; // Or handle the error differently
    }

    return (unsigned int)value;
}

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

void parse_query_string(const char *query_string, unsigned int *id, char **to, char **from) {
    char *token, *key, *value;
    

    char* cquery_string = strdup(query_string);
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
        fprintf(stderr,"param%d: %s ",pc,key);
        value=strtok(NULL,"=");
        fprintf(stderr,"value%d: %s\n",pc,value);
        tok=otok;

        // Save param and the value accordingly
        if (strcmp(key, "id") == 0)
        {
            *id = atoi(value);
        }
        else if (strcmp(key, "to") == 0) {
            *to = strdup(value);
        }
        else if (strcmp(key, "from") == 0) {
            *from = strdup(value);
        }

    };

    // while ((token = strtok(NULL, delimeters)) != NULL) {
    //     key = strtok(token, "=");
    //     value = strtok(NULL, "=");

    //     printf("parsed %s:%s\n", key, value);

    //     if (strcmp(key, "id") == 0) {
    //         printf("Captured string id val: %s\n", value);
    //         *id = atoi(value);
    //     } else if (strcmp(key, "to") == 0) {
    //         *to = strdup(value);
    //     }
    // }
}

char * get_note(unsigned int id)
{
    for(int i = 0; i < 3; i++){
        printf("Searching note: %s\n", notes[i].note);
        if(notes[i].id == id){
            return notes[i].note;
        }
    }
    
    return "Cannot find note";
}

void query_database(unsigned int *ids, const size_t *num_ids, const unsigned int id, const char* to, const char* from)
{

}


char * process_GET_request(char *URL, unsigned int *response_len)
{

    printf("%s(%u) called with URL = %s\n", __FUNCTION__, __LINE__, URL);
    
    char *strid = NULL, *to = NULL, *from = NULL;

    unsigned int id = UINT_MAX;

    // use an array of param structs insteead of hardcoding to, from, id??
    parse_query_string(URL, &id, &to, &from);

    const size_t ARRSIZE = sizeof(notes);
    unsigned int selected_ids[ARRSIZE];

    // Initialize all elements to UINT_MAX
    memset(selected_ids, UINT_MAX, sizeof(selected_ids));

    query_database(selected_ids, &ARRSIZE, id, to, from);

    // unsigned int id = convert_char_to_uint(strid);

    printf("Parsed id: %u\n", id);

    char* a_note = get_note(id);
    
    /*We have got the notes of interest here*/
    char *response = calloc(1, 1024);


    strcpy(response,
        "<html>"
        "<head>"
            "<title>HTML Response</title>"
            "<style>"
            "table, th, td {"
                "border: 1px solid black;}"
             "</style>"
        "</head>"
        "<body>"
        "<table>"
        "<tr>"
        "<td>");

    strcat(response , 
        a_note
    );

    strcat(response ,
        "</td></tr>");
    strcat(response , 
            "</table>"
            "</body>"
            "</html>");

    unsigned int content_len_str = strlen(response);

    /*create HTML hdr returned by server*/
    char *header  = calloc(1, 248 + content_len_str);
    strcpy(header, "HTTP/1.1 200 OK\n");      
    strcat(header, "Server: My Personal HTTP Server\n"    );
    strcat(header, "Content-Length: "  ); 
    strcat(header, "Connection: close\n"   );
    //strcat(header, itoa(content_len_str)); 
    strcat(header, "2048");
    strcat(header, "\n");
    strcat(header, "Content-Type: text/html; charset=UTF-8\n");
    strcat(header, "\n");

    strcat(header, response);
    content_len_str = strlen(header); 
    *response_len = content_len_str;
    free(response);
    return header;
}

char * process_POST_request(char *URL, unsigned int *response_len)
{

    return NULL;
}
