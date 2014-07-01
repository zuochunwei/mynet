#include <unistd.h>
#include <errno.h>
#include "socketconnection.h"
#include "session.h"

socket_connection::socket_connection()
	: _session(new session)
{
	_session->set_socket(this);
}

socket_connection::~socket_connection()
{
	delete _session;
}

int socket_connection::poll_in()
{
	return _session->poll_in();
}

int socket_connection::poll_out()
{
	return _session->poll_out();
}

int socket_connection::poll_close()
{
	if (_close)
		_session->del_from_manager();
	socket_base::poll_close();
	return 0;
}

int socket_connection::read(void* buf, unsigned int len)
{
	while (true)
	{
		ssize_t n = ::read(_fd, buf, len);
		if (n < 0 && errno == EINTR)
			continue;
		return n;
	}
}

int socket_connection::write(const void* buf, unsigned int len)
{
	while (true)
	{
		ssize_t n = ::write(_fd, buf, len);
		if (n < 0 && errno == EINTR)
			continue;
		return n;
	}
}
