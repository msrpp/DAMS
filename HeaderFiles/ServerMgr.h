#pragma once
#include "CommonHeader.h"
#include "config.h"
#include "ServerControl.h"
#include "CserverSession.h"
#include "MessageHandler.h"
class CServerMgr :public IServerMsgNotify, public boost::serialization::singleton<CServerMgr>
{
public:
	CServerMgr();
	~CServerMgr();
	virtual int OnStart();
	virtual int OnStop();


public:
//	virtual int OnRecvData(string uri, string command, map<string, string>& headers, char*& body, int& iLength, char* retBody, int nLen);

private:
	CserverSession m_serverSession;

	map<string,pfnRoute> BuildRoutes();

	CMessageHandler m_handler;
	
};

