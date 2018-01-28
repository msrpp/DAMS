#pragma once
/*
对libevent http库的简单封装。
*/
#if defined(__APPLE__) && defined(__clang__)
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#define snprintf _snprintf
#define strcasecmp _stricmp 
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <event2/bufferevent_ssl.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/http.h>

 #include "CommonHeader.h"
// 
// #include <boost/thread/thread.hpp>
// #include <boost/bind.hpp>
#define  BODYLEN 2048

class CClientSession
{
public:
	CClientSession();
	~CClientSession();
	
public:
	void setSessionParams();
	int http_client_short_link(const char* url, const char* body, int iBodyLen, string& retBody);
public:
	void OnResult(evhttp_request* req);


private:
	int m_retries;
	int m_timeout;
	struct bufferevent *m_bev;
	char m_retBody[BODYLEN];

};

//typedef public boost::serialization::singleton<CserverSession> CServerSessionSingleton;