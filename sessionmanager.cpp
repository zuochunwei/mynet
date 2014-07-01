#include <unistd.h>
#include <iostream>
#include <string.h>
#include "sessionmanager.h"
#include "session.h"
#include "protocol.h"
#include "socketcmd.h"
#include "socketconnection.h"
#include "poller.h"

session_manager::session_manager(const std::string &id)
	: _id(id), _poller(nullptr), _protocollist(1024*64, 1024*1024)
{
}

session_manager::~session_manager()
{
}

bool session_manager::add_session(session *s)
{
	unsigned int id = s->get_id();

	{
		std::lock_guard<std::mutex> lock(_sessionmutex);
		auto a = _sessionmap.find(id);
		if (a != _sessionmap.end())
			return false;

		_sessionmap[id] = s;
		std::cout << "manager:" << _id << " add session:" << id << std::endl;
	}
	
	std::cout << "new connection established:" << id << std::endl; 
	on_add_session(id);

	return true;
}

bool session_manager::del_session(session* s)
{
	int id = s->get_id();
	std::cout << "manager:" << _id << " del session:" << id << std::endl;

	{
		std::lock_guard<std::mutex> lock(_sessionmutex);
		_sessionmap.erase(id);
	}

	std::cout << "connection disconnect, error:" << s->get_socket()->get_error() << std::endl; 
	on_del_session(id);

	return true;
}

bool session_manager::del_session(unsigned int id)
{
	std::lock_guard<std::mutex> lock(_sessionmutex);
	auto a = _sessionmap.find(id);
	if (a != _sessionmap.end())
	{
		del_session(a->second);
		return true;
	}
	return false;
}

void session_manager::listen_failed()
{
	on_listen_failed();
}

void session_manager::connect_failed()
{
	on_connect_failed();
}

bool session_manager::add_protocol(unsigned int sid, protocol* p)
{
	std::lock_guard<std::mutex> lock(_protocolmutex);
	if (_protocollist.full())
	{
		std::cout << "manager:" << _id << " have too much protocol waiting to process" << std::endl;
		return false;
	}

	protocol_wrap w{sid, p};
	_protocollist.push(w);

	return true;
}

bool session_manager::process_protocol()
{
	unsigned sid = 0;
	protocol* p = nullptr;
	{
		std::lock_guard<std::mutex> lock(_protocolmutex);
		if (!_protocollist.empty())
		{
			auto a = _protocollist.pop();
			sid = a.sid;
			p = a.p;
		}
	}

	if (p)
	{
		p->process(sid, this);
		delete p;
		return true;
	}

	return false;
}

bool session_manager::send(unsigned int sid, const protocol* p)
{
	std::lock_guard<std::mutex> lock(_sessionmutex);
	auto a = _sessionmap.find(sid);
	if (a == _sessionmap.end())
		return false;
	return a->second->send(p);
}

bool session_manager::send(unsigned int sid, const protocol& p)
{
	return send(sid, &p);
}

bool session_manager::broadcast(const protocol* p)
{
	bool ret = true;
	std::lock_guard<std::mutex> lock(_sessionmutex);
	for (auto a = _sessionmap.begin(); a != _sessionmap.end(); ++a)
		ret = a->second->send(p) && ret;
	return ret;
}

bool session_manager::broadcast(const protocol& p)
{
	return broadcast(&p);
}

void session_manager::launch_server(const char* ip, unsigned short port)
{
	socket_cmd c;
	c.type = socket_cmd::type_listen;
	c.param.listen.manager = this;
	strncpy(c.param.listen.ip, ip, INET_ADDRSTRLEN);
	c.param.listen.port = port;
	c.param.listen.backlog = 1024;
	_poller->send_cmd(c);
}

void session_manager::launch_client(const char* ip, unsigned short port)
{
	socket_cmd c;
	c.type = socket_cmd::type_connect;
	c.param.connect.manager = this;
	strncpy(c.param.connect.ip, ip, INET_ADDRSTRLEN);
	c.param.connect.port = port;
	_poller->send_cmd(c);
}

void session_manager::close_session(unsigned int id, int reason)
{
	std::lock_guard<std::mutex> lock(_sessionmutex);
	auto a = _sessionmap.find(id);
	if (a != _sessionmap.end())
	{
		socket_cmd c;
		c.type = socket_cmd::type_close;
		c.param.close.fd = a->second->get_socket()->get_fd();
		c.param.close.reason = reason;
		_poller->send_cmd(c);
	}
}
