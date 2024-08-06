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
#include "multiplex.h"


/* Define the port which the client has to send data to */
#define SERVER_PORT 2000
#define CONNECTIONS_QUEUE 5
#define MAX_SUPPORTED_CLIENTS 32

char DATA_BUFFER[1024];         // data buffer for data from client
char ip_str[INET_ADDRSTRLEN];   // client IP address buffer (formatted string)

SocketAddress monitored_fd_set[MAX_SUPPORTED_CLIENTS];

int setup_tcp_socket_connection(struct sockaddr_in *server_addr)
{

    /* Master socket file descriptor, used to accept new client connection only, no data exchange */
    int master_socket_fd = 0;
    int opt = 1;

    /* tcp master socket creation */
    if ((master_socket_fd = socket(
        AF_INET,
        SOCK_STREAM,
        IPPROTO_TCP)
    ) == -1)
    {
        printf("socket creation failed\n");
        exit(1);
    }
    
    // set master socket to allow multiple connections
    if (setsockopt(
        master_socket_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char *)&opt,
        sizeof(opt)
    ) < 0)  
    {
        printf("TCP socket creation failed for multiple connections\n");
        exit(EXIT_FAILURE);
    }

    server_addr->sin_family = AF_INET;          //This socket will process only ipv4 network packets
    server_addr->sin_port = htons(SERVER_PORT); //Server will process any data arriving on port SERVER_PORT
    server_addr->sin_addr.s_addr = INADDR_ANY; 

    if (bind(
        master_socket_fd,
        (struct sockaddr *)server_addr,
        sizeof(struct sockaddr)
    ) == -1)
    {
        printf("socket bind failed\n");
        return -1;
    }

    // Tell the Linux OS to maintain the queue of max length to Queue incoming client connections 
    if (listen(master_socket_fd, 5) < 0)  
    {
        printf("listen failed\n");
        return -1;
    }

    

    return master_socket_fd;  
}


