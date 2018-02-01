#include "MessageHandler.h"
#include "DeviceMgr.h"
#include "json/json.h"
#include "ErrorDef.h"
CMessageHandler::CMessageHandler()
{
}


CMessageHandler::~CMessageHandler()
{
}

string MakeSimpleResponse(int iCode)
{
	Json::Value root;
	root["code"] = iCode;
	Json::FastWriter writer;
	return writer.write(root);
}
int CMessageHandler::Dispatch(string uri, string command, map<string, string>& headers, char*& body, int& iLength, string& strResponse)
{
	int iRetVal = -1;
	do 
	{
		strResponse = MakeSimpleResponse(iRetVal);
		if (NULL == body )
		{
			break;
		}
		LOG_INFO << "recv client msg" << *body;
		Json::Value root;
		Json::Reader reader;

		if (uri.find("AddDevice") != string::npos)
		{
			if (!reader.parse(body, root))
			{
				break;
			}
			try
			{
				string strDevType = root["DeviceType"].asString();
				string devIndex = root["DeviceIndexCode"].asString();
				Json::Value DevInfo = root["DeviceInfo"];
				Json::Value::Members mem = DevInfo.getMemberNames();
				map<string, string> mapDevInfo;
				for (Json::Value::Members::iterator iter = mem.begin(); iter != mem.end(); iter++)
				{
					if (DevInfo[*iter].type() == Json::stringValue)
					{
						mapDevInfo[*iter] = DevInfo[*iter].asString();
					}
				}
				CDeviceMgr::get_mutable_instance().AddDevice(strDevType, devIndex, mapDevInfo);
				iRetVal = 0;
			}
			catch (...)
			{
				iRetVal = ERROR_PARAMS_WRONG;
				break;
			}

			string command2 = root["name"].asString();

		}
		else if (uri.find("DelDevice") != string::npos)
		{
			if (!reader.parse(body, root))
			{
				iRetVal = ERROR_PARAMS_WRONG;
				break;
			}
			try
			{
				string devIndex = root["DeviceIndexCode"].asString();
				CDeviceMgr::get_mutable_instance().RemoveDevice(devIndex, true);
			}
			catch (...)
			{
				iRetVal = ERROR_PARAMS_WRONG;
				break;
			}
		}
		else if (uri.find("QueryDevice") != string::npos)
		{
			Json::Value root;
			map<string, AutoBaseDevice> devMap = CDeviceMgr::get_mutable_instance().GetAllDevice();
			Json::Value devices ;
			root["code"] = 0;
			Json::FastWriter writer;
			int i = 0;
			for (auto iter = devMap.begin(); iter != devMap.end(); iter++, i++)
			{
				map<string, string> devParams = iter->second->GetAllParams();
				devices[i]["indexcode"] = iter->first.c_str();
				for (auto itr = devParams.begin(); itr != devParams.end();itr++)
				{
					devices[i][itr->first] = itr->second;
				}
			}
			root["devices"] = devices;
			strResponse = writer.write(root);
			return 0;
		}
		else if (uri.find("LabelInsert") != string::npos)
		{
			if (!reader.parse(body, root))
			{
				break;
			}
			string strTagName;
			int iChannelNo = -1;
			string devIndex;
			string strRetGuid;
			try
			{
				strTagName = root["tagName"].asString();
				iChannelNo = root["channel"].asInt();
				devIndex = root["DeviceIndexCode"].asString();
				
			}
			catch (...)
			{
				iRetVal = ERROR_PARAMS_WRONG;
				break;
			}

			iRetVal = CDeviceMgr::get_mutable_instance().InsertLabel(strTagName, devIndex, iChannelNo, strRetGuid);
			if (iRetVal != 0)
			{
				break;
			}
			Json::FastWriter writer;
			Json::Value root;
			root["code"] = 0;
			root["identify"] = strRetGuid;
			strResponse = writer.write(root);
			return 0;
		}
		iRetVal = 0;
	} while (0);
	if (iRetVal != 0)
	{
		LOG_WARNING << "operator failed";
		strResponse = MakeSimpleResponse(iRetVal);
	}
	
	return 0;
}
