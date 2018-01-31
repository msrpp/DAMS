#pragma  once
#include "DeviceFactory.h"


class Initer
{
public:
	Initer();
	~Initer();
	map<int, HikDevice*> m_map;
};
class HikDevice :public BaseDevice
{
public:
	virtual int InitParams(map<string, string> params, CDeviceMgr* pSink);
	virtual int Connect();
	virtual int DisConnect();
	virtual map<string, string> GetAllParams();

	virtual int InsertLabel(string strLabelName, int iChannelNo ,string& strGuid);
	
public:
	HikDevice();
	~HikDevice();
	int OnDevEvent();

	CDeviceMgr* m_pSink;
	string m_strUserName;
	string m_strPwd;
	int m_iPort;
	string m_strIp;

	int m_iDevHandle;

/*	static map<int, AutoBaseDevice> m_smap;*/
	static Initer initer;
};