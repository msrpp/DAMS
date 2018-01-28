#include "CClientSession.h"
#include "DeviceMgr.h"
#include "config.h"

bool dirExists(const std::string& dirName_in)
{
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!  

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!  

	return false;    // this is not a directory!  
}

CDeviceMgr::CDeviceMgr()
{
	
		pDataDriver = NULL;
		pDataConnect = NULL;
		//sql操作对象
		pStmt = NULL;
		pResult = NULL;
		pPrepStmt = NULL;
		pSavepoint = NULL;
	
}


CDeviceMgr::~CDeviceMgr()
{
}

void TimerProc(CDeviceMgr* pDevMgr )
{

	pDevMgr->CheckDevOnline();
}
  
#define NUMOFFSET 100  
#define COLNAME 200 



int CDeviceMgr::Init()
{
	string strfolder = CConfig::get_mutable_instance().GetPicSavePath();
	if (dirExists(strfolder)) {
		std::cout << "Folder : " << strfolder.c_str() << " exist!" << std::endl;
	}
	else {
		std::cout << "Folder : " << strfolder.c_str() << " doesn't exist!" << std::endl;
	}
	m_resStorageMgr.Init();
	vector<DeviceInfo> ldevinfo;
	m_resStorageMgr.GetAllDevice(ldevinfo);
	for (vector<DeviceInfo>::iterator iter = ldevinfo.begin(); iter != ldevinfo.end(); iter++)
	{
		AddDevice(iter->strType, iter->strDevIndex, iter->mapDevParams);
	}
	m_bRun = true;
	m_tOntimer = new boost::thread(boost::bind(TimerProc, this));
	
    /*	m_tOntimer*/
//	string picUri = m_resStorageMgr.GetPicUri();
	testDevExit = false;			 //测试用


	 //数据库连接初始化
	string strDBHostName = CConfig::get_mutable_instance().GetDBHostName();
	string strDBHostUserName = CConfig::get_mutable_instance().GetDBHostUserName();
	string strDBPassWord = CConfig::get_mutable_instance().GetDBHostPassword();
	string strDBHostObject = CConfig::get_mutable_instance().GetDBHostObject();

	do
	{
		try	  //初始化数据库
		{
			pDataDriver = sql::mysql::get_driver_instance();
			if (NULL == pDataDriver)
			{
				break;
			}
			pDataConnect = pDataDriver->connect(strDBHostName.c_str(), strDBHostUserName.c_str(), strDBPassWord.c_str());
			if (NULL == pDataConnect)
			{
				break;
			}
			//数据量大的时候最好关闭自动提交
			pDataConnect->setAutoCommit(0);
			pDataConnect->setSchema(strDBHostObject.c_str());
			string strInitDB = "CREATE TABLE IF NOT EXISTS  `tb_car_plate_info` (`c_plate_info` varchar(128) COLLATE utf8_bin NOT NULL COMMENT '车牌描述信息',`i_camera_id` bigint(20) DEFAULT NULL COMMENT '前端相机id',`c_capture_time` varchar(64) COLLATE utf8_bin DEFAULT NULL COMMENT '抓拍时间');";
			pStmt = pDataConnect->createStatement();
			if (NULL == pDataConnect)
			{
				break;
			}
			pStmt->execute(strInitDB.c_str());
			pDataConnect->commit();
		}
		catch (sql::SQLException e)
		{
			int iErr = e.getErrorCode();
			{
				LOG_INFO << "SQLException：" << iErr;
			}
		}
		return 0;
	} while (0);
	
	{

		if (NULL != pStmt)
		{
			delete pStmt;
		}

		if (NULL != pDataConnect)
		{
			pDataConnect->close();
			delete pDataConnect;
		}

	}
	return 0;
}

int CDeviceMgr::Finit()
{
	m_bRun = false;
	if (m_tOntimer != NULL)
	{
		m_tOntimer->join();
		delete m_tOntimer;
		m_tOntimer = NULL;
	}
	{

		if (NULL != pStmt)
		{
			delete pStmt;
		}

		if (NULL != pDataConnect)
		{
			pDataConnect->close();
			delete pDataConnect;
		}

	}

	return 0;
}

int CDeviceMgr::AddDevice(string strType,string devIndex, map<string, string> Devparams)
{
	m_resStorageMgr.AddDevice(devIndex, strType, Devparams);
	AutoBaseDevice device = m_DevCreater.CreateDevcieByName(strType);
	if (device.get() == NULL)
	{
		//LOG_ERROR << "add Device failed as cant find device type";
	}
	device->InitParams(Devparams,this);
	boost::lock_guard<boost::mutex> lock(m_mutex4DevMap);
	m_devMap[devIndex] = device;
	return 0;
}

int CDeviceMgr::RemoveDevice(string devIndex ,bool bDel)
{
	boost::lock_guard<boost::mutex> lock(m_mutex4DevMap);
	if (m_devMap.find(devIndex) != m_devMap.end())
	{
		m_devMap[devIndex]->DisConnect();
		if (bDel)
		{
			m_resStorageMgr.DelDevice(devIndex);
			m_devMap.erase(devIndex);
		}
	}
	return 0;
}

std::map<string, AutoBaseDevice> CDeviceMgr::GetAllDevice()
{
	boost::lock_guard<boost::mutex> lock(m_mutex4DevMap);
	return m_devMap;
}

int CDeviceMgr::CheckDevOnline()
{
	while (m_bRun)
	{
	for (int i = 0; i < 10; i++)
	{
		if (m_bRun == false)
		{
			return 0;
		}
		Sleep(1000);
	}
		{
			//测试程序begin
			if (testDevExit == false)
			{
				LOG_INFO << "CheckDevOnline start" ;

				map<string, string> Devparams;
				Devparams["ip"] = CConfig::get_mutable_instance().GetDevIp();
				Devparams["port"] = CConfig::get_mutable_instance().GetDevPort();
				Devparams["userName"] = CConfig::get_mutable_instance().GetUsrName();
				Devparams["password"] = CConfig::get_mutable_instance().GetPasswd();
				AddDevice("hik", "20171201", Devparams);
				testDevExit = true;
			}
			//测试程序end _
			map<string, AutoBaseDevice>mapCp = GetAllDevice();

			for (DeviceMapIterator iter = mapCp.begin(); iter != mapCp.end(); iter++)
			{
				iter->second->Connect();
			}
		}

	}

	return 0;
}


int CDeviceMgr::InsertLabel(string strLabelName, string strDevIndex, int iChannelNo ,string &strGuid)
{
	
	AutoBaseDevice device;
	{
		m_mutex4DevMap.lock();
		device = m_devMap[strDevIndex];
		m_mutex4DevMap.unlock();
	}

	if (device == NULL)
	{
		LOG_WARNING << "cant find device " << strDevIndex;
		return -1;
	}
	return device->InsertLabel(strLabelName, iChannelNo, strGuid);
}

int CDeviceMgr::AddPlateData2DB(DB_DATA_PLATEDATA& stPlateData)
{
	char chChanNum[32] = { 0 };
	itoa(stPlateData.iCamerID, chChanNum, 10);
	string strPlateDB = "INSERT INTO `tb_car_plate_info` VALUES ('" + stPlateData.strCarPlateData + "', '" + string(chChanNum) + "','" + stPlateData.strCaptureTime + "');";

	try
	{
		pStmt = pDataConnect->createStatement();
		pStmt->execute(strPlateDB.c_str());

		pDataConnect->commit();
	}
	catch (sql::SQLException e)
	{
		int iErr = e.getErrorCode();
	}

	return 0;
}