#pragma once


// method and URL strings are passed by reference
void process_request_line(char *request_line, char **method, char **URL);
void string_space_trim(char *string);
char * process_GET_request(char *URL, unsigned int *response_len);
char * process_POST_request(char *URL, unsigned int *response_len);

