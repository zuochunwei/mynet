#pragma once

#include <netinet/in.h>

struct socket_cmd
{
	enum cmd_type
	{
		type_listen,
		type_connect,
		type_close,
	};

	struct listen_param
	{
		class session_manager* manager;
		char ip[INET_ADDRSTRLEN];
		unsigned short port;
		int backlog;
	};

	struct connect_param
	{
		class session_manager* manager;
		char ip[INET_ADDRSTRLEN];
		unsigned short port;
	};

	struct close_param
	{
		int fd;
		int reason;
	};

	cmd_type type;

	union
	{
		listen_param listen;
		connect_param connect;
		close_param close;
	} param;
};
