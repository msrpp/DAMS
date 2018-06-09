#include "ServerControl.h"
#include "CommonHeader.h"
#include "ServerMgr.h"
#include "Crash.h"
int main(int argsc,char* argsv[])
{
	SYSTEMTIME time;
	GetLocalTime(&time);
	//if (time.wYear>2018)
	//{
	//	return -1;
	//}
#ifdef _WIN32
	WSADATA WSAData;
	WSAStartup(0x101, &WSAData);
#endif

	MyLog::Init();
	
	if (argsc < 2)
	{
		//LOG_ERROR << "argsc input illegal";
		return -1;
	}
	SetCurrentWorkDir();

// 	void* 	m_hSvcStopEvent = CreateEvent(
// 		NULL,    // default security attributes  
// 		TRUE,    // manual reset event  
// 		FALSE,   // not signaled  
// 		NULL);
// 	SetEvent(m_hSvcStopEvent);
// 	WaitForSingleObject(m_hSvcStopEvent, INFINITE);
	CConfig::get_mutable_instance().LoadConfig();
	CServerControl* pController = &CServerControl::get_mutable_instance();
	string serverName = CConfig::get_mutable_instance().GetServerName();
	pController->Init(serverName.c_str());
	if (strcmp(argsv[1],"install") == 0)
	{
		pController->Install();
		pController->startService();
		
		return 0;
		
	}
	else if (strcmp(argsv[1], "uninstall") == 0)
	{
		return pController->Uninstall();
	}
	else if (strcmp(argsv[1], "service") == 0)
	{
		LOG_INFO << "service start to run now";
		if (0 != pController->RunServer(&CServerMgr::get_mutable_instance()))
		{
			LOG_INFO << "service is running and wait for exit";
			if (0 == pController->Wait())
			{
				LOG_INFO << "service exit in normal";
			}
		}
		return 0;
	}
	else if (strcmp(argsv[1], "console") == 0)
	{
		pController->Console(&CServerMgr::get_mutable_instance());
	}
	return -1;
}