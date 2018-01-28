#pragma once
#include <windows.h>
#include"CommonHeader.h"
class IServerMsgNotify
{
public:
	virtual int OnStart() = 0;
	virtual int OnStop() = 0;

};

class CServerControl :public boost::serialization::singleton<CServerControl>
{
public:
	CServerControl();
	~CServerControl();
	int Init(const char* serverName){ m_szServerName = serverName; return 0; }
	int Install();
	int Uninstall();
	int RunServer(IServerMsgNotify* server);
	int Wait();
	int Console(IServerMsgNotify* server);
public:
	void ReportSvcStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
	int _OnRun();
	int _OnStop();
	void _SetCtrlHandle(SERVICE_STATUS_HANDLE handle){ gSvcStatusHandle = handle; }
private:

	bool IsInstalled();

	const char* m_szServerName;
	IServerMsgNotify* m_pServer;
	string m_strExePath;
	HANDLE m_hSvcStopEvent;
	SERVICE_STATUS_HANDLE   gSvcStatusHandle; //服务状态句柄
	SERVICE_STATUS          gSvcStatus;  //服务状态
};

