#include <iostream>
#include <atomic>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include "socketbase.h"
#include "poller.h"

static std::atomic_uint atomic_socket_id(0);

static unsigned int new_socket_id()
{
	return  ++atomic_socket_id;
}

socket_base::socket_base() 
	: _id(new_socket_id()), _fd(-1), _error(0), _event(0), _poller(nullptr), _close(false)
{
	memset(&_addr, 0, sizeof(_addr));
}

socket_base::~socket_base()
{
	close_socket();
}

int socket_base::poll_close()
{
	if (_close)
	{
		bool b = _poller->del(this); 
		assert(b);
		delete this;
	}
	return 0;
}

void socket_base::forbid_write()
{
	_poller->forbid_write(this);
}

void socket_base::forbid_read()
{
	_poller->forbid_read(this);
}

void socket_base::permit_write()
{
	_poller->permit_write(this);
}

void socket_base::permit_read()
{
	_poller->permit_read(this);
}

bool socket_base::create_socket()
{
	_fd = socket(PF_INET, SOCK_STREAM, 0/*IPPROTO_TCP*/);
	if (_fd == -1)
	{
		std::cout << "create_socket() errno:" << strerror(errno) << std::endl;
		return false;
	}
	return true;
}

bool socket_base::close_socket()
{
	if (_fd != -1)
	{
		while (::close(_fd) == -1 && errno == EINTR)
			;
		_fd = -1;
		return true;
	}
	return false;
}

void socket_base::set_close(int error)
{
	_error = error;
	_close = true;
}

int socket_base::get_opt_int(int optname) const
{
	int value = 0;
	socklen_t len = 0;
	::getsockopt(_fd, SOL_SOCKET, optname, &value, &len);
	return value;
}

void socket_base::set_non_block()
{
	fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | O_NONBLOCK);
}

void socket_base::set_reuse_addr()
{
	set_opt_int(SO_REUSEADDR, 1);
}

void socket_base::set_keep_alive()
{
	set_opt_int(SO_KEEPALIVE, 1);
}

void socket_base::set_send_buf(int size)
{
	set_opt_int(SO_SNDBUF, size);
}

void socket_base::set_recv_buf(int size)
{
	set_opt_int(SO_RCVBUF, size);
}

void socket_base::set_opt_int(int optname, int value)
{
	setsockopt(_fd, SOL_SOCKET, optname, &value, sizeof(value));
}

sockaddr_in socket_base::make_address(const char* ip, unsigned short port)
{
	sockaddr_in out;
	out.sin_family = AF_INET;
	out.sin_port = htons(port);
	if (inet_aton(ip, &out.sin_addr) == 0)
		out.sin_addr.s_addr = INADDR_ANY;
	return out;
}
