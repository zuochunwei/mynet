#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <netinet/in.h>
#include "protocol.h"
#include "fifo.h"

class poller;
class session;

class session_manager
{
public:
	session_manager(const std::string &id);
	session_manager(const session_manager&) = delete;
	session_manager& operator= (const session_manager&) = delete;
	virtual ~session_manager();

	virtual void on_add_session(unsigned int sid) {}
	virtual void on_del_session(unsigned int sid) {}

	virtual void on_listen_failed() {}
	virtual void on_connect_failed() {}

	bool add_session(session* s);
	bool del_session(session* s);
	bool del_session(unsigned int sid);

	void listen_failed();
	void connect_failed();

	bool add_protocol(unsigned int sid, protocol*);
	bool process_protocol();

	bool send(unsigned int sid, const protocol*);
	bool send(unsigned int sid, const protocol&);

	bool broadcast(const protocol*);
	bool broadcast(const protocol&);

	void launch_server(const char* ip, unsigned short port);
	void launch_client(const char* ip, unsigned short port);
	void close_session(unsigned int id, int reason);

	void set_poller(poller* p) { _poller = p; }
private:
	struct protocol_wrap
	{
		unsigned sid;
		protocol *p;
	};

	std::string _id; //identity
	poller *_poller; //associate poller
	fifo<protocol_wrap> _protocollist;
	std::mutex _protocolmutex;
	std::unordered_map<unsigned, session*> _sessionmap;	//session id -> session*
	std::mutex _sessionmutex;
};
