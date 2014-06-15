#include <iostream>
#include <string.h>
#include "net.h"
#include "protocolheader.h"

session_manager *m = nullptr;

std::map<unsigned int, std::string> usermap;

void on_chat(unsigned int speak, unsigned int towho, const char* content)
{
	std::string speak_name = usermap[speak];
	std::string to_name = "all";
	if (towho)
	{
		if (usermap.find(towho) == usermap.end())
		{
			std::cout << "on_chat: speak=" << speak_name << ", target not found" << std::endl;
			return;
		}
		to_name = usermap[towho];
	}

	std::cout << "on_chat: speak=" << speak_name << ", to=" << to_name << ", content=" << content << std::endl;

	protocol_chat chat;
	chat._who = speak;
	strcpy(chat._content, content);
	towho ? m->send(towho, chat) : m->broadcast(chat);
}

void on_query(unsigned int sid, session_manager* m)
{
	std::cout << "on_query: sid=" << sid << std::endl;

	protocol_userlist ul;
	ul._num = usermap.size();
	int i = 0;
	for (auto a = usermap.begin(); a != usermap.end(); ++a, ++i)
	{
		ul._list[i].sid = a->first;
		strcpy(ul._list[i].name, a->second.c_str());
	}
	m->send(sid, ul);
}

void on_login(unsigned int sid, const char* name)
{
	std::cout << "on_login: sid=" << sid << ", name=" << name << std::endl;
	usermap[sid] = name;

	protocol_echo echo;
	strcpy(echo._content, "welcome to you ");
	strcat(echo._content, name);
	m->send(sid, echo);

	protocol_userlist ul;
	ul._num = usermap.size();
	int i = 0;
	for (auto a = usermap.begin(); a != usermap.end(); ++a, ++i)
	{
		ul._list[i].sid = a->first;
		strcpy(ul._list[i].name, a->second.c_str());
	}
	m->broadcast(ul);
}

void on_logout(unsigned int sid)
{
	const char* name = usermap[sid].c_str();
	std::cout << "on_login: sid=" << sid << ", name=" << name << std::endl;
	
	usermap.erase(sid);

	protocol_echo echo;
	strcpy(echo._content, "goodbye");
	m->send(sid, echo);

	protocol_userlist ul;
	ul._num = usermap.size();
	int i = 0;
	for (auto a = usermap.begin(); a != usermap.end(); ++a, ++i)
	{
		ul._list[i].sid = a->first;
		strcpy(ul._list[i].name, a->second.c_str());
	}
	m->broadcast(ul);
}

int main()
{
	net_init();

	m = new session_manager("MyServer");
	launch_server(m, "127.0.0.1", 7777);

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
