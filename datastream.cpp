#include <string.h>
#include <arpa/inet.h>
#include "datastream.h"
#include "protocol.h"

data_stream::data_stream(data_buffer* db)
	: _db(db), _dbout(0)
{
}

data_stream::~data_stream()
{
}

unsigned int data_stream::length() const
{
	return _db->length();
}

data_stream& data_stream::pop(void* to, unsigned int len)
{
	if (_db->length() < len)
		throw out_of_data();

	unsigned int l = 0;
	void* data = _db->get_data_buf(l);
	if (l >= len)
	{
		memcpy(to, data, len);
		_db->on_get(len);
		return *this;
	}

	memcpy(to, data, l);
	_db->on_get(l);

	len -= l;
	to = (char*)to + l;
	data = _db->get_data_buf(l);
	memcpy(to, data, len);
	_db->on_get(len);

	return *this;
}

data_stream& data_stream::push(const void* from, unsigned int len)
{
	if (_db->available_max() < len)
		throw out_of_memory();
	_db->put(from, len);
	return *this;
}

data_stream& data_stream::operator<< (int data)
{
	data = htonl(data);
	push(&data, sizeof(data));
	return *this;
}

data_stream& data_stream::operator<< (unsigned int data)
{
	data = htonl(data);
	push(&data, sizeof(data));
	return *this;
}

data_stream& data_stream::operator<< (short data)
{
	data = htons(data);
	push(&data, sizeof(data));
	return *this;
}

data_stream& data_stream::operator<< (unsigned short data)
{
	data = htons(data);
	push(&data, sizeof(data));
	return *this;
}

data_stream& data_stream::operator<< (long data)
{
	union 
	{
		long l;
		unsigned int i[2];
	} i, o;

	i.l = data;
	o.i[0] = htonl(i.i[1]);
	o.i[1] = htonl(i.i[0]);

	push(&o.l, sizeof(o.l));
	return *this;
}

data_stream& data_stream::operator<< (unsigned long data)
{
	union 
	{
		unsigned long l;
		unsigned int i[2];
	} i, o;

	i.l = data;
	o.i[0] = htonl(i.i[1]);
	o.i[1] = htonl(i.i[0]);

	push(&o.l, sizeof(o.l));
	return *this;
}

data_stream& data_stream::operator<< (long long data)
{
	static_assert(sizeof(long long) == sizeof(long), "data_stream data length error");
	union 
	{
		long l;
		unsigned int i[2];
	} i, o;

	i.l = data;
	o.i[0] = htonl(i.i[1]);
	o.i[1] = htonl(i.i[0]);

	push(&o.l, sizeof(o.l));
	return *this;
}

data_stream& data_stream::operator<< (unsigned long long data)
{
	static_assert(sizeof(unsigned long long) == sizeof(unsigned long), "data_stream data length error");
	union 
	{
		unsigned long l;
		unsigned int i[2];
	} i, o;

	i.l = data;
	o.i[0] = htonl(i.i[1]);
	o.i[1] = htonl(i.i[0]);

	push(&o.l, sizeof(o.l));
	return *this;
}

data_stream& data_stream::operator<< (const protocol& data)
{
	data.write(*this);
	return *this;
}

data_stream& data_stream::operator<< (const data_stream& data)
{
	unsigned int len = data._db->length();
	operator<< (len);

	void* buf = data._db->get_data_buf(len);
	push(buf, len);
	data._db->on_get(len);

	if (data._db->length())
	{
		buf = data._db->get_data_buf(len);
		push(buf, len);
		data._db->on_get(len);
	}

	return *this;
}

data_stream& data_stream::operator>> (int& data)
{
	pop(&data, sizeof(data));
	data = ntohl(data);
	return *this;
}

data_stream& data_stream::operator>> (unsigned int& data)
{
	pop(&data, sizeof(data));
	data = ntohl(data);
	return *this;
}

data_stream& data_stream::operator>> (short& data)
{
	pop(&data, sizeof(data));
	data = ntohs(data);
	return *this;
}

data_stream& data_stream::operator>> (unsigned short& data)
{
	pop(&data, sizeof(data));
	data = ntohs(data);
	return *this;
}

data_stream& data_stream::operator>> (long& data)
{
	union 
	{
		long l;
		unsigned int i[2];
	} i, o;

	pop(&i.l, sizeof(i.l));
	o.i[0] = ntohl(i.i[1]);
	o.i[1] = ntohl(i.i[0]);

	data = o.l;
	return *this;
}

data_stream& data_stream::operator>> (unsigned long& data)
{
	union 
	{
		unsigned long l;
		unsigned int i[2];
	} i, o;

	pop(&i.l, sizeof(i.l));
	o.i[0] = ntohl(i.i[1]);
	o.i[1] = ntohl(i.i[0]);

	data = o.l;
	return *this;
}

data_stream& data_stream::operator>> (long long& data)
{
	static_assert(sizeof(long long) == sizeof(long), "data_stream data length error");
	union 
	{
		long l;
		unsigned int i[2];
	} i, o;

	pop(&i.l, sizeof(i.l));
	o.i[0] = ntohl(i.i[1]);
	o.i[1] = ntohl(i.i[0]);

	data = o.l;
	return *this;
}

data_stream& data_stream::operator>> (unsigned long long& data)
{
	static_assert(sizeof(unsigned long long) == sizeof(unsigned long), "data_stream data length error");
	union 
	{
		unsigned long l;
		unsigned int i[2];
	} i, o;

	pop(&i.l, sizeof(i.l));
	o.i[0] = ntohl(i.i[1]);
	o.i[1] = ntohl(i.i[0]);

	data = o.l;
	return *this;
}

data_stream& data_stream::operator>> (protocol& p)
{
	p.read(*this);
	return *this;
}

data_stream& data_stream::operator>> (const begin& data)
{
	_dbout = _db->get_out();
	return *this;
}

data_stream& data_stream::operator>> (const commit& data)
{
	_dbout = 0;
	return *this;
}

data_stream& data_stream::operator>> (const rollback& data)
{
	_db->set_out(_dbout);
	return *this;
}
