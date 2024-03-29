//
//  client.c
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#include "client.h"

int client_socket;
struct pollfd client_fd_list[2];

void client_halt(int num)
{
    write(client_socket, "quit", 5);
    close(client_socket);
    printf("\nDisconnecting from server...\n");
    exit(EXIT_SUCCESS);
}

void init_client(ARGUMENTS *args)
{
    signal(SIGINT, client_halt);
    connect_to_server(args);
    
    // FD for client's STDIN
    client_fd_list[0].fd = 0;
    client_fd_list[0].events = (POLLIN | POLLPRI);
    
    // FD for new connections
    client_fd_list[1].fd = client_socket;
    client_fd_list[1].events = (POLLIN | POLLPRI);
    
    client_loop();
}

void client_help()
{
    printf("Available commands as client:\n- quit => end the connection and quit.\n");
}

void client_loop()
{
    int i = 0;
    char buffer[1024];
    print_prompt();
    while(1) {
        //4. Start calling poll and wait for the file descriptor set of interest to be ready
        switch( poll(client_fd_list, MAX_CLIENTS, 3000) ) {
            case 0: {
                //printf("timeout...\n");
                continue;
            }
            case -1: {
                printf("poll fail...\n");
                continue;
            }
            default: { // poll is successfull
                //   If it is a listener file descriptor, call accept to accept a new connection
                //   If it is a normal file descriptor, read is called to read the data
                for(i = 0; i < 2; i++) {
                    // stdin
                    if(i == 0 && (client_fd_list[0].revents & POLLIN)) {
                        //server_prompt();
                        fgets(buffer, 1024, stdin);
                        buffer[strcspn(buffer, "\n")] = 0;
                        write(client_socket, buffer, strlen(buffer)+1);
                        
                        if(strcmp(buffer, "quit") == 0) {
                            close(client_socket);
                            exit(EXIT_SUCCESS);
                        } else if (strcmp(buffer, "help") == 0) {
                            client_help();
                            print_prompt();
                        }
                        
                        //print_prompt();
                    }
                    // server socket
                    if(i == 1 && (client_fd_list[1].revents & POLLIN)) {
                        char buf[16384];
                        ssize_t size = read(client_fd_list[1].fd,buf,sizeof(buf)-1);
                        if (size > 0) {
                            buf[size] = '\0';
                            printf("\n%s", buf);
                            if (strcmp("Closing server", buf) == 0 || strcmp("Disconnected due to timeout", buf) == 0) {
                                close(client_socket);
                                exit(EXIT_SUCCESS);
                            }
                            print_prompt();
                            memset(buf, 0, 16384);
                        }
                    }
                } // end for
                
                break;
            } // end default case
                
        } // end switch
    } // end while loop
    
    
    return;
}

void connect_to_server(ARGUMENTS *args)
{
    
    if (args->socket_path && strlen(args->socket_path) > 0) {
        struct sockaddr_un servaddr;
        client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        servaddr.sun_family = AF_UNIX;
        strcpy(servaddr.sun_path, args->socket_path);
        
        if (connect(client_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
            printf("Connection with the server failed...\n");
            exit(EXIT_FAILURE);
        }
        else
            printf("connected to the server..\n");
    } else {
        struct sockaddr_in servaddr;
        client_socket = socket(AF_INET, SOCK_STREAM, 0);
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = (strlen(args->socket_address) > 0) ? inet_addr(args->socket_address) : htonl(INADDR_ANY);
        servaddr.sin_port = htons(args->socket_port);
        
        
        if (connect(client_socket, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0) {
            printf("Connection with the server failed...\n");
            exit(EXIT_FAILURE);
        }
        else
            printf("connected to the server..\n");
    }
}

