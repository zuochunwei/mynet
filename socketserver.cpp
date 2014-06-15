#include <string.h>
#include <iostream>
#include <errno.h>
#include <assert.h>
#include <sys/epoll.h>
#include "socketserver.h"
#include "socketio.h"
#include "session.h"
#include "poller.h"
#include "sessionmanager.h"

socket_server::socket_server(session_manager *m)
	: _manager(m)
{
}

int socket_server::poll_in()
{
	sockaddr_in addr;
	int newfd = accept(&addr);
	if (newfd > 0)
	{
		socket_io* sio = new socket_io;
		sio->set_fd(newfd);
		sio->set_address(addr);
		sio->set_non_block();
		sio->set_keep_alive();

		bool b = _poller->add(sio, true, true);
		sio->set_poller(_poller);
		assert(b);
		b = sio->get_session()->add_to_manager(_manager);
		assert(b);
		return newfd;
	}
	return -1;
}

int socket_server::poll_out()
{
	std::cout << "socket_server::pull_out() err" << std::endl;
	return -1;
}

bool socket_server::bind()
{
	if (::bind(_fd, (const struct sockaddr *)&_addr, sizeof(_addr)) == 0)
		return true;
	std::cout << "bind error: " << strerror(errno) << std::endl;
	return false;
}

bool socket_server::listen(int backlog)
{
	if (::listen(_fd, backlog) == 0)
		return true;
	std::cout << "listen error: " << strerror(errno) << std::endl;
	return false;
}

int socket_server::accept(sockaddr_in *addr)
{
	socklen_t len = addr ? sizeof(*addr) : 0;
	int newfd = ::accept(_fd, (struct sockaddr *)addr, &len);
	if (newfd < 0)
		std::cout << "accept error: " << strerror(errno) << std::endl;
	return newfd; 
}
