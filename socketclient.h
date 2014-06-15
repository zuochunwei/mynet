#pragma once

#include "socketbase.h"

class session_manager;

class socket_client : public socket_base
{
public:
	socket_client(session_manager*);
	~socket_client();

	virtual int get_type() const { return socket_type_client; }

	virtual int poll_in();
	virtual int poll_out();

	bool connect();
private:
	session_manager *_manager;
};
