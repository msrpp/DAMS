#pragma once


/*
对libevent http库的简单封装。
*/
#include "CommonHeader.h"

#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

typedef void (*pfnRoute)(map<string,string>& headers,char* body, int iLength, void* pUserData);
class IServerMsgSink
{
public:
	virtual int Dispatch(string uri, string command, map<string, string>& headers, char*& body, int& iLength, string & strResponse) = 0;
};
class CserverSession
{
public:
	CserverSession();
	~CserverSession();

	int Open(int port, string strIp, IServerMsgSink* pSink);
	void Close();
	void SetRoutes(map<string, pfnRoute> routes);
public:
	int _BindHttpEv2Fd();
	IServerMsgSink* m_pSink;
private:
	SOCKET m_fd;
	
	typedef map<string, pfnRoute>::iterator RouteIterator;
	map<string, pfnRoute> m_routes;
	thread_group m_group;

	boost::mutex m_mutex;
};

typedef public boost::serialization::singleton<CserverSession> CServerSessionSingleton;