#pragma once

#include "protocol.h"
#include <map>

class protocol_factory
{
public:
	protocol_factory();
	virtual ~protocol_factory();
	virtual protocol* create() = 0;
};

template <typename PROTOCOL>
class protocol_factory_template : public protocol_factory
{
public:
	virtual protocol* create() { return new PROTOCOL; }
};

class protocol_factory_manager
{
public:
	protocol_factory_manager();
	static protocol_factory_manager*& instance();
	~protocol_factory_manager();
	protocol* create(int type);
private:
	std::map<int, protocol_factory*> _factorymap;
};
