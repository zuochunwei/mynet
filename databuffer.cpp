#include <assert.h>
#include <algorithm>
#include <string.h>
#include "databuffer.h"

static void roundup_power_2(unsigned int & i)
{
	unsigned int n = 2;
	while (n < i)
		n *= 2;
	i = n;
}

data_buffer::data_buffer(unsigned int size)
	: data_buffer(size, size)
{}

data_buffer::data_buffer(unsigned int size, unsigned int size_max)
	: _buf(nullptr), _size(size), _size_max(size_max), _in(0), _out(0)
{
	assert(size <= size_max);
	if (size & (size - 1))
		roundup_power_2(size);
	if (size_max & (size_max - 1))
		roundup_power_2(size_max);
	_buf = new unsigned char[size];
}

data_buffer::data_buffer(const data_buffer& rhs)
	: _buf(nullptr), _size(rhs._size), _size_max(rhs._size_max), _in(rhs._in), _out(rhs._out)
{
	_buf = new unsigned char[_size];
	memcpy(_buf, rhs._buf, _size);
}

data_buffer& data_buffer::operator= (const data_buffer& rhs)
{
	if (_size < rhs.length())
	{
		delete [] _buf;
		_buf = new unsigned char[rhs._size];
		_size = rhs._size;
		_size_max = rhs._size_max;
	}
	
	data_buffer copy(rhs);	//for rhs is const ref

	unsigned int len1 = 0, len2 = 0;
	void *buf = copy.get_data_buf(len1);
	memcpy(_buf, buf, len1);
	copy.on_get(len1);

	buf = copy.get_data_buf(len2);
	memcpy(_buf + len1, buf, len2);
	copy.on_get(len2);

	_out = 0;
	_in = len1 + len2;

	return *this;
}

data_buffer::~data_buffer()
{
	delete [] _buf;
}

unsigned int data_buffer::length() const
{
	return _in - _out;
}

unsigned int data_buffer::available() const
{
	return _size - length();
}

unsigned int data_buffer::available_max() const
{
	return _size_max - length();
}

unsigned int data_buffer::put(const void* data, unsigned int len)
{
	reserve(len + length());
	len = std::min(len, _size - length());
	unsigned int n = std::min(len, _size - (_in & (_size - 1)));
	memcpy(_buf + (_in & (_size - 1)), data, n);
	memcpy(_buf, (const char*)data + n, len - n);
	_in += len;
	return len;
}

unsigned int data_buffer::get(void* dst, unsigned int len)
{
	len = std::min(len, length());
	unsigned int n = std::min(len, _size - (_out & (_size - 1)));
	memcpy(dst, _buf + (_out & (_size - 1)), n);
	memcpy((char*)dst + n, _buf, len - n);
	_out += len;
	if (_out == _in) _out = _in = 0;
	return len;
}

bool data_buffer::reserve(unsigned int size)
{
	if (_size < size)
	{
		unsigned int s = _size;
		while (s < size && s < _size_max)
			s *= 2;
		unsigned char* buf = new unsigned char[s];
		unsigned int l = length();
		get(buf, l);
		_in = l;
		_out = 0;
		_size = s;
		delete [] _buf;
		_buf = buf;

		return s >= size;
	}
	return true;
}

void* data_buffer::get_free_buf(unsigned int& len)
{
	unsigned int mask = (_size - 1);
	if (_in - _out == _size)
		len = 0;
	else if ((_in & mask) < (_out & mask))
		len = (_out & mask) - (_in & mask);
	else
		len = _size - (_in & mask);
	return &_buf[_in & mask];
}

void* data_buffer::get_data_buf(unsigned int& len)
{
	unsigned int mask = (_size - 1);
	if ((_in & mask) >= (_out & mask))
		len = _in -_out;
	else
		len = _size - (_out & mask);
	return &_buf[_out & mask];
}

