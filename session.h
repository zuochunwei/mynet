#pragma once

#include <mutex>
#include "databuffer.h"
#include "protocol.h"

class socket_base;
class session_manager;

class session
{
public:
	session();
	session(unsigned isize, unsigned isizemax, unsigned osize, unsigned osizemax);
	session(const session&) = delete;
	session& operator= (const session&) = delete;
	~session();

	bool add_to_manager(session_manager*);
	bool del_from_manager();

	int poll_in();
	int poll_out();

	void on_poll_in();
	void on_poll_out();

	bool send(const protocol*);

	void set_socket(socket_base* socket) { _socket = socket; }
	void set_manager(session_manager *manager) { _manager = manager; }

	socket_base *get_socket() { return _socket; }
	session_manager *get_manager() { return _manager; }
	int get_id() const { return _id; }
private:
	protocol* decode();

	unsigned int _id;
	socket_base *_socket;
	session_manager *_manager;
	std::mutex _mutex;
	data_buffer _ibuf;
	data_buffer _obuf;
};
