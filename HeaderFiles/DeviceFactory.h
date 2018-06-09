#pragma once
#include"CommonHeader.h"

#include <boost/thread/thread.hpp>
class CDeviceMgr;
class HikDevice;

class BaseDevice
{
public:
	virtual int InitParams(map<string, string> params,CDeviceMgr* pSink) = 0;
	virtual int Connect() = 0;
	virtual int DisConnect() = 0;
	virtual map<string, string> GetAllParams() = 0;

	virtual int InsertLabel(string strLabelName, string strTime, int iChannelNo, string& strRetGuid)
	{
		return -1;
	}
public:

protected:
	map<string, string> m_params;
};
typedef boost::shared_ptr<BaseDevice> AutoBaseDevice;

class DeviceFactory
{
public:
	AutoBaseDevice CreateDevcieByName(string devType);

};