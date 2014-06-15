#pragma once

#include "poller.h"
#include "sessionmanager.h"

void net_init();
void net_term();
void net_poll(int timeout);

void launch_server(session_manager*, const char* ip, unsigned short port);
void launch_client(session_manager*, const char* ip, unsigned short port);

void launch_server(poller*, session_manager*, const char* ip, unsigned short port);
void launch_client(poller*, session_manager*, const char* ip, unsigned short port);
