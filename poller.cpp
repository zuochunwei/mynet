#include <sys/select.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include "poller.h"
#include "session.h"
#include "socketserver.h"
#include "socketclient.h"
#include "socketconnection.h"
#include "sessionmanager.h"

bool poller::init()
{
	signal(SIGPIPE, SIG_IGN);
	return true;
}

poller::poller()
	: _epfd(-1), _pipe{-1,-1},  _maxevents(0), _events()
{
}

poller::~poller()
{
	close();
}

bool poller::create(int max)
{
	static bool i = init();
	(void)i;

	if (pipe(_pipe) == -1)
		return false;

	_epfd = epoll_create(max);
	if (_epfd == -1)
		return false;

	_maxevents = max;
	_events.resize(max);

	return true;
}

void poller::close()
{
	if (_epfd != -1)
	{
		::close(_epfd);
		_epfd = -1;
	}
	if (_pipe[0] != -1)
	{
		::close(_pipe[0]);
		_pipe[0] = -1;
	}
	if (_pipe[1] != -1)
	{
		::close(_pipe[1]);
		_pipe[1] = -1;
	}
	_maxevents = 0;
	_events.clear();

	for (auto a = _sockmap.begin(); a != _sockmap.end(); ++a)
		delete a->second;
	_sockmap.clear();
}

bool poller::poll(int timeout/*milliseconds*/)
{
	if (!handle_cmd())
		return false;

	int n = 0;
	while ((n = epoll_wait(_epfd, &_events[0], _maxevents, timeout)) < 0 && errno == EINTR)
		;

	if (n < 0)
		return false;

	for (int i = 0; i < n; ++i)
	{
		socket_base *s = (socket_base*)_events[i].data.ptr;
		if (_events[i].events & (EPOLLIN | EPOLLERR | EPOLLHUP))
			s->poll_in();
		if (_events[i].events & (EPOLLOUT))
			s->poll_out();
		s->poll_close();
	}

	return true;
}

bool poller::send_cmd(const socket_cmd &c) const
{
	ssize_t n = 0;
	while ((n = write(_pipe[1], &c, sizeof(c))) < 0 && errno == EINTR)
		;

	if (n < 0)
	{
		std::cout << "send_socket_cmd err: " << strerror(errno) << std::endl;
		return false;
	}

	assert(n == sizeof(c));
	return true;
}

bool poller::add(socket_base* s, bool permit_read, bool permit_write)
{
	int fd = s->get_fd();
	if (_sockmap.find(fd) != _sockmap.end())
		return false;

	unsigned int event = 0;
	if (permit_read) event |= EPOLLIN;
	if (permit_write) event |= EPOLLOUT;

	if (!ctrl(EPOLL_CTL_ADD, fd, event, s))
		return false;

	s->set_event(event);
	_sockmap[fd] = s;

	return true;
}

bool poller::del(socket_base* s)
{
	_sockmap.erase(s->get_fd());

	if (s->get_event() && !ctrl(EPOLL_CTL_DEL, s->get_fd(), 0, nullptr))
		return false;

	return true;
}

/*static const char* strevent(unsigned int event)
{
	static char str[64] = {};
	memset(str, 0, sizeof(str));
	if (event & EPOLLIN)
		strcpy(str, "EPOLLIN,");
	if (event & EPOLLERR)
		strcat(str, "EPOLLERR,");
	if (event & EPOLLHUP)
		strcat(str, "EPOLLHUP,");
	if (event & EPOLLOUT)
		strcat(str, "EPOLLOUT,");
	return str;
}*/

bool poller::ctrl(int opt, int fd, unsigned int event, void* ptr)
{
	/*if (opt == EPOLL_CTL_ADD)
		std::cout << "poller::ctrl " << fd << " add " << strevent(event) << std::endl;
	else if (opt == EPOLL_CTL_MOD)
		std::cout << "poller::ctrl " << fd << " mod " << strevent(event) << std::endl;
	else
		std::cout << "poller::ctrl " << fd << " del " << strevent(event) << std::endl;*/

	struct epoll_event ee;
	ee.events = event;
	ee.data.ptr = ptr;
	int n = epoll_ctl(_epfd, opt, fd, &ee);
	return n == 0;
}

