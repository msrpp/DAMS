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
		markup.FindElem("ServerName");
		m_serverName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DevIp");
		m_strDevIp = markup.GetData();
	}
	if (markup.FindElem(NULL))
	{
		markup.FindElem("DevPort");
		m_strDevPort = markup.GetData().c_str();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("UsrName");
		m_strUsrName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("Passwd");
		m_strPasswd = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostName");
		m_strDBHostName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostUserName");
		m_strDBHostUserName = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostPassword");
		m_strDBHostPassword = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("DBHostObject");
		m_strDBHostObject = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("strPicSavePath");
		m_strPicSavePath = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("strFacePicSavePath");
		m_strFacePicSavePath = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("listenPort");
		m_listenPort = atoi(markup.GetData().c_str());
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("picUrl");
		m_picUrl = markup.GetData();
	}

	if (markup.FindElem(NULL))
	{
		markup.FindElem("enablePicSave");
		m_iEnableSavePic = atoi(markup.GetData().c_str());
	}
	return true;
}
