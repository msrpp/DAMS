#pragma once
#include"CommonHeader.h"
#include "DeviceFactory.h"
#include "ResourceStorage.h"
#include "DBmanager.h"

// 新增数据库头文件（后期建议数据库操作独立一个单例类）
#include "mysql_driver.h"  
#include "mysql_connection.h"  
#include "cppconn/driver.h"  
#include "cppconn/statement.h"  
#include "cppconn/prepared_statement.h"  
#include "cppconn/metadata.h"  
#include "cppconn/exception.h" 

enum DEV_STATUS
{
	STATUS_ONLINE = 0,
	STATUS_OFFLINE = 1,
	STATUS_BREAKDOWN = 2
};

enum DEV_MSG_TYPE
{
	TYPE_PICTURE = 0,
	TYPEz_EVENT = 1
};

typedef struct
{
	string strCarPlateData;
	string strCaptureTime;
	int iCamerID;
}DB_DATA_PLATEDATA;


class CDeviceMgr:public boost::serialization::singleton<CDeviceMgr>
{
public:
	CDeviceMgr();
	
	~CDeviceMgr();

	int Init();
	int Finit();
	int AddDevice(string strType,string devIndex, map<string, string> Devparams);
	int RemoveDevice(string devIndex,bool bDelFromDisk = false);
	map<string, AutoBaseDevice> GetAllDevice();
	int OnDevStatusChange(DEV_STATUS status);
	int OnDevMsgCallBack(DEV_MSG_TYPE ,void* params);
	int AddPlateData2DB(DB_DATA_PLATEDATA& stPlateData);

public:
	int CheckDevOnline();
	int InsertLabel(string strLabelName, string strDevIndex, int iChannelNo ,string& strGuid);
private:
	DeviceFactory m_DevCreater;
/*	CResourceStorage m_resStorageMgr;*/
	map<string, AutoBaseDevice> m_devMap;
	typedef map<string, AutoBaseDevice>::iterator DeviceMapIterator;
	boost::mutex m_mutex4DevMap;
	boost::thread* m_tOntimer;
	int m_iTimer;
	bool m_bRun;
	bool testDevExit;

	//sql对象
	sql::Driver *pDataDriver;
	sql::Connection *pDataConnect;
	//sql操作对象
	sql::Statement *pStmt;
	sql::ResultSet *pResult;
	sql::PreparedStatement *pPrepStmt;
	sql::Savepoint *pSavepoint;

	DeviceDbStorage m_resStorageMgr;
};