void poller::forbid_write(socket_base* sb)
{
	unsigned int event = sb->get_event();
	if (event & EPOLLOUT)
	{
		unsigned int newevent = event & (~EPOLLOUT);
		if (newevent)
			ctrl(EPOLL_CTL_MOD, sb->get_fd(), newevent, sb);
		else
			ctrl(EPOLL_CTL_DEL, sb->get_fd(), 0, sb);

		sb->set_event(newevent);
	}
}

void poller::forbid_read(socket_base* sb)
{
	unsigned int event = sb->get_event();
	if (event & EPOLLIN)
	{
		unsigned int newevent = event & (~EPOLLIN);
		if (newevent)
			ctrl(EPOLL_CTL_MOD, sb->get_fd(), newevent, sb);
		else
			ctrl(EPOLL_CTL_DEL, sb->get_fd(), 0, sb);

		sb->set_event(newevent);
	}
}

void poller::permit_write(socket_base* sb)
{
	unsigned int event = sb->get_event();
	if ((event & EPOLLOUT) == 0)
	{
		unsigned int newevent = event | EPOLLOUT;
		if (event)
			ctrl(EPOLL_CTL_MOD, sb->get_fd(), newevent, sb);
		else
			ctrl(EPOLL_CTL_ADD, sb->get_fd(), newevent, sb);

		sb->set_event(newevent);
	}
}

void poller::permit_read(socket_base* sb)
{
	unsigned int event = sb->get_event();
	if ((event & EPOLLIN) == 0)
	{
		unsigned int newevent = event | EPOLLIN;
		if (event)
			ctrl(EPOLL_CTL_MOD, sb->get_fd(), newevent, sb);
		else
			ctrl(EPOLL_CTL_ADD, sb->get_fd(), newevent, sb);

		sb->set_event(newevent);
	}
}

bool poller::handle_cmd()
{ 
	fd_set rfds;
	FD_ZERO(&rfds);
	FD_SET(_pipe[0], &rfds); 

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	ssize_t n = 0;
	while ((n = select(_pipe[0] + 1, &rfds, nullptr, nullptr, &tv)) < 0 && errno == EINTR)
		;

	if (n < 0)
		return false;

	if (n == 0)
		return true;

	socket_cmd c;
	n = read(_pipe[0], &c, sizeof(c));
	if (n != sizeof(c))
	{
		std::cout << "handle_socket_cmd err, read " << n << " bytes data." << std::endl;
		return false;
	}

	switch (c.type)
	{
		case socket_cmd::type_listen:
			cmd_listen(c.param.listen);
			break;
		case socket_cmd::type_connect:
			cmd_connect(c.param.connect);
			break;
		case socket_cmd::type_close:
			cmd_close(c.param.close);
			break;
		default:
			std::cout << "handle_socket_cmd err, invalid type: " << c.type << "." << std::endl;
			break;
	}

	return true;
} 

bool poller::cmd_listen(const socket_cmd::listen_param& p)
{
	socket_server* ss = new socket_server(p.manager);
	if (!ss->create_socket())
	{
		delete ss;
		p.manager->listen_failed();
		return false;
	}

	ss->set_address(socket_base::make_address(p.ip, p.port));
	ss->set_reuse_addr();
	ss->set_keep_alive();
	ss->set_non_block();

	if (!ss->bind() || !ss->listen(p.backlog))
	{
		std::cout << "cmd_listen failed" << std::endl;
		delete ss;
		p.manager->listen_failed();
		return false;
	}

	bool b = add(ss, true, false);
	ss->set_poller(this);
	assert(b);

	return true;
}

bool poller::cmd_connect(const socket_cmd::connect_param& p)
{
	socket_client *sc = new socket_client(p.manager);
	if (!sc->create_socket())
	{
		delete sc;
		p.manager->connect_failed();
		return false;
	}

	sc->set_address(socket_base::make_address(p.ip, p.port));
	sc->set_keep_alive();
	sc->set_non_block();

	if (!sc->connect())
	{
		std::cout << "cmd_connect failed" << std::endl;
		delete sc;
		p.manager->connect_failed();
		return false;
	}

	bool b = add(sc, true, true);
	sc->set_poller(this);
	assert(b);

	return true;
}

bool poller::cmd_close(const socket_cmd::close_param& p)
{
	auto a = _sockmap.find(p.fd);
	if (a == _sockmap.end())
		return false;

	a->second->set_close(error_active_close);

	return true;
}

