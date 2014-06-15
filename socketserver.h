#pragma once

#include "socketbase.h"

class session_manager;

class socket_server : public socket_base
{
public:
	socket_server(session_manager*);

	virtual int get_type() const { return socket_type_server; }

	virtual int poll_in();
	virtual int poll_out();

	bool bind();
	bool listen(int backlog);
private:
	int accept(sockaddr_in*);

	session_manager *_manager;
};
