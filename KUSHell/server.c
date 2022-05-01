//
//  server.c
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#include "server.h"

int num_of_tokens;
int num_of_commands;
struct pollfd fd_list[MAX_CLIENTS + 2];
int server_socket;
int piped_command;

void start_server_socket(ARGUMENTS *args) {
    struct sockaddr_in local;
    
    server_socket = socket(AF_INET,SOCK_STREAM,0);
    
    if ( server_socket < 0 ) {
        perror("Socket failed to create...\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Socket created...\n");
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = (strlen(args->socket_address) > 0) ? inet_addr(args->socket_address) : htonl(INADDR_ANY);
    local.sin_port = htons(args->socket_port);
    
    // 3. Binding port number
    if (bind(server_socket,(struct sockaddr *)&local, sizeof(local)) < 0) {
        perror("Server bind failed.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Socket binded...\n");

    // 4. Get a listening socket
    if (listen(server_socket, 32) < 0) {
        perror("Server listen failed.\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening...\n");
    
    // init FDs
    for(int i = 0; i < MAX_CLIENTS; i++) {
        fd_list[i].fd = -1;// File descriptor
        fd_list[i].events = 0;// Set of events to monitor
        fd_list[i].revents = 0;// Ready Event Set of Concerned Descriptors
    }
    
    // FD for server's STDIN
    fd_list[0].fd = 0;
    fd_list[0].events = (POLLIN | POLLPRI);
    
    // FD for new connections
    fd_list[1].fd = server_socket;
    fd_list[1].events = (POLLIN | POLLPRI);
 
}

void server_help()
{
    printf("server help\n");
    return;
}

void server_halt()
{
    printf("server halt\n");
    return;
}

void server_stat()
{
    printf("server stat\n");
    return;
}


char **tokenizer(char *input)
{
    char *copied_input = malloc(strlen(input)*sizeof(char));
    strcpy(copied_input, input);
    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    //char delim = " ";
    num_of_tokens = 0;
    num_of_commands = 0;
    
    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    token = strtok(copied_input, " ");
    while (token != NULL) {
        tokens[position] = token;
        if ( strcmp("|", token) == 0 || strcmp(";", token) == 0) num_of_commands++;
        position++;
        
        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        
        token = strtok(NULL, " ");
    }
    tokens[position] = NULL;
    num_of_tokens = position;
    num_of_commands++;
    
    // TODO: escaping
    /*
     for (int i = 0; i < position; i++) {
     
     }*/
    
    
    return tokens;
}

COMMAND **parse_tokens(char **tokens)
{
    COMMAND **commands = malloc(num_of_commands * sizeof(COMMAND*));
    int j = 0;
    
    for (int i = 0; i < num_of_tokens; i++) {
        
        if (i == 0) {
            commands[j] = malloc(sizeof(COMMAND));
            commands[j]->program_name = malloc(strlen(tokens[i])*sizeof(char));
            strcpy(commands[j]->program_name, tokens[i]);
            commands[j]->argc = 0;
            continue;
        }
        else if ((strcmp("|", tokens[i]) == 0 || strcmp(";", tokens[i]) == 0) && i+1 < num_of_tokens) {
            if ((strcmp("|", tokens[i]) == 0)) piped_command = 1;
            j++;
            commands[j] = malloc(sizeof(COMMAND));
            commands[j]->program_name = malloc(strlen(tokens[i+1])*sizeof(char));
            strcpy(commands[j]->program_name, tokens[i+1]);
            commands[j]->argc = 0;
            i++;
            continue;
        }
        
        if (commands[j]->argc == 0) {
            commands[j]->argv = malloc(sizeof(char*));
        } else if (commands[j]->argc > 0) {
            realloc(commands[j]->argv, commands[j]->argc);
        }
        
        commands[j]->argv[commands[j]->argc] = malloc(strlen(tokens[i])*sizeof(char));
        strcpy(commands[j]->argv[commands[j]->argc], tokens[i]);
        commands[j]->argc++;
    }
    
    return commands;
}

void handle_server_command(char *command)
{
    
    if (strcmp(command, "help") == 0) {
        server_help();
    }
    else if (strcmp(command, "halt") == 0) {
        server_halt();
    }
    else if (strcmp(command, "stat") == 0) {
        server_stat();
    } else {
        printf("Unknown command.\n");
    }
    
    
    return;
}




char *execute_commands(COMMAND **commands)
{
    return "HAHAHAHAH U GOT FUCKED";
}

char *handle_command(char *command, int client_id)
{
    char *output = NULL;
    
    if (strlen(command) == 0)
        return 0; // return but do not close client socket
    
    
    // the client exited program, kill the socket afterwards
    if (!(strcmp(command, "quit")))
        return "quit";
    
    char **tokens = tokenizer(command);
    COMMAND **commands = parse_tokens(tokens);
    
    output = execute_commands(commands);
    
    return output; // do not close client socket
}

void close_client_connection(int client_id)
{
    if(fd_list[client_id].fd != -1) {
        return;
    }
    
    //close the socket and reset FD for future use
    close(fd_list[client_id].fd);
    fd_list[client_id].fd = -1;
    fd_list[client_id].revents = 0;
    fd_list[client_id].events = 0;
    
    //TODO: release client IP from connected_clients array
    
}

void server_loop(ARGUMENTS *args)
{
    int i;
    char server_command[1024];
    char client_command[1024];
    char *exec_output;
    
    print_prompt();
    while(1) {
        
        //4. Start calling poll and wait for the file descriptor set of interest to be ready
        switch( poll(fd_list, MAX_CLIENTS + 2, 1000) ) {
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
                // stdin
                if((fd_list[0].revents & POLLIN)) {
                    fgets(server_command, 1024, stdin);
                    server_command[strcspn(server_command, "\n")] = 0;
                    handle_server_command(server_command);
                    print_prompt();
                }
                
                // server socket
                if(fd_list[1].fd == server_socket && (fd_list[1].revents & POLLIN)) {
                    
                    // 1. Provide a connection acceptance service if the listening socket is ready to read
                    struct sockaddr_in client;
                    socklen_t len = sizeof(client);
                    int new_sock = accept(server_socket,(struct sockaddr *)&client,&len);
                    
                    if(new_sock < 0) {
                        perror("Client accept failed...\n ");
                        continue;
                    }
                    
                    // find free FDs
                    for(i = 0; i < MAX_CLIENTS; i++)
                        if( fd_list[i].fd == -1 )//Place the first value in the array at - 1
                            break;
                    
                    if(i < MAX_CLIENTS) {
                        // add new client socket to list
                        fd_list[i].fd = new_sock;
                        fd_list[i].events = POLLIN;
                    }
                    else {
                        // close the connection if MAX_CLIENTS is reached
                        close(new_sock);
                    }
                    printf("\ngot a new connection![%s:%d]\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
                    print_prompt();
                    continue;
                }
                
                
                for(i = 2; i < MAX_CLIENTS + 2; i++) {
                    
                    // skip empty FDs
                    if(fd_list[i].fd == -1)
                        continue;
                    
                    //2. At this point, we are concerned with ordinary file descriptors.
                    //   Provide services to read data at this time
                    if( fd_list[i].revents & POLLIN ) {
                        
                        ssize_t s = read(fd_list[i].fd, client_command, sizeof(client_command)-1);
                        if( s < 0 ) {
                            printf("read fail...\n");
                            continue;
                        }
                        else if (s > 0) {
                            //buf[s] = 0;
                            printf("client[%d] command: %s\n", i-2, client_command);
                            exec_output = handle_command(client_command, i);
                            
                            if (strcmp(exec_output, "quit") == 0) {
                                close_client_connection(i);
                            } else {
                                write(fd_list[i].fd, exec_output, strlen(exec_output)+1);
                            }
                            print_prompt();
                        }
                        
                        memset(client_command, 0, 1024);
                    }
                } // end for
                
                //
                
                break;
            } // end default case
        } // end switch
       
        
        // TODO: keep-alive check
        
        
    } // end while loop
        

    return;
}

void init_server(ARGUMENTS *args) {
    
    start_server_socket(args);
    server_loop(args);
    return;
}
