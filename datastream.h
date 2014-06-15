#pragma once

#include "databuffer.h"

class data_stream
{
public:
	struct out_of_data {};
	struct out_of_memory {};
	struct begin {};
	struct commit {};
	struct rollback {};

	data_stream(data_buffer* db);
	data_stream(const data_stream&) = delete;
	data_stream& operator= (const data_stream&) = delete;
	~data_stream();

	unsigned int length() const;

	data_stream& pop(void* to, unsigned int);
	data_stream& push(const void* from, unsigned int);

	data_stream& operator<< (int);
	data_stream& operator<< (unsigned int);
	data_stream& operator<< (short);
	data_stream& operator<< (unsigned short);
	data_stream& operator<< (long);
	data_stream& operator<< (unsigned long);
	data_stream& operator<< (long long);
	data_stream& operator<< (unsigned long long);
	data_stream& operator<< (const class protocol&);
	data_stream& operator<< (const data_stream&);

	data_stream& operator>> (int&);
	data_stream& operator>> (unsigned int&);
	data_stream& operator>> (short&);
	data_stream& operator>> (unsigned short&);
	data_stream& operator>> (long&);
	data_stream& operator>> (unsigned long&);
	data_stream& operator>> (long long&);
	data_stream& operator>> (unsigned long long&);
	data_stream& operator>> (class protocol&);
	data_stream& operator>> (const begin&);
	data_stream& operator>> (const commit&);
	data_stream& operator>> (const rollback&);
private:
	data_buffer* _db;
	unsigned int _dbout;
};