void service_client(const int master_socket_fd)
{
    print_available_interfaces(SERVER_PORT);
    printf("Server ready to service client msgs.\n");
    /*client specific communication socket file descriptor, 
     * used for only data exchange/communication between client and server*/
    

    int sent_recv_bytes = 0;
    int addr_len = sizeof(struct sockaddr);

    fd_set readfds;

    /* Server infinite loop for servicing the client*/

    while(1){

        // clear the readfds set then re-add all the active FDs
        re_init_fds(
            monitored_fd_set,
            MAX_SUPPORTED_CLIENTS,
            &readfds
        );

        /* Wait for client connection */
        
        select(                                 // process waits for any request from a FD in the readfds set
            get_max_fd(
                monitored_fd_set,
                MAX_SUPPORTED_CLIENTS
            ) + 1,                              // provide the number for creating the next FD
            &readfds,                           // provide the set of FDs
            NULL, NULL, NULL
        );

        if (FD_ISSET(                           // check which FD in readfds set is activated
                master_socket_fd,               // provide the master FD to check if that one is active
                &readfds))                      // within the FD set
        {                                       // master socket FD being active means a new connection request
            printf("New connection received! Accepting the connection..\n");
            struct sockaddr_in client_addr; /*structure to store the server and client info*/

            /* Create a temp file descriptor for the rest of the connections life */

            int comm_socket_fd = accept(        // accept the connection and return the FD
                master_socket_fd,               // master FD only used for accepting the new clients connection
                (struct sockaddr*)&client_addr, // pass empty client_addr to be populated with IP address & port
                &addr_len                       // const size of sockaddr
            );

            if (comm_socket_fd < 0)             // check accept didn't fail creating a FD
            {
                printf("accept error: errno=%d\n", errno);
                exit(0);
            }

            add_to_monitored_fd_set(
                monitored_fd_set,
                MAX_SUPPORTED_CLIENTS,
                comm_socket_fd,
                client_addr
            );

            printf("Connection accepted from client: %s:%u\n",
                inet_ntoa(client_addr.sin_addr),// print IP address with "x.x.x.x" format
                // ntohs(client_addr.sin_port));   // convert port into readable integer
                client_addr.sin_port);
        }
        else {
            int comm_socket_fd = -1;
            for (int i = 0; i < MAX_SUPPORTED_CLIENTS; i++)
            {
                if (FD_ISSET(                   // find which comm socket FD is active in monitored_fd_set
                    monitored_fd_set[i].fd,     // use the array of FD integers to check which one
                    &readfds                    // is active in readfds
                ))
                {
                    comm_socket_fd = monitored_fd_set[i].fd;
                    struct sockaddr_in curr_client = monitored_fd_set[i].client;

                    memset(                     // prepare memory space for server to store data
                        DATA_BUFFER,            // received from the client of size DATA_BUFFER
                        0,
                        sizeof(DATA_BUFFER));

                    if (inet_ntop(              // Convert uint32_t IP address into readable string, return NULL on error
                            AF_INET,            // specify IPv4 address and the clients uint32_t IP address
                            &(curr_client.sin_addr),
                            ip_str,             // the ip buffer to be assigned the IP string
                            sizeof(ip_str)      // size of the buffer
                        ) == NULL)              // NULL means it failed to convert address to string
                    {
                        perror("inet_ntop");
                        printf("Error converting network address uint to a string\n");
                        strcpy(ip_str, "null");
                    }

                    /* Server receiving data from the client */

                    sent_recv_bytes = recv(     // TCP method for retrieving data
                        comm_socket_fd,         // all communciation happens on the communication FD (not master FD)
                        (char*)DATA_BUFFER,     // the location which the data is going to be stored
                        sizeof(DATA_BUFFER),    // how many bytes long is this data buffer
                        0
                    );

                    /* state Machine state 4*/
                    printf("Server received %d bytes from client %s:%u\n",
                        sent_recv_bytes,
                        ip_str,
                        // ntohs(curr_client.sin_port));    
                        curr_client.sin_port);      

                    if(sent_recv_bytes == 0){
                        close(comm_socket_fd);
                        remove_from_monitored_fd_set(
                            monitored_fd_set,
                            MAX_SUPPORTED_CLIENTS,
                            comm_socket_fd
                        );
                        printf("Server closed connection with client %s:%u\n",
                            ip_str,
                            // ntohs(curr_client.sin_port));
                            curr_client.sin_port);

                        break;
                    }

                    printf("Msg recieved:\n%s\n", DATA_BUFFER);
                    char *request_line = NULL;
                    char *method = NULL;
                    char *URL = NULL;

                    char del[] = "\n";

                    char *buffer_copy = strdup(DATA_BUFFER);

                    request_line = strtok(buffer_copy, del);

                    process_request_line(request_line, &method, &URL);

                    printf("Method = %s\n", method);
                    printf("URL = %s\n", URL);
                    char *response = NULL;
                    unsigned int response_length = 0 ;

                    if(strncmp(method, "GET", strlen("GET")) == 0){
                        response = process_GET_request(URL, &response_length);
                    }
                    else if(strncmp(method, "POST", strlen("POST")) == 0){
                        response = process_POST_request(DATA_BUFFER, &response_length);
                    }
                    else{
                        printf("Unsupported URL method request\n");
                        close(comm_socket_fd);
                        break;
                    }

                    free(buffer_copy);


                    /* Server replying back to client now*/
                    if(response)
                    {
                        // printf("response to be sent to client = \n%s", response);
                        sent_recv_bytes = sendto(
                            comm_socket_fd,
                            response,
                            response_length,
                            0,
                            (struct sockaddr *)&curr_client,
                            sizeof(struct sockaddr)
                        );
                        free(response);
                        printf("Server sent %d bytes in reply to client\n", sent_recv_bytes);                        
                    }
                }
            }
        }
    }/*step 10 : wait for new client request again*/    
}

int main(int argc, char **argv)
{
    /* Set of file descriptor on which select() polls. Select() unblocks whever data arrives on 
     * any fd present in this set*/
     fd_set readfds;             
    /*variables to hold server information*/
    struct sockaddr_in server_addr; /*structure to store the server and client info*/

    int master_socket_fd = setup_tcp_socket_connection(&server_addr);

    if (master_socket_fd < 0)
    {
        return 1;
    }

    // Set all values in monitored_fd_set to -1
    init_monitor_fd_set(
        monitored_fd_set,
        MAX_SUPPORTED_CLIENTS
    );

    // Add master socket DF to set being monitored
    add_to_monitored_fd_set(
        monitored_fd_set,
        MAX_SUPPORTED_CLIENTS,
        master_socket_fd,
        server_addr
    );

    service_client(master_socket_fd);
    return 0;
}