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
        printf("init tok:%s\n", tok);
        pc++;
        otok=tok+strlen(tok)+1;
        printf("init otok:%s\n", otok);
        key=strtok(tok,"=");
        fprintf(stderr,"param%d: %s ",pc,key);
        value=strtok(NULL,"=");
        fprintf(stderr,"value%d: %s\n",pc,value);
        tok=otok;

        if (!value || !key) {
            printf("Error with a ghost param! {%s:%s}\n", key, value);
            continue;
        }

        printf("end tok:%s\n", tok);

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

        // OLD METHOD
        // Save param and the value accordingly
        // if (strcmp(key, "id") == 0)
        // {
        //     *id = atoi(value);
        // }
        // else if (strcmp(key, "to") == 0) {
        //     *to = strdup(value);
        // }
        // else if (strcmp(key, "from") == 0) {
        //     *from = strdup(value);
        // }

    };
    return head;
}
