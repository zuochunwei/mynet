#include <string.h>
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
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (unsigned int data)
{
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (short data)
{
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (unsigned short data)
{
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (long data)
{
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (unsigned long data)
{
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (long long data)
{
	return push(&data, sizeof(data));
}

data_stream& data_stream::operator<< (unsigned long long data)
{
	return push(&data, sizeof(data));
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
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (unsigned int& data)
{
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (short& data)
{
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (unsigned short& data)
{
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (long& data)
{
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (unsigned long& data)
{
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (long long& data)
{
	return pop(&data, sizeof(data));
}

data_stream& data_stream::operator>> (unsigned long long& data)
{
	return pop(&data, sizeof(data));
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
