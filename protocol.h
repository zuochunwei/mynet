#pragma once

#include <sys/time.h>

inline unsigned long long current_time()
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	unsigned long long l = tv.tv_sec * 1000000 + tv.tv_usec;
	return l;
}

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
