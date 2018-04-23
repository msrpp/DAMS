#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "markup/markup.h"
#include "CommonHeader.h"
#define OUT
#define  IN

static const char* DEFAULT_CONFIG = "./config.xml";

class CConfig :public boost::serialization::singleton<CConfig>

{
public:
	bool LoadConfig(const char* filePath = DEFAULT_CONFIG);
	string	GetDevIp() { return strDevIp; }
	string 	GetDevPort() { return strDevPort; }
	string	GetUsrName() { return strUsrName; }
	string	GetPasswd() { return strPasswd; }

	string	GetDBHostName() { return strDBHostName; }
	string 	GetDBHostUserName() { return strDBHostUserName; }
	string	GetDBHostPassword() { return strDBHostPassword; }
	string	GetDBHostObject() { return strDBHostObject; }
	string GetServerName(){ return m_serverName; }
	string	GetPicSavePath() { return strPicSavePath; }
	int getListenPort(){ return m_listenPort; }
	string getPicUrl(){ return m_picUrl; }
	int	GetEnablePicSave() { return m_iEnableSavePic; }
private:
	string  strDevIp;
	string  strDevPort;
	string strUsrName;
	string strPasswd;

	string  strDBHostName;
	string  strDBHostUserName;
	string strDBHostPassword;
	string strDBHostObject;

	string strPicSavePath;
	int m_listenPort;
	string m_serverName;
	string m_picUrl;

	int m_iEnableSavePic;
};


#endif // !_CONFIG_H_


