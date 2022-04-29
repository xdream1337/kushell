//
//  server.h
//  KUSHell
//
//  Created by Jakub Taraba on 28/04/2022.
//

#ifndef server_h
#define server_h

#include <stdio.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "helpers.h"
#define MAX_CLIENTS 32

int server_socket;
struct pollfd fd_list[MAX_CLIENTS];
void init_server(ARGUMENTS *args);
void start_server_socket(ARGUMENTS *args);
void handle_communication(ARGUMENTS *args);

#endif /* server_h */

