#include "CClientSession.h"
#include "HikDevice.h"
#include "HCNetSDK.h"
#include "sqlite3_db_helper.h"
#include <stdio.h>
#include "DeviceMgr.h"
#include "config.h"
#include "json/json.h"

static Initer g_initer;

void switchColorData(OUT string& strColorData, IN int iColorData)
{
	switch (iColorData)
	{
	case 0:
	case 0xff:
		strColorData = "其他";
		break;
	case 1:
		strColorData = "白色";
		break;
	case 2:
		strColorData = "银色";
		break;
	case 3:
		strColorData = "灰色";
		break;
	case 4:
		strColorData = "黑色";
		break;
	case 5:
		strColorData = "红色";
		break;
	case 6:
		strColorData = "深蓝";
		break;
	case 7:
		strColorData = "蓝色";
		break;
	case 8:
		strColorData = "黄色";
		break;
	case 9:
		strColorData = "绿色";
		break;
	case 10:
		strColorData = "棕色";
		break;
	case 11:
		strColorData = "粉色";
		break;
	case 12:
		strColorData = "紫色";
		break;
	case 13:
		strColorData = "深灰";
		break;
	case 14:
		strColorData = "青色";
		break;
	default:
		strColorData = "其他";
		break;
	}
	return;
}

BOOL CALLBACK HIKMSGCallBack(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
	LOG_INFO << "Event happens lcommand = " << lCommand;
	//if (g_initer.m_map.find(lCommand) == g_initer.m_map.end())
	//{
	//	return false;
		
	//}
	//HikDevice* pDevice = g_initer.m_map[pAlarmer->];

	//获取相关事件信息
	{
		LOG_INFO << "Event happens lcommand =  " << lCommand;
	}

	char chTime[256] = { 0 };
	char chTimeDB[256] = { 0 };
	time_t tlocalTime = time(NULL);
	struct tm* pstTimer = localtime(&tlocalTime);

	unsigned int dwY = pstTimer->tm_year + 1900;
	unsigned int dwM = pstTimer->tm_mon + 1;
	unsigned int dwD = pstTimer->tm_mday;
	unsigned int dwH = pstTimer->tm_hour;
	unsigned int dwMin = pstTimer->tm_min;
	unsigned int dwSec = pstTimer->tm_sec;

	//mod 修改时间格式
	_snprintf(chTime, 256 - 1, "%4d%02d%02d%02d%02d%02d", dwY, dwM, dwD, dwH, dwMin, dwSec);
	_snprintf(chTimeDB, 256 - 1, "%4d-%02d-%02d %02d:%02d:%02d", dwY, dwM, dwD, dwH, dwMin, dwSec);

	switch (lCommand)
	{
	
		case COMM_ITS_PLATE_RESULT:
		{
			//车牌_时间_摄像机ID_图片数据
			NET_ITS_PLATE_RESULT* pAlarm = (NET_ITS_PLATE_RESULT*)pAlarmInfo;
			  
			string strCarLicense = pAlarm->struPlateInfo.sLicense;

			char* pPicPlate = (char*)pAlarm->struPicInfo[0].pBuffer;
			DWORD pPic1Len = pAlarm->struPicInfo[0].dwDataLen;
	
			//log
			{
				LOG_INFO << "car  license info :  " << strCarLicense.c_str();
				LOG_INFO << "car  license  date len:  " << pPic1Len;
				LOG_INFO << "car  Pic num   " << pAlarm->dwPicNum;
				LOG_INFO << "car  DriveChan num   " << (int)pAlarm->byDriveChan;
				LOG_INFO << "car  Dir num   " << (int)pAlarm->byDir;
				LOG_INFO << "car  DetectType num   " << (int)pAlarm->byDetectType;
				LOG_INFO << "car  CarDirectionType num   " << (int)pAlarm->byCarDirectionType;
				LOG_INFO << "car  Vehice Color   " << (int)pAlarm->struVehicleInfo.byColor;
				LOG_INFO << "car  Vehice Logo   " << (int)pAlarm->struVehicleInfo.wVehicleLogoRecog;
			}
			  
			//Carpic name	    以下操作建议另起线程操作
			{
				//存图片
				string strfolder = CConfig::get_mutable_instance().GetPicSavePath();
				string strFileName1 = strfolder + "/" + "CarData_" + strCarLicense + "_"  + string(chTime)  + ".jpg";
				FILE* pFile_Carpic = NULL;
				CClientSession session;
				string retBody;
				string strRetUrl;
				if (CConfig::get_mutable_instance().GetEnablePicSave()==0)
				{

					if (0 == session.http_client_short_link(CConfig::get_mutable_instance().getPicUrl().c_str(), pPicPlate, pPic1Len, retBody))
					{
						Json::Value root;
						Json::Reader reader;
						try{
							if (reader.parse(retBody, root))
							{
								//strRetUrl = root["imageUrl"].asString();

								Json::Value strRetUrl1 = root["data"];
								string retBodyEx = strRetUrl1["imageUrl"].asString();
								strRetUrl = retBodyEx;
							}
							else
							{
								strRetUrl = string("recv data error");
							}

							{
								LOG_INFO << "car  Pic path Ex  " << strRetUrl.c_str();
							}
						}
						catch (...)
						{
							strRetUrl = string("recv data error");
							LOG_WARNING << "parse ret body failed";
						}

					}
				}
				  

				{
					LOG_INFO << "car  Pic path   " << strFileName1.c_str();
				}


				pFile_Carpic = fopen(strFileName1.c_str(), "wb");
				if (pFile_Carpic != NULL&&pPic1Len>0)
				{
					fwrite(pPicPlate, pPic1Len, 1, pFile_Carpic);
				}

				if (pFile_Carpic != NULL)
				{
					fclose(pFile_Carpic);
				}

				{
					LOG_INFO << "start insert db   " ;

				}
				//写数据库
				DB_DATA_PLATEDATA  stPlateData;
				stPlateData.strCaptureTime = string(chTimeDB);
				stPlateData.strCarPlateData = strCarLicense;
				stPlateData.iCamerID = (int)pAlarm->byChanIndex;
				stPlateData.strPicUrl = strRetUrl;
				stPlateData.iDirChanNum = (int)pAlarm->byDriveChan;
				stPlateData.strIp = pAlarmer->sDeviceIP/*CConfig::get_mutable_instance().GetDevIp()*/;
				  
				switchColorData(stPlateData.strVehiceColor, pAlarm->struVehicleInfo.byColor);
				stPlateData.iVehiceLogo = (int)pAlarm->struVehicleInfo.wVehicleLogoRecog;
				{
					LOG_INFO << "car  strCaptureTime   " << stPlateData.strCaptureTime.c_str();
					LOG_INFO << "car  strCarPlateData   " << stPlateData.strCarPlateData.c_str();
					LOG_INFO << "car  iCamerID   " << stPlateData.iCamerID;
				}
				CDeviceMgr::get_mutable_instance().AddPlateData2DB(stPlateData);
	  
			}
		}
		break;
		case COMM_UPLOAD_FACESNAP_RESULT:
		{
			NET_VCA_FACESNAP_RESULT* pAlarm = (NET_VCA_FACESNAP_RESULT*)pAlarmInfo;
			FILE* pFileFace = NULL;
			FILE* pFileBackGround = NULL;
			int iLenFace = 0;
			int iLenBackGround = 0;

			char* pFace = (char*)pAlarm->pBuffer1;
			iLenFace = pAlarm->dwFacePicLen;

			char* pFaceGround = (char*)pAlarm->pBuffer2;
			iLenBackGround = pAlarm->dwBackgroundPicLen;
			string strfolder = CConfig::get_mutable_instance().GetFacePicSavePath();

			string strFileNameFace = strfolder + "/" + "FaceData_" + "_" + string(chTime) + ".jpg";
			string strFileNameBackGround = strfolder + "/" + "BackGroundData_" + "_" + string(chTime) + ".jpg";

			//小图
			pFileFace = fopen(strFileNameFace.c_str(), "wb");
			if (pFileFace != NULL && iLenFace>0)
			{
				fwrite(pFace, iLenFace, 1, pFileFace);
			}

			if (pFileFace != NULL)
			{
				fclose(pFileFace);
			}

			//大图
			pFileBackGround = fopen(strFileNameBackGround.c_str(), "wb");
			if (pFileBackGround != NULL && iLenBackGround>0)
			{
				fwrite(pFaceGround, iLenBackGround, 1, pFileBackGround);
			}

			if (pFileBackGround != NULL)
			{
				fclose(pFileBackGround);
			}
		}
		break;
		case COMM_ALARM_PDC:
		{
			NET_DVR_PDC_ALRAM_INFO* pAlarm = (NET_DVR_PDC_ALRAM_INFO*)pAlarmInfo;
		}
		break;
	default:
		break;
	}


	return true;
}

