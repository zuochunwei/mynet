#pragma once

#include <string.h>
#include "sessionmanager.h"
#include "protocoldemo.h"

class test_session_manager : public session_manager
{
public:
	test_session_manager() : session_manager("testsessionmanager") {}
	virtual void on_add_session(int id)
	{
		protocol_echo echo;
		strcpy(echo._content, "hello\n");
		send(id, echo);
	}
};
