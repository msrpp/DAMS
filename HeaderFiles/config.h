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
	string	GetDevIp()       { return m_strDevIp; }
	string 	GetDevPort()     { return m_strDevPort; }
	string	GetUsrName()     { return m_strUsrName; }
	string	GetPasswd()      { return m_strPasswd; }

	string	GetDBHostName()  { return m_strDBHostName; }
	string 	GetDBHostUserName() { return m_strDBHostUserName; }
	string	GetDBHostPassword() { return m_strDBHostPassword; }
	string	GetDBHostObject()   { return m_strDBHostObject; }
	string GetServerName()      { return m_serverName; }
	string	GetPicSavePath()    { return m_strPicSavePath; }
	int getListenPort()         { return m_listenPort; }
	string getPicUrl()          { return m_picUrl; }
	int	GetEnablePicSave()      { return m_iEnableSavePic; }

	string	GetFacePicSavePath(){ return m_strFacePicSavePath; }
	string getPicFaceUrl()          { return m_picFaceUrl; }
	string getBKGUrl()          { return m_BKGUrl; }
	string getpicIntrusionUrl()          { return m_picIntrusionUrl; }
private:
	string  m_strDevIp;
	string  m_strDevPort;
	string  m_strUsrName;
	string  m_strPasswd;

	string  m_strDBHostName;
	string  m_strDBHostUserName;
	string  m_strDBHostPassword;
	string  m_strDBHostObject;

	string  m_strPicSavePath;
	string  m_strFacePicSavePath;
	int     m_listenPort;
	string  m_serverName;
	string  m_picUrl;
	string  m_picFaceUrl;
	string  m_BKGUrl;
	string  m_picIntrusionUrl;
	int     m_iEnableSavePic;
};


#endif // !_CONFIG_H_


