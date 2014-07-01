#include <string.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <thread>
#include "net.h"
#include "protocolheader.h"

class my_session_manager : public session_manager
{
public:
	my_session_manager(std::string& id) : session_manager(id) {}
	virtual void on_add_session(unsigned int sid) 
	{
		std::cout << "connect to server ok, sid:" << sid << std::endl;
	}
	virtual void on_del_session(unsigned int sid) 
	{
		std::cout << "disconnect from server, sid:" << sid << std::endl;
	}
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cout << "usage: ./client port connectnum" << std::endl;
		return 0;
	}

	int port = atoi(argv[1]);
	int connectnum = atoi(argv[2]);

	net_init();
	
	std::string id("MyClient");
	my_session_manager* m = new my_session_manager(id);
	poller *p = new poller();
	p->create(connectnum);

	for (int i = 0; i < connectnum; ++i)
	{
		launch_client(p, m, "127.0.0.1", port);
		usleep(5000);
		p->poll(0);
		while (m->process_protocol()) 
			;
	}

	std::cout << "all connect done" << std::endl;

	protocol_echo e;
	for (int i = 0; i < connectnum; ++i)
		for (int j = 0; j < 1; ++j)
			m->send(i+1, e);

	while (true)
	{
		p->poll(0);
		while (m->process_protocol())
			;
	}

	p->close();
	delete p;
	delete m;

	net_term();

	return 0;
}
