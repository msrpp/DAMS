
#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include "CserverSession.h"

#include "boost/thread/lock_guard.hpp"


void bindHttp2Socket(void* params)
{
	CserverSession* pSession = reinterpret_cast<CserverSession*>(params);
	pSession->_BindHttpEv2Fd();

}



static void
send_document_cb(struct evhttp_request *req, void *params)
{
	CserverSession* pSession = reinterpret_cast<CserverSession*>(params);
	if (pSession && pSession->m_pSink)
	{

		const char *cmdtype;
		switch (evhttp_request_get_command(req)) {
		case EVHTTP_REQ_GET: cmdtype = "GET"; break;
		case EVHTTP_REQ_POST: cmdtype = "POST"; break;
		case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
		case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
		case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
		case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
		case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
		case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
		case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
		default: cmdtype = "unknown"; break;
		}

		map<string, string> headers;
		
		const char *uri = evhttp_request_get_uri(req);
		struct evbuffer *buf = evhttp_request_get_input_buffer(req);
		int dataLen = evbuffer_get_length(buf);
		char* pData = new char[dataLen];
// 		int iRecvDatalen = 1024;
// 		char recvData[1024] = { 0 };
// 		memset(pData, 0, dataLen);
		evbuffer_remove(buf, pData, dataLen);
		string strResponse;
		pSession->m_pSink->Dispatch(uri, cmdtype, headers, pData, dataLen, strResponse);

		struct evbuffer * evb = evbuffer_new();
		evbuffer_add_printf(evb, strResponse.c_str());
		evhttp_send_reply(req, 200, "OK", evb);


//		evbuffer_free(evb);
	}
	
}

CserverSession::CserverSession()
:m_pSink(NULL)
, m_fd(INVALID_SOCKET)
{

}


CserverSession::~CserverSession()
{

}

int CserverSession::Open(int port, string strIp, IServerMsgSink* pSink)
{
	m_pSink = pSink;
	struct event_base *base = NULL;
	struct evhttp *http = NULL;
	struct evhttp_bound_socket *handle = NULL;

// 	{
// 		//LOG_ERROR << "bind port" << port << "failed";
// 		return -1;
// 	}
	m_fd = bind_by_port(port);
	if (m_fd == INVALID_SOCKET)
	{
		//LOG_ERROR<<"bind_by_port failed";
		return -1;
	}
	int retVal = listen(m_fd, 128);
	retVal = GetLastError();
	for (int num = 0; num < 10; num++)
	{
		m_group.create_thread(boost::bind(&bindHttp2Socket, this));
	}
		
	return 0;
}

void CserverSession::Close()
{

	m_group.join_all();
}

void CserverSession::SetRoutes(map<string, pfnRoute> routes)
{
	m_routes = routes;
}

int CserverSession::_BindHttpEv2Fd()
{
/*	boost::lock_guard<boost::mutex> lock(m_mutex);*/

	struct event_base *base = event_base_new();
	if (!base) {
		fprintf(stderr, "Couldn't create an event_base: exiting\n");
		return 1;
	}

	/* Create a new evhttp object to handle requests. */
	struct evhttp *http = evhttp_new(base);
	if (!http) {
		fprintf(stderr, "couldn't create evhttp. Exiting.\n");
		return 1;
	}

	RouteIterator iter;
	for (iter = m_routes.begin(); iter != m_routes.end(); iter++)
	{

/*		evhttp_set_cb(http, iter->first.c_str(), iter->second, NULL);*/

	}

	evhttp_set_gencb(http, send_document_cb, this);

	struct evhttp_bound_socket * handle = NULL;
	handle = evhttp_accept_socket_with_handle(http, m_fd);
/*	handle = evhttp_bind_socket_with_handle(http, "127.0.0.1", port);*/
	if (NULL ==handle) {
		//LOG_ERROR << "evhttp_accept_socket failed";
		return 1;
	}
	event_base_dispatch(base);
	return 0;
}
