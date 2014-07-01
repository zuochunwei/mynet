#pragma once

#include <sys/epoll.h>
#include <vector>
#include <map>
#include "socketcmd.h"

class socket_base;

class poller
{
public:
	poller();
	poller(const poller&) = delete;
	poller& operator= (const poller&) = delete;
	~poller();

	bool create(int max=8192);
	void close();
	bool poll(int timeout);
	bool send_cmd(const socket_cmd&) const;
	bool test_add() const { return _maxevents > (int)_sockmap.size(); }
	bool add(socket_base*, bool permit_read, bool permit_write);
	bool del(socket_base*);
	bool ctrl(int opt, int fd, unsigned int event, void* ptr);

	void forbid_write(socket_base*);
	void forbid_read(socket_base*);
	void permit_write(socket_base*);
	void permit_read(socket_base*);
private:
	static bool init();

	bool handle_cmd();
	bool cmd_listen(const socket_cmd::listen_param&); 
	bool cmd_connect(const socket_cmd::connect_param&); 
	bool cmd_close(const socket_cmd::close_param&); 

	int _epfd;
	int _pipe[2]; //for cmd, _pipe[0] for read, _pipe[1] for write
	int _maxevents;
	std::vector<epoll_event> _events;
	std::map<int, socket_base*> _sockmap; //fd --> socket_base*
};
