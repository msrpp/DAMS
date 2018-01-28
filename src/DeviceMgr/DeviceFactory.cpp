#include "DeviceFactory.h"
#include "HikDevice.h"


AutoBaseDevice DeviceFactory::CreateDevcieByName(string devType)
{
		AutoBaseDevice dev;
		if (devType == "hik")
		{
			BaseDevice* pDev = new HikDevice();
			dev = AutoBaseDevice(pDev);
		}
		return  dev;
}
