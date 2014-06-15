#include <iostream>
#include "protocolheader.h"
#include "datastream.h"
#include "sessionmanager.h"

void protocol_echo::read(data_stream& ds)
{
	ds.pop(_content, sizeof(_content));
}

void protocol_echo::write(data_stream& ds) const
{
	ds.push(_content, sizeof(_content));
}

void protocol_echo::process(unsigned int sid, session_manager* m)
{
	m->send(sid, this);
}

/*
void protocol_login::read(data_stream& ds)
{
	ds.pop(_name, sizeof(_name));
}

void protocol_login::write(data_stream& ds) const
{
	ds.push(_name, sizeof(_name));
}

void protocol_login::process(unsigned int sid, session_manager* m)
{
}

void protocol_logout::read(data_stream& ds)
{
}

void protocol_logout::write(data_stream& ds) const
{
}

void protocol_logout::process(unsigned int sid, session_manager* m)
{
}

void protocol_chat::read(data_stream& ds)
{
	ds >> _who; 
	ds.pop(_content, sizeof(_content));
}

void protocol_chat::write(data_stream& ds) const
{
	ds << _who;
	ds.push(_content, sizeof(_content));
}

void on_chat(unsigned, const char*);

void protocol_chat::process(unsigned int sid, session_manager* m)
{
	on_chat(_who, _content);
}

void protocol_query::read(data_stream& ds)
{
}

void protocol_query::write(data_stream& ds) const
{
}

void protocol_query::process(unsigned int sid, session_manager* m)
{
}

void protocol_userlist::read(data_stream& ds)
{
	ds >> _num;
	for (auto i = 0; i < _num && i < 64; ++i)
	{
		ds >> _list[i].sid;
		ds.pop(_list[i].name, 64);
	}
}

void protocol_userlist::write(data_stream& ds) const
{
	ds << _num;
	for (auto i = 0; i < _num && i < 64; ++i)
	{
		ds << _list[i].sid;
		ds.push(_list[i].name, 64);
	}
}

void begin_on_userlist(int);
void on_userlist(unsigned int, const char*);
void end_on_userlist(int);

void protocol_userlist::process(unsigned int sid, session_manager* m)
{
	begin_on_userlist(_num);
	for (auto i = 0; i < _num; ++i)
		on_userlist(_list[i].sid, _list[i].name);
	end_on_userlist(_num);
}*/
