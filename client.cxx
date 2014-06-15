#include <string.h>
#include <sstream>
#include <iostream>
#include <thread>
#include "net.h"
#include "protocolheader.h"

class my_session_manager : public session_manager
{
public:
	my_session_manager(std::string& id) : session_manager(id), sid(0) {}
	virtual void on_add_session(unsigned int sid) 
	{
		this->sid = sid;
		std::cout << "connect to server ok, sid:" << sid << ", please login..." << std::endl;
	}
	virtual void on_del_session(unsigned int sid) 
	{
		this->sid = 0;
		std::cout << "disconnect from server" << std::endl;
	}

	bool send_protocol(const protocol& p)
	{
		if (sid) 
			return send(sid, p);
		return false;
	}
private:
	unsigned int sid;
} *m = nullptr;

std::map<unsigned int, std::string> usermap;
std::string myname;

void on_chat(unsigned int sid, const char* content)
{
	std::cout << usermap[sid] << " speak: " << content << std::endl;
}

void begin_on_userlist(int num)
{
	std::cout << "get usermap begin ..., num=" << num << std::endl;
	usermap.clear();
}

void on_userlist(unsigned int sid, const char* name)
{
	if (myname == name)
		std::cout << "sid:" << sid << ", name:" << name << ". self !!!" << std::endl;
	else
		std::cout << "sid:" << sid << ", name:" << name << std::endl;
	usermap[sid] = std::string(name);
}

void end_on_userlist(int)
{
	std::cout << "get usermap end" << std::endl;
}

void console_thread()
{
	while (true)
	{
		char buff[512] = {};
		memset(buff, 0, sizeof(buff));
		std::cin.getline(buff, 512);

		std::string s(buff);
		std::istringstream ss(s);

		std::string cmd;
		ss >> cmd;

		if (cmd == "login")
		{
			protocol_login login;
			ss >> login._name;
			myname = login._name;
			m->send_protocol(login);
		}
		else if (cmd == "logout")
		{
			protocol_logout logout;
			m->send_protocol(logout);
		}
		else if (cmd == "chat")
		{
			protocol_chat chat;
			ss >> chat._who >> chat._content;
			m->send_protocol(chat);
		}
		else if (cmd == "query")
		{
			protocol_query query;
			m->send_protocol(query);
		}
		else if (cmd == "echo")
		{
			protocol_echo echo;
			ss >> echo._content;
			m->send_protocol(echo);
		}
		else
		{
			std::cout << "invalid cmd" << std::endl;
		}
	}
}

int main()
{
	std::thread t(console_thread);

	net_init();
	
	std::string id("MyClient");
	m = new my_session_manager(id);

	launch_client(m, "127.0.0.1", 7777);

	while (true)
	{
		net_poll(25);
		m->process_protocol();
	}

	delete m;
	m = nullptr;

	net_term();

	return 0;
}
