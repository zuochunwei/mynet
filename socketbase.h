#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class poller;
class session;

enum
{
	socket_type_server,
	socket_type_client,
	socket_type_io,
};

enum
{
	error_null,
	error_connect_failed,
	error_read_failed,
	error_write_failed,
	error_peer_closed,
	error_active_close,
	error_invalid_protocol,
};

class socket_base
{
public:
	socket_base();
	socket_base(const socket_base&) = delete;
	socket_base& operator= (const socket_base&) = delete;
	virtual ~socket_base();

	virtual int get_type() const = 0;

	virtual int poll_in() = 0;
	virtual int poll_out() = 0;
	virtual int poll_close();

	virtual int read(void* buf, unsigned int len) { return 0; }
	virtual int write(const void* buf, unsigned int len) { return 0; }

	void forbid_write();
	void forbid_read();
	void permit_write();
	void permit_read();

	bool create_socket();
	bool close_socket();
	
	void set_close(int error);
	void set_address(const sockaddr_in& addr) { _addr = addr; }
	void set_fd(int fd) { _fd = fd; }
	void set_event(unsigned int event) { _event = event; }
	void set_poller(poller* p) { _poller = p; }

	void set_non_block();
	void set_reuse_addr();
	void set_keep_alive();
	void set_send_buf(int size);
	void set_recv_buf(int size);
	void set_opt_int(int optname, int value);

	int get_fd() const { return _fd; }
	int get_error() const { return _error; }
	int get_opt_int(int optname) const;
	unsigned int get_id() const { return _id; }
	unsigned int get_event() const { return _event; }

	static sockaddr_in make_address(const char* ip, unsigned short port);
protected:
	unsigned int _id;
	int _fd;
	int _error;
	unsigned int _event;
	poller *_poller;
	sockaddr_in _addr;
	bool _close;
};