void CALLBACK EventMsgCBListen(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
	LOG_INFO << "EventMsgCBListen start  " ;

	HIKMSGCallBack(lCommand, pAlarmer, pAlarmInfo, dwBufLen, pUser);
}

int HikDevice::InitParams(map<string, string> params, CDeviceMgr* pSink)
{
	m_params = params;
	m_strIp = params["ip"];
	m_iPort = atoi(params["port"].c_str());
	m_strUserName = params["userName"];
	m_strPwd = params["password"];
	return 0;
}

int HikDevice::Connect()
{
	if (m_iDevHandle >=0)
	{
		return 0;
	}
	NET_DVR_DEVICEINFO_V30 devInfo = { 0 };
	m_iDevHandle = NET_DVR_Login_V30((char*)m_strIp.c_str(), m_iPort, (char*)m_strUserName.c_str(), (char*)m_strPwd.c_str(), &devInfo);
	if (m_iDevHandle < 0)
	{
		LOG_INFO << "login failed ip reson:" << NET_DVR_GetLastError();
		LOG_INFO << "login failed ip" << m_strIp.c_str();
		return -1; 
	}
	else
	{
		LOG_INFO << "login sucessed ip :" << m_strIp.c_str();
	}
	g_initer.m_map[m_iDevHandle] = this;

	NET_DVR_SETUPALARM_PARAM struSetupParam = { 0 };
	struSetupParam.dwSize = sizeof(NET_DVR_SETUPALARM_PARAM);
	struSetupParam.byLevel = 1; //布防优先级：0- 一等级（高），1- 二等级（中）   
	struSetupParam.byAlarmInfoType = 1;

	NET_DVR_SetupAlarmChan_V41(m_iDevHandle, &struSetupParam);
	return 0;
}

