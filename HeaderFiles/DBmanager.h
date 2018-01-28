#ifndef _DBMANAGER_H_
#define _DBMANAGER_H_


#include "sqlite/sqlite3.h"
#include "CommonHeader.h"

struct DeviceInfo
{
	string strDevIndex;
	string strType;
	map<string, string> mapDevParams;
};

class DeviceDbStorage
{
private:
	sqlite3* m_pdb;
public:
	bool Init();
	void Finit();
	void DelDevice(string strDevIndex);
	bool AddDevice(string strDevIndex, string strType, map<string, string>mapDevParams);

	bool GetAllDevice(vector<DeviceInfo> &listDev);
};
#endif

