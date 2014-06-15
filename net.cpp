#include <string.h>
#include <stdlib.h>
#include <iostream>
#include "net.h"
#include "socketcmd.h"
#include "protocolfactory.h"

static poller* s_poller = nullptr;

void net_init()
{
	protocol_factory_manager::instance() = new protocol_factory_manager;

	/*s_poller = new poller;
	if (!s_poller->create(8096))
	{
		std::cout << "net_init abort" << std::endl;
		abort();
	}*/
}

void net_term()
{
	/*if (s_poller)
	{
		s_poller->close();
		delete s_poller;
		s_poller = nullptr;
	}*/

	delete protocol_factory_manager::instance();
	protocol_factory_manager::instance() = nullptr;
}

void net_poll(int timeout)
{
	s_poller->poll(timeout);
}

void launch_server(session_manager* m, const char* ip, unsigned short port)
{
	launch_server(s_poller, m, ip, port);
}

void launch_client(session_manager* m, const char* ip, unsigned short port)
{
	launch_client(s_poller, m, ip, port);
}

void launch_server(poller* p, session_manager* m, const char* ip, unsigned short port)
{
	m->set_poller(p);
	m->launch_server(ip, port);
}

void launch_client(poller* p, session_manager* m, const char* ip, unsigned short port)
{
	m->set_poller(p);
	m->launch_client(ip, port);
}
