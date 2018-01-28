#ifndef _MESSAGE_HANDLER_H_
#define _MESSAGE_HANDLER_H_

#include "CserverSession.h"

class CMessageHandler :public IServerMsgSink
{
public:
	CMessageHandler();
	~CMessageHandler();
	virtual int Dispatch(string uri, string command, map<string, string>& headers, char*& body, int& iLength, string & strResp);

private:
	void dispatch();
};
#endif