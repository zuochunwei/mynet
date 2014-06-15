#include "protocolfactory.h"
#include "protocolheader.h"

static protocol_factory_manager* ptr = nullptr;

protocol_factory::protocol_factory()
{
}

protocol_factory::~protocol_factory()
{
}

protocol_factory_manager*& protocol_factory_manager::instance()
{
	return ptr;
}

protocol_factory_manager::protocol_factory_manager()
{
	_factorymap[PROTOCOL_TYPE_ECHO] = new protocol_factory_template<protocol_echo>();
	/*_factorymap[PROTOCOL_TYPE_LOGIN] = new protocol_factory_template<protocol_login>();
	_factorymap[PROTOCOL_TYPE_LOGOUT] = new protocol_factory_template<protocol_logout>();
	_factorymap[PROTOCOL_TYPE_CHAT] = new protocol_factory_template<protocol_chat>();
	_factorymap[PROTOCOL_TYPE_QUERY] = new protocol_factory_template<protocol_query>();
	_factorymap[PROTOCOL_TYPE_USERLIST] = new protocol_factory_template<protocol_userlist>();*/
}

protocol_factory_manager::~protocol_factory_manager()
{
	for (auto a = _factorymap.begin(); a != _factorymap.end(); ++a)
		delete a->second;
}

protocol* protocol_factory_manager::create(int type)
{
	auto a = _factorymap.find(type);
	if (a == _factorymap.end())
		return nullptr;
	return a->second->create();
}
