#include "ServerMgr.h"
#include "config.h"
#include "CserverSession.h"
#include "DeviceMgr.h"

CServerMgr::CServerMgr()
{
}


CServerMgr::~CServerMgr()
{
}

void OnRecvAddDevice(map<string, string>& headers, char* body, int iLength, void* pUserData)
{
	return ;
}
int CServerMgr::OnStart()
{

	CServerSessionSingleton::get_mutable_instance().SetRoutes(BuildRoutes());
	
	CServerSessionSingleton::get_mutable_instance().Open(CConfig::get_mutable_instance().getListenPort(), "0.0.0.0", &m_handler);

	CDeviceMgr::get_mutable_instance().Init();
	return 0;
}

int CServerMgr::OnStop()
{
	return 0;
}

// int CServerMgr::OnRecvData(string uri,string command,map<string, string>& headers, const char* body, int& iLength ,char* retBody,int nLen)
// {
// 	return 0;
// }

std::map<string, pfnRoute> CServerMgr::BuildRoutes()
{
	map<string, pfnRoute> routes;
	routes["/dump"] = OnRecvAddDevice;
	return routes;
}
