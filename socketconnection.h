#pragma once

#include "socketbase.h"

class session;

class socket_connection : public socket_base
{
public:
	socket_connection();
	~socket_connection();

	virtual int get_type() const { return socket_type_io; }

	virtual int poll_in();
	virtual int poll_out();
	virtual int poll_close();

	virtual int read(void* buf, unsigned int len);
	virtual int write(const void* buf, unsigned int len);

	session* get_session() { return _session; }
private:
	session* _session;
};
