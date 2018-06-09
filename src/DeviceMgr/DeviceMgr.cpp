#include "CClientSession.h"
#include "DeviceMgr.h"
#include "config.h"
#include "json/json.h"
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

char* G2U(const char* gb2312)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
	if (wstr) delete[] wstr;
	return str;
}

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
			//string strInitDB = "CREATE TABLE IF NOT EXISTS  `tb_car_plate_info` (`c_plate_info` varchar(128)  NOT NULL COMMENT '车牌描述信息',`c_plate_pic_path` varchar(128)  NOT NULL COMMENT '图片地址',`c_device_ip` varchar(128)  NOT NULL COMMENT '超脑IP',`i_camera_id` bigint(20) DEFAULT NULL COMMENT '前端相机id',`c_capture_time` varchar(64) COLLATE utf8_bin DEFAULT NULL COMMENT '抓拍时间');";
            string strInitDB = "CREATE TABLE IF NOT EXISTS  `tb_car_plate_info` (`pic_id` bigint(20) AUTO_INCREMENT primary key  NOT NULL COMMENT '图片索引', `c_plate_info` varchar(128)  NOT NULL COMMENT '车牌描述信息',`c_plate_color` varchar(128)  NOT NULL COMMENT '车牌颜色',`i_plate_logo_type` bigint(20) DEFAULT NULL COMMENT '车辆品牌',`i_drive_chan` bigint(20) DEFAULT NULL COMMENT '车道号',`c_plate_pic_path` varchar(128)  NOT NULL COMMENT '图片地址',`c_device_ip` varchar(128)  NOT NULL COMMENT '超脑IP',`i_camera_id` bigint(20) DEFAULT NULL COMMENT '前端相机id',`c_capture_time` varchar(64) COLLATE utf8_bin DEFAULT NULL COMMENT '抓拍时间')AUTO_INCREMENT=1 ;";
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

#if 0
		//写数据库		   test
		DB_DATA_PLATEDATA  stPlateData;
		stPlateData.strCaptureTime = "1111";
		stPlateData.strCarPlateData = "2222";
		stPlateData.iCamerID = 1;
		stPlateData.strPicUrl = "/dit/carimage/201810551145113541.jpg";
		stPlateData.strIp = CConfig::get_mutable_instance().GetDevIp();
		stPlateData.iDirChanNum = 2;
		stPlateData.strVehiceColor = "黑色";
		stPlateData.iVehiceLogo = 1;
		{
			LOG_INFO << "car  strCaptureTime   " << stPlateData.strCaptureTime.c_str();
			LOG_INFO << "car  strCarPlateData   " << stPlateData.strCarPlateData.c_str();
			LOG_INFO << "car  iCamerID   " << stPlateData.iCamerID;
		}
		CDeviceMgr::get_mutable_instance().AddPlateData2DB(stPlateData);
#endif
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
			boost::lock_guard<boost::mutex> lock(m_mutex4DevMap);
			for (DeviceMapIterator iter = m_devMap.begin(); iter != m_devMap.end(); iter++)
			{
				
				iter->second->Connect();
			}
		}

	}

	return 0;
}


int CDeviceMgr::AddPlateData2DB(DB_DATA_PLATEDATA& stPlateData)
{
	m_mutex4Sql.lock();
	{
		LOG_INFO << "AddPlateData2DB start   ";
	}
	char chChanNum[32] = { 0 };
	char chDirChanNum[32] = { 0 };
	char chVehiceLogoType[32] = { 0 };
	itoa(stPlateData.iCamerID, chChanNum, 10);
	itoa(stPlateData.iDirChanNum, chDirChanNum, 10);
	itoa(stPlateData.iVehiceLogo, chVehiceLogoType, 10);
	char* pDataRes = G2U(stPlateData.strCarPlateData.c_str());
	stPlateData.strCarPlateData = pDataRes;

	char* pDataRes1 = G2U(stPlateData.strVehiceColor.c_str());
	stPlateData.strVehiceColor = pDataRes1;
	//string strPlateDB = "INSERT INTO `tb_car_plate_info` VALUES ('" + stPlateData.strCarPlateData + "', " + string(chDirChanNum) + "','" + stPlateData.strPicUrl + "','" + stPlateData.strIp + "', " + string(chChanNum) + ",'" + stPlateData.strCaptureTime + "');";
	string strPlateDB = "INSERT INTO `tb_car_plate_info` (c_plate_info,c_plate_color,i_plate_logo_type,i_drive_chan,c_plate_pic_path,c_device_ip,i_camera_id,c_capture_time) VALUES('" + stPlateData.strCarPlateData + "', '" + stPlateData.strVehiceColor + "', " + string(chVehiceLogoType) + ", " + string(chDirChanNum) + ", '" + stPlateData.strPicUrl + "', '" + stPlateData.strIp + "', " + string(chChanNum) + ", '" + stPlateData.strCaptureTime + "'); ";
	{
		LOG_INFO << "strPlateDB: " << strPlateDB.c_str();
	}
	try
	{
		//pStmt = pDataConnect->createStatement();
		pStmt->execute(strPlateDB.c_str());

		pDataConnect->commit();
	}
	catch (sql::SQLException e)
	{
		int iErr = e.getErrorCode();
		{
			LOG_INFO << "err: " << iErr;
			if (iErr == 2006 || iErr == 1046)
			{
				exit(0);
				//pDataConnect->reconnect();
				string strDBHostName = CConfig::get_mutable_instance().GetDBHostName();
				string strDBHostUserName = CConfig::get_mutable_instance().GetDBHostUserName();
				string strDBPassWord = CConfig::get_mutable_instance().GetDBHostPassword();
				string strDBHostObject = CConfig::get_mutable_instance().GetDBHostObject();

				pDataConnect->close();
				delete pDataConnect;
				pDataConnect = pDataDriver->connect(strDBHostName.c_str(), strDBHostUserName.c_str(), strDBPassWord.c_str());

				if (NULL != pStmt)
					delete pStmt;
				pStmt = pDataConnect->createStatement();
	
			}
		}
	}
	if (pDataRes)
	{
		delete[] pDataRes;
	}
	m_mutex4Sql.unlock();
	return 0;
}

int CDeviceMgr::InsertLabel(string strLabelName, string strDeviceIp, string strTime, int iChannelNo, string &strGuid)
{

	AutoBaseDevice device;
	{
		m_mutex4DevMap.lock();
		int i = 0;
		//device = m_devMap[strDevIndex];
		map<string, AutoBaseDevice>::iterator iterFind = m_devMap.begin();
		for (; iterFind != m_devMap.end(); iterFind++)
		{
			map<string, string> devParams = iterFind->second->GetAllParams();
			string strTemp("ip");
			map<string, string>::iterator itr = devParams.find(strTemp);
			if (itr->second == strDeviceIp)
			{
				device = iterFind->second;
				break;
			}
		}
		m_mutex4DevMap.unlock();
	}

	if (device == NULL)
	{
		LOG_WARNING << "cant find device " << strDeviceIp.c_str();
		return -1;
	}

	{
		LOG_INFO << "find device " << strDeviceIp.c_str();

	}
	

	return device->InsertLabel(strLabelName, strTime, iChannelNo, strGuid);
}