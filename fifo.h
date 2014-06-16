#pragma once

#include <assert.h>

template <typename T>
class fifo
{
public:
	fifo(unsigned int size);
	fifo(unsigned int size, unsigned int size_max);
	fifo(const fifo& rhs);
	fifo& operator=(const fifo& rhs);
	~fifo();

	unsigned int length() const { return _in - _out; }
	void push(const T&);
	T& pop();
	bool full() const;
	bool empty() const;
private:
	void reserve(unsigned int size);

	T *_elements;
	unsigned int _size, _size_max;
	unsigned int _in, _out;
};

static void roundup_power_2(unsigned int& size)
{
	unsigned int n = 2;
	while (n < size) n *= 2;
	size = n;
}

template <typename T>
fifo<T>::fifo(unsigned int size)
	: fifo(size, size)
{
}

template <typename T>
fifo<T>::fifo(unsigned int size, unsigned int size_max)
	: _elements(nullptr), _size(size), _size_max(size_max), _in(0), _out(0)
{
	assert(size <= size_max);
	if (size & (size - 1))
		roundup_power_2(_size);
	if (size_max & (size_max - 1))
		roundup_power_2(_size_max);
	_elements = new T[_size];
}

template <typename T>
fifo<T>::fifo(const fifo& rhs)
	: _size(rhs._size), _size_max(rhs._size_max), _in(rhs._in), _out(rhs._out)
{
	_elements = new T[_size];
	for (int i = 0; i < _size; ++i)
		_elements[i] = rhs._elements[i];
}

template <typename T>
fifo<T>& fifo<T>::operator= (const fifo& rhs)
{
	unsigned int len = rhs.length();
	if (_size_max < len)
		assert(false);

	if (_size < len)
	{
		delete [] _elements;
		_size = rhs._size;
		_elements = new T[_size];
	}

	_in = _out = 0;

	unsigned int out = rhs._out % rhs._size;
	for (unsigned int i = out; i < out + len; ++i)
		_elements[_in++] = rhs._elements[i % rhs._size];

	return *this;
}

template <typename T>
fifo<T>::~fifo()
{
	delete [] _elements;
}

template <typename T>
void fifo<T>::push(const T& t)
{
	reserve(length() + 1);
	_elements[_in++ % _size] = t;
}

template <typename T>
T& fifo<T>::pop()
{
	T& t = _elements[_out++ % _size];
	if (_out == _in) _in = _out = 0;
	return t;
}

template <typename T>
bool fifo<T>::full() const
{
	return length() >= _size_max;
}

template <typename T>
bool fifo<T>::empty() const
{
	return length() == 0;
}

template <typename T>
void fifo<T>::reserve(unsigned int size) 
{
	if (_size < size)
	{
		assert(size <= _size_max);
		unsigned int sz = _size;
		while (sz < size) sz *= 2;

		/*fifo<T> copy(*this);
		delete [] _elements;
		_elements = new T[_size = sz];
		*this = copy;*/

		T* elements = new T[sz];
		unsigned int out = _out % _size;
		unsigned int len = length();
		_in = _out = 0;
		for (unsigned int i = out; i < out + len; ++i)
			elements[_in++] = _elements[i % _size];

		delete [] _elements;
		_elements = elements;
		_size = sz;
	}
}

