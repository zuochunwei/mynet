#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <assert.h>
#include "socketclient.h"
#include "socketconnection.h"
#include "session.h"
#include "sessionmanager.h"
#include "poller.h"

socket_client::socket_client(session_manager *m)
	: _manager(m)
{
}

socket_client::~socket_client()
{
	if (_close)
	{
		if (get_opt_int(SO_ERROR))
		{
			_error = error_connect_failed;
			std::cout << "socket_client connect failed, reason" << strerror(errno) << std::endl;
		}
		else
		{
			int newfd = dup(_fd);

			sockaddr_in addr;
			socklen_t len = sizeof(addr);
			getpeername(newfd, (sockaddr*)&addr, &len);

			socket_connection* conn = new socket_connection;
			conn->set_fd(newfd);
			conn->set_address(addr);
			conn->set_non_block();
			conn->set_keep_alive();

			bool b = _poller->add(conn, true, true);
			conn->set_poller(_poller);
			assert(b);
			b = conn->get_session()->add_to_manager(_manager);
			assert(b);
		}
	}
}

int socket_client::poll_in()
{
	set_close(0);
	return -1;
}

int socket_client::poll_out()
{
	set_close(0);
	return -1;
}

bool socket_client::connect()
{
	while (true)
	{
		if (::connect(_fd, (const sockaddr *)&_addr, sizeof(_addr)) == 0)
			return true;
		if (errno == EINTR)
			continue;
		if (errno == EINPROGRESS)
			return true;
		std::cout << "socket_client::connect() err:" << strerror(errno) << std::endl;
		return false;
	}
}
