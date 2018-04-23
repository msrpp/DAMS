#include "MessageHandler.h"


CMessageHandler::CMessageHandler()
{
}


CMessageHandler::~CMessageHandler()
{
}

int CMessageHandler::Dispatch(string uri, string command, map<string, string>& headers, char*& body, int& iLength, char* retBody, int nLen)
{
	LOG_INFO << "recv client msg" <<*body;
	Json::Value root;
	Json::Reader reader;
	if (!reader.parse(body, root))
	{
		return -1;
	}
	if (uri.find("/addres") != string::npos)
	{
		
	}

	return 0;
}
