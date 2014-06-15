#include <unistd.h>
#include <errno.h>
#include "socketio.h"
#include "session.h"

socket_io::socket_io()
	: _session(new session)
{
	_session->set_socket(this);
}

socket_io::~socket_io()
{
	delete _session;
}

int socket_io::poll_in()
{
	return _session->poll_in();
}

int socket_io::poll_out()
{
	return _session->poll_out();
}

int socket_io::poll_close()
{
	if (_close)
		_session->del_from_manager();
	socket_base::poll_close();
	return 0;
}

int socket_io::read(void* buf, unsigned int len)
{
	while (true)
	{
		ssize_t n = ::read(_fd, buf, len);
		if (n < 0 && errno == EINTR)
			continue;
		return n;
	}
}

int socket_io::write(const void* buf, unsigned int len)
{
	while (true)
	{
		ssize_t n = ::write(_fd, buf, len);
		if (n < 0 && errno == EINTR)
			continue;
		return n;
	}
}
