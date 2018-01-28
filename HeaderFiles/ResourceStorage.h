#pragma once
#include "CommonHeader.h"
class CResourceStorage
{
public:
	void GetAllResource();
	void PushDevice();
	void SetPicUri();
	string GetPicUri();
	void DelDevice(string Indexcode);
};