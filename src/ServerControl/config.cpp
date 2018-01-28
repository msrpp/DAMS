#include "config.h"

bool CConfig::LoadConfig(const char* filePath /*= DEFAULT_CONFIG*/)
{
	CMarkup markup;
	if (!markup.Load(filePath))
	{
		return false;
	}

	if (markup.FindElem(NULL))
	{
		markup.IntoElem();
		markup.FindElem("DevIp");
		strDevIp = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DevPort");
		strDevPort = markup.GetData().c_str();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("UsrName");
		strUsrName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("Passwd");
		strPasswd = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostName");
		strDBHostName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostUserName");
		strDBHostUserName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostPassword");
		strDBHostPassword = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostObject");
		strDBHostObject = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("strPicSavePath");
		strPicSavePath = markup.GetData();
	}
	if (markup.FindElem(NULL))
	{
		markup.FindElem("listenPort");
		m_listenPort = atoi(markup.GetData().c_str());
	}
	if (markup.FindElem(NULL))
	{
		markup.FindElem("serverName");
		m_serverName = markup.GetData();
	}
	if (markup.FindElem(NULL))
	{
		markup.FindElem("picUrl");
		m_picUrl = markup.GetData();
	}
	return true;
}
