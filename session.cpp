#include <netinet/in.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <atomic>
#include <string.h>
#include "session.h"
#include "sessionmanager.h"
#include "socketbase.h"
#include "protocol.h"
#include "datastream.h"
#include "protocolfactory.h"

static std::atomic_uint atomic_sid(0);

static unsigned int alloc_session_id()
{
	return ++atomic_sid;
}

session::session(unsigned isize, unsigned isizemax, unsigned osize, unsigned osizemax)
	: _id(alloc_session_id()), _socket(nullptr), 
	_ibuf(isize, isizemax), _obuf(osize, osizemax)
{
}

session::session()
	: session(64*1024, 512*1024, 64*1024, 512*1024)
{
}

session::~session()
{
}

bool session::add_to_manager(session_manager* manager)
{
	_manager = manager;
	return _manager->add_session(this);
}

bool session::del_from_manager()
{
	bool b = _manager->del_session(this);
	_manager = nullptr;
	return b;
}

int session::poll_in()
{
	unsigned int len = 0; 
	void* buf = _ibuf.get_free_buf(len);
	if (len == 0)
	{
		_socket->set_close(error_read_failed);
		std::cout << "poll_in() no free space" << std::endl;
		return -1;
	}

	int n = _socket->read(buf, len);
	if (n == 0)
	{
		_socket->set_close(error_peer_closed);
		return 0;
	}

	if (n == -1)
	{
		_socket->set_close(error_read_failed);
		return -1;
	}

	int sum = 0;
	sum += n;
	_ibuf.on_put(n);

	if ((unsigned int)n == len)
	{
		void* buf = _ibuf.get_free_buf(len);
		if (len > 0)
		{
			n = _socket->read(buf, len);
			if (n < 0)
			{
				if (errno != EAGAIN)
				{
					_socket->set_close(error_read_failed);
					return -1;
				}
				n = 0;
			}

			sum += n;
			_ibuf.on_put(n);
		}
	}

	on_poll_in();

	return sum;
}

int session::poll_out()
{
	unsigned int len = 0; 
	int n = 0, sum = 0;

	std::lock_guard<std::mutex> lock(_mutex);

	void* buf = _obuf.get_data_buf(len);
	if (len == 0)
	{
		_socket->forbid_write();
		return 0;
	}

	n = _socket->write(buf, len);
	if (n <= 0)
	{
		_socket->set_close(error_write_failed);
		return -1;
	}

	sum += n;
	_obuf.on_get(n);

	if ((unsigned int)n == len)
	{
		void* buf = _obuf.get_data_buf(len);
		if (len == 0)
			_socket->forbid_write();
		else
		{
			n = _socket->write(buf, len);
			if (n < 0)
			{
				if (errno != EAGAIN)
				{
					_socket->set_close(error_write_failed);
					return -1;
				}
				n = 0;
			}

			sum += n;
			_obuf.on_get(n);
		}
	}

	on_poll_out();

	return sum;
}

void session::on_poll_in()
{
	while (protocol* p = decode())
	{
		_manager->add_protocol(_id, p);
		_socket->permit_read();
	}

	if (_ibuf.available_max() == 0)
	{
		_socket->forbid_read();
	}
}

void session::on_poll_out()
{
}

bool session::send(const protocol* p)
{
	try
	{
		std::lock_guard<std::mutex> lock(_mutex);
		data_stream ds(&_obuf);

		static thread_local data_buffer db(512*1024);
		db.clear();
		data_stream tmp(&db);

		tmp << p->get_type() << *p;

		if (_obuf.available_max() < sizeof(tmp.length()) + tmp.length())
			return false;

		ds << tmp;
		_socket->permit_write();
		return true;
	}
	catch (data_stream::out_of_memory& e)
	{
		std::cout << "session::send() out_of_memory" << std::endl;
		return false;
	}
}

protocol* session::decode()
{
	data_stream ds(&_ibuf);
	unsigned int len = 0;
	int type = 0;
	protocol *p = nullptr;
	try
	{
		ds >> data_stream::begin() >> len >> type;
		p = protocol_factory_manager::instance()->create(type);
		if (p == nullptr)
		{
			_socket->set_close(error_invalid_protocol);
			return nullptr;
		}
		ds >> *p >> data_stream::commit();
		return p;
	}
	catch (data_stream::out_of_data& e)
	{
		delete p;
		ds >> data_stream::rollback();
	}

	return nullptr;
}