int HikDevice::DisConnect()
{
	return 0;
}

std::map<string, string> HikDevice::GetAllParams()
{
	return m_params;
}

HikDevice::HikDevice()
:m_pSink(NULL)
, m_iPort(-1)
{

}

HikDevice::~HikDevice()
{

}

void CALLBACK hikStreamCallback(LONG lPlayHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, void *pUser)
{
	LOG_INFO << "hikStreamCallback Call suc";
	return;
}
int HikDevice::InsertLabel(string strLabelName, string strTime, int iChannelNo, string& strGuid)
{
	if (m_iDevHandle < 0)
	{
		LOG_WARNING << "dev login failed";
		//return -1;
	}

	SYSTEMTIME time = {0};

	if (!strTime.empty())
	{
		//NET_DVR_TIME stSDKTime = {0};
		struct tm stTime = { 0 };
		sscanf(strTime.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
			&stTime.tm_year, &stTime.tm_mon, &stTime.tm_mday, &stTime.tm_hour, &stTime.tm_min, &stTime.tm_sec);
		time.wYear = (DWORD)stTime.tm_year;
		time.wMonth = (DWORD)stTime.tm_mon;
		time.wDay = (DWORD)stTime.tm_mday;
		time.wHour = (DWORD)stTime.tm_hour;
		time.wMinute = (DWORD)stTime.tm_min;
		time.wSecond = (DWORD)stTime.tm_sec;
	}
	else
	{
		GetLocalTime(&time);
	}


	NET_DVR_RECORD_LABEL label = { 0 };
	label.struTimeLabel.dwYear = time.wYear;
	label.struTimeLabel.dwMonth = time.wMonth;
	label.struTimeLabel.dwDay = time.wDay;
	label.struTimeLabel.dwHour = time.wHour;
	label.struTimeLabel.dwMinute = time.wMinute;
	label.struTimeLabel.dwSecond = time.wSecond;
	label.dwSize = sizeof(NET_DVR_RECORD_LABEL);


	double days;
	SystemTimeToVariantTime(&time, &days);
	days -= 0.0006944*5;//5分钟以前
	VariantTimeToSystemTime(days, &time);

	LOG_INFO << "Insert Label begin " << m_strIp.c_str() << "channo = " << iChannelNo;
	NET_DVR_TIME beginTime = { 0 };
	NET_DVR_TIME endTime = { 0 };
	endTime.dwYear = time.wYear;
	endTime.dwMonth = time.wMonth;
	endTime.dwDay = time.wDay;
	endTime.dwHour = time.wHour;
	endTime.dwMinute = time.wMinute;
	endTime.dwSecond = time.wSecond;
	SystemTimeToVariantTime(&time, &days);
	days -= 0.0006944;//一分钟以前
	VariantTimeToSystemTime(days, &time);

	beginTime.dwYear = time.wYear;
	beginTime.dwMonth = time.wMonth;
	beginTime.dwDay = time.wDay;
	beginTime.dwHour = time.wHour;
	beginTime.dwMinute = time.wMinute;
	beginTime.dwSecond = time.wSecond;

	NET_DVR_VOD_PARA stVODPara = { 0 };
	stVODPara.struIDInfo.dwChannel = iChannelNo;
	memcpy(&(stVODPara.struBeginTime), &beginTime, sizeof(NET_DVR_TIME));
	memcpy(&(stVODPara.struEndTime), &endTime, sizeof(NET_DVR_TIME));
	int iRetVal = -1;
	do
	{
		int iPlayHandle = NET_DVR_PlayBackByTime_V40(m_iDevHandle, &stVODPara);
		if (iPlayHandle < 0)
		{
			LOG_WARNING << "NET_DVR_PlayBackByTime_V40 failed error code:" << NET_DVR_GetLastError();
			break;
		}
		if (!NET_DVR_PlayBackControl(iPlayHandle, NET_DVR_PLAYSTART, 0, NULL))
		{
			LOG_WARNING << "NET_DVR_PlayBackControl failed error code:" << NET_DVR_GetLastError();
			break;
		}
		if (!NET_DVR_SetPlayDataCallBack_V40(iPlayHandle, hikStreamCallback, this))
		{
			LOG_WARNING << "NET_DVR_SetPlayDataCallBack_V40 failed error code:" << NET_DVR_GetLastError();
			break;
		}
		NET_DVR_LABEL_IDENTIFY identify = { 0 };
		strncpy((char*)label.sLabelName, strLabelName.c_str(), LABEL_NAME_LEN);

		bool loop = true;
		int iTryTimes = 10;
		do 
		{
			iTryTimes--;
			if (!NET_DVR_InsertRecordLabel(iPlayHandle, &label, &identify))
			{
				int iErrCode = NET_DVR_GetLastError();
				LOG_WARNING << "NET_DVR_InsertRecordLabel failed error code:" << iErrCode;
				if (iErrCode != 29 || iTryTimes<0)
				{
					loop = false;
				}
				else
				{
					Sleep(1000);
				}
			}
			else
			{
				LOG_INFO << "insert label suc";
				loop =false;
			}

		} while (loop);

		strGuid = (char*)identify.sLabelIdentify;
		iRetVal = 0;
	} while (0);

	return iRetVal;
}


Initer::Initer()
{

	NET_DVR_Init();
	NET_DVR_SetDVRMessageCallBack_V31(HIKMSGCallBack, NULL);

	//创建数据库
	//CDBHelper cDBobj;
	//cDBobj.CreatDB("ttt.db", "111.txt");
	//int iRet = sqlite3_open("Date.db", &pDb);

}

Initer::~Initer()
{

}


