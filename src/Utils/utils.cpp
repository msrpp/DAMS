#include "utils.h"
#include "Logger.h"
#include <winsock.h>



std::string SetCurrentWorkDir()
{
	char strModule[256];
	GetModuleFileName(NULL, strModule, 256);

	std::string strExePath = strModule;
	size_t nPos = strExePath.rfind('\\');
	std::string strDirPath = strExePath.substr(0, nPos);
	SetCurrentDirectory(strDirPath.c_str()); 
	return strDirPath;
}


int bind_by_port(int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	u_long nonblocking = 1;
	if (ioctlsocket(fd, FIONBIO, &nonblocking) == SOCKET_ERROR)
	{
		//LOG_ERROR << "set nonblocking failed";
		return INVALID_SOCKET;
	}
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char*)(void *)&on, sizeof(on)) < 0)
	{
		int iRe = GetLastError();
		//LOG_ERROR << "set keepalive failed";
		return INVALID_SOCKET;
	}

	SOCKADDR_IN addSvr = { 0 };
	addSvr.sin_family = AF_INET;
	addSvr.sin_port = htons(port);
	addSvr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	int iRet = bind(fd, (LPSOCKADDR)&addSvr, sizeof(SOCKADDR_IN));
	return fd;
}