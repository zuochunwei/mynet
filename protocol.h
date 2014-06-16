#pragma once

#include <sys/time.h>

class data_stream;

class protocol
{
public:
	protocol(int type) : _type(type) {}
	virtual ~protocol() {}
	virtual void read(data_stream&) = 0;
	virtual void write(data_stream&) const = 0;
	virtual void process(unsigned int sid, class session_manager*) = 0;
	int get_type() const { return _type; }
private:
	int _type;
};
