#include "ServerControl.h"

// void GainAdminPrivileges(string strApp, UINT idd){
// 	string         strCmd;
// 	strCmd.Format(_T("/adminoption %d"), idd);
// 
// 	SHELLEXECUTEINFO execinfo;
// 	memset(&execinfo, 0, sizeof(execinfo));
// 	execinfo.lpFile = strApp;
// 	execinfo.cbSize = sizeof(execinfo);
// 	execinfo.lpVerb = _T("runas");
// 	execinfo.fMask = SEE_MASK_NO_CONSOLE;
// 	execinfo.nShow = SW_SHOWDEFAULT;
// 	execinfo.lpParameters = strCmd;
// 
// 	ShellExecuteEx(&execinfo);
// 
// }



CServerControl::CServerControl()
{
	memset(&gSvcStatus, 0, sizeof(gSvcStatus));
	gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS; //只有一个单独的服务
	gSvcStatus.dwServiceSpecificExitCode = 0;
}

CServerControl::~CServerControl()
{

}


void __stdcall serverControl(DWORD params)
{
	LOG_INFO << "serverControl func called and params = " << params;
	if (params == SERVICE_CONTROL_STOP || params == SERVICE_CONTROL_SHUTDOWN)
	{
		CServerControl::get_mutable_instance().ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		CServerControl::get_mutable_instance()._OnStop();
	}
	CServerControl::get_mutable_instance().ReportSvcStatus(params, NO_ERROR, 0);

}
VOID __stdcall mmServerEntry(
	DWORD   dwNumServicesArgs,
	LPWSTR  *lpServiceArgVectors
	)
{
	LOG_INFO << "entry mmServerEntry func";
	CServerControl::get_mutable_instance().ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	SERVICE_STATUS_HANDLE handle = RegisterServiceCtrlHandler("mmserver", serverControl);
	if (handle != NULL)
	{
		CServerControl::get_mutable_instance()._SetCtrlHandle(handle);
		CServerControl::get_mutable_instance()._OnRun();
	}
	CServerControl::get_mutable_instance().ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);



}

void CServerControl::ReportSvcStatus(DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;

	// Fill in the SERVICE_STATUS structure.  

	gSvcStatus.dwCurrentState = dwCurrentState;
	gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
	gSvcStatus.dwWaitHint = dwWaitHint;

	if (dwCurrentState == SERVICE_START_PENDING)
		gSvcStatus.dwControlsAccepted = 0;
	else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

	if ((dwCurrentState == SERVICE_RUNNING) ||
		(dwCurrentState == SERVICE_STOPPED))
		gSvcStatus.dwCheckPoint = 0;
	else gSvcStatus.dwCheckPoint = dwCheckPoint++;

	// Report the status of the service to the SCM.  
	SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}


int CServerControl::RunServer(IServerMsgNotify* server)
{
	m_pServer = server;
/*	CServerControl::get_mutable_instance()._OnRun();*/
	const SERVICE_TABLE_ENTRYW entryTable[] =
	{
		{ (LPWSTR)m_szServerName, mmServerEntry },
		{ NULL, NULL },
	};
	if (!StartServiceCtrlDispatcher((CONST SERVICE_TABLE_ENTRYA*)entryTable))
	{
		int iErr = GetLastError();
		//LOG_ERROR << "StartServiceCtrlDispatcher failed system err " << iErr;
		return iErr;
	}
	m_hSvcStopEvent = CreateEvent(
		NULL,    // default security attributes  
		TRUE,    // manual reset event  
		FALSE,   // not signaled  
		NULL);


	return 0;
}

int CServerControl::Wait()
{
	return WaitForSingleObject(m_hSvcStopEvent, INFINITE);
}

int CServerControl::Console(IServerMsgNotify* server)
{
	m_pServer = server;
	_OnRun();
	Sleep(1000000);
	return 0;
}

int CServerControl::_OnRun()
{
	if (m_pServer !=NULL)
	{
		m_pServer->OnStart();
		return 0;
	}
	return -1;
}

int CServerControl::_OnStop()
{
	LOG_INFO << "CServerControl _OnStop called ";
	if (m_pServer != NULL)
	{
		m_pServer->OnStop();
	}
	SetEvent(m_hSvcStopEvent);
	return 0;
}

int CServerControl::Install()
{
	if (IsInstalled())
		return 0;

	//打开服务控制管理器   
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hSCM == NULL)
	{
		//LOG_ERROR <<"OpenSCManager failed system err "<<GetLastError();
		return -1;
	}

	// Get the executable file path   
	TCHAR szFilePath[MAX_PATH];
	::GetModuleFileName(NULL, szFilePath, MAX_PATH);
	m_strExePath = szFilePath;

	string strFilePath = "\"" + string(szFilePath) + "\" " + "\"service\"";
	//创建服务   
	SC_HANDLE hService = ::CreateService(
		hSCM, m_szServerName, m_szServerName,
		SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
		strFilePath.c_str(), NULL, NULL, (""), NULL, NULL);

	if (hService == NULL)
	{
		//LOG_ERROR << "CreateService failed system err " << GetLastError();
		::CloseServiceHandle(hSCM);
		return -1;
	}

	::CloseServiceHandle(hService);
	::CloseServiceHandle(hSCM);
	return 0;
}



int CServerControl::Uninstall()
{
		if (!IsInstalled())
			return 0;

		SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

		if (hSCM == NULL)
		{
			//LOG_ERROR << "OpenSCManager failed system err " << GetLastError();
			return -1;
		}

		SC_HANDLE hService = ::OpenService(hSCM, m_szServerName, SERVICE_STOP | DELETE);

		if (hService == NULL)
		{
			//LOG_ERROR << "OpenService failed system err " << GetLastError();
			::CloseServiceHandle(hSCM);
			return -1;
		}
		SERVICE_STATUS status;
		::ControlService(hService, SERVICE_CONTROL_STOP, &status);

		//删除服务   
		BOOL bDelete = ::DeleteService(hService);
		::CloseServiceHandle(hService);
		::CloseServiceHandle(hSCM);

		if (bDelete)
			return -1;
		return 0;
}

bool CServerControl::IsInstalled()
{
	bool bResult = false;

	//打开服务控制管理器   
	SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (hSCM != NULL)
	{
		//打开服务   
		SC_HANDLE hService = ::OpenService(hSCM, m_szServerName, SERVICE_QUERY_CONFIG);
		if (hService != NULL)
		{
			LOG_INFO << "server has been installed " << GetLastError();
			bResult = true;
			::CloseServiceHandle(hService);
		}
		::CloseServiceHandle(hSCM);
		return bResult;
	}
	int iErr = GetLastError();
	return iErr;
}