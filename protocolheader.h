#pragma once

#include "protocol.h"
#include "cachedobject.h"

enum
{
	//all protocol type define here
	PROTOCOL_TYPE_NULL,
	PROTOCOL_TYPE_ECHO,
	PROTOCOL_TYPE_LOGIN,
	PROTOCOL_TYPE_LOGOUT,
	PROTOCOL_TYPE_CHAT,
	PROTOCOL_TYPE_QUERY,
	PROTOCOL_TYPE_USERLIST,
};

class protocol_echo : public protocol, public cached_object<protocol_echo>
{
public:
	protocol_echo() : protocol(PROTOCOL_TYPE_ECHO), _content() {}
	virtual void read(data_stream&);
	virtual void write(data_stream&) const;
	virtual void process(unsigned int sid, session_manager* m);

	char _content[16];
};

/*class protocol_login : public protocol
{
public:
	protocol_login() : protocol(PROTOCOL_TYPE_LOGIN), _name() {}
	virtual void read(data_stream&);
	virtual void write(data_stream&) const;
	virtual void process(unsigned int sid, session_manager* m);

	char _name[64];
};

class protocol_logout : public protocol
{
public:
	protocol_logout() : protocol(PROTOCOL_TYPE_LOGOUT) {}
	virtual void read(data_stream&);
	virtual void write(data_stream&) const;
	virtual void process(unsigned int sid, session_manager* m);
};

class protocol_chat : public protocol
{
public:
	protocol_chat() : protocol(PROTOCOL_TYPE_CHAT), _who(0), _content() {}
	virtual void read(data_stream&);
	virtual void write(data_stream&) const;
	virtual void process(unsigned int sid, session_manager* m);

	unsigned int _who;  //who = 0, broadcast; else who is session id
	char _content[64];
};

class protocol_query : public protocol
{
public:
	protocol_query() : protocol(PROTOCOL_TYPE_QUERY) {}
	virtual void read(data_stream&);
	virtual void write(data_stream&) const;
	virtual void process(unsigned int sid, session_manager* m);
};

class protocol_userlist : public protocol
{
public:
	protocol_userlist() : protocol(PROTOCOL_TYPE_USERLIST), _num(0) {}
	virtual void read(data_stream&);
	virtual void write(data_stream&) const;
	virtual void process(unsigned int sid, session_manager* m);

	int _num;
	struct
	{
		unsigned int sid;
		char name[64];
	} _list[64];
};*/
