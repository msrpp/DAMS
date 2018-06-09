#include "DBmanager.h"
#include <stdio.h>
#include "io.h"


map<string,string> parseParams(string strParams)
{
	int iCurPos = 0;
	map<string, string> retVal;
	while (1)
	{
		int iPos = strParams.find(";",iCurPos);
		if (iPos == string::npos)
		{
			break;
		}
		string strPart = strParams.substr(iCurPos, iPos - iCurPos);
		int iPos2 = strPart.find("=");
		if (iPos2 != -1)
		{
			string key = strPart.substr(0, iPos2);
			string value = strPart.substr(iPos2+1, strPart.length() - iPos2-1);
			retVal[key] = value;
		}
		else
		{
			break;
		}
		iCurPos = iPos+1;

	}
	return retVal;
}
bool DeviceDbStorage::Init()
{
	int rc = -1;
	if (0 == _access("local.db", 0))
	{
		rc = sqlite3_open("local.db", &m_pdb);
		return rc == 0;
	}
	rc = sqlite3_open("local.db", &m_pdb);
	if (rc != 0)
	{
		return false;
	}
	//½¨±í

	char* sql = "CREATE TABLE device("  \
		"devIndex TEXT PRIMARY KEY     NOT NULL," \
		"type           TEXT    NOT NULL," \
		"devParams            TEXT     NOT NULL);";

	/* Execute SQL statement */
	char* zErrMsg;
	rc = sqlite3_exec(m_pdb, sql, NULL, 0, &zErrMsg);
// 	map<string, string>dd;
// 	dd["123"] = "4565";
// 	dd["sss"] = "dsaf";
// 	AddDevice("test", "hik", dd);
// 	vector<DeviceInfo> listDev;
// 	GetAllDevice(listDev);

}

void DeviceDbStorage::DelDevice(string strDevIndex)
{
	char sql[256] = { 0 };
	sprintf_s(sql, 256, "delete from device where devIndex = \"%s\" ", strDevIndex.c_str());

	char* zErrMsg;
	sqlite3_exec(m_pdb, sql, NULL, 0, &zErrMsg);
}

bool DeviceDbStorage::AddDevice(string strDevIndex, string strType, map<string, string>mapDevParams)
{
	char sql[256] = { 0 };
	string strDevParams;
	for (map<string, string>::iterator iter = mapDevParams.begin(); iter != mapDevParams.end();iter++)
	{
		strDevParams.append(iter->first).append("=").append(iter->second).append(";");
	}
	sprintf_s(sql,256, "insert into device values(\"%s\",\"%s\",\"%s\");",strDevIndex.c_str(),strType.c_str(),strDevParams.c_str());

	char* zErrMsg;
	return sqlite3_exec(m_pdb, sql, NULL, 0, &zErrMsg) == 0;
}

bool DeviceDbStorage::GetAllDevice(vector<DeviceInfo> &listDev)
{
	char sql[256] = { 0 };
	char* zErrMsg;
	int iRow=0, iColumn=0;
	char** pppResult;
	if (0 == sqlite3_get_table(m_pdb, "select * from device", &(pppResult), &iRow, &iColumn, &zErrMsg))
	{
		if (iColumn != 3)
		{
			return false;
		}
		for (int i = 1; i <= iRow;i++)
		{
			DeviceInfo info;
			info.strDevIndex = pppResult[i*iColumn + 0];
			info.strType = pppResult[i*iColumn+1];
			string strParams = pppResult[i*iColumn + 2];
			info.mapDevParams = parseParams(strParams);
			listDev.push_back(info);

		}
	}
	return true;
}

