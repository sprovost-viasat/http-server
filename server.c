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

typedef struct student_{

    char name[32];
    unsigned int roll_no;
    char hobby[32];
    char dept[32];
} student_t; 

student_t student[5] = {
    {"Abhishek", 10305042, "Programming", "CSE"},
    {"Nitin", 10305048, "Programming", "CSE"},
    {"Avinash", 10305041, "Cricket", "ECE"},
    {"Jack", 10305032, "Udemy Teaching", "Mechanical"},
    {"Cris", 10305030, "Programming", "Electrical"}};


char * process_GET_request(char *URL, unsigned int *response_len)
{

    printf("%s(%u) called with URL = %s\n", __FUNCTION__, __LINE__, URL);
    
    /*Let us extract the roll no of a students from URL using 
     * string handling 
     *URL : /College/IIT/?dept=CSE&rollno=10305042/
     * */
    char delimeter[2] = {'?', '\0'};

    string_space_trim(URL);
    char *token[5] = {0};

    token[0] = strtok(URL, delimeter);
    token[1] = strtok(0, delimeter);
    /*token[1] = dept=CSE&rollno=10305042*/
    delimeter[0] = '&';

    token[2] = strtok(token[1], delimeter);
    token[3] = strtok(0, delimeter);
    /*token[2] = dept=CSE, token[3] = rollno=10305042*/

    printf("token[0] = %s, token[1] = %s, token[2] = %s, token[3] = %s\n",
        token[0] , token[1], token[2], token[3]);

    delimeter[0] = '=';
    char *roll_no_str = strtok(token[3], delimeter);
    char *roll_no_value = strtok(0, delimeter);
    printf("roll_no_value = %s\n", roll_no_value);
    unsigned int roll_no = atoi(roll_no_value), i = 0;

    for(i = 0; i < 5; i++){
        if(student[i].roll_no != roll_no){
            continue;
        }
        break;
    }
    
    if(i == 5)
        return NULL;
    
    /*We have got the students of interest here*/
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
            student[i].name
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
