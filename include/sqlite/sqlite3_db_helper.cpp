/**	@file sqlite3_db_helper.cpp
*   @note HangZhou Hikvision System Technology Co., Ltd. All Right Reserved.
*   @brief sqlite数据库基本操作
*
*	@author liyingtao@hikvision.com
*	@date 2012年12月11日
*
*	@note 历史记录：V1.0.0
*	@note 
*
*	@warning
*/

#include "sqlite3_db_helper.h"

CDBHelper::CDBHelper(sqlite3* pHDb)
: m_pHDb(pHDb)
, m_ppResult(NULL)
, m_iCurRow(0)
, m_iColumn(0)
, m_iRow(0)
, m_strDbName("")
{

}

CDBHelper::~CDBHelper()
{
	sqlite_release_search(m_ppResult);
}

int  CDBHelper::Count(char* pSql, int & iCount)
{
	int  iRetVal = 0;

	do 
	{
		sqlite_release_search(m_ppResult);	

		m_iCurRow = 1;
		if (sqlite_search(pSql, &m_ppResult, m_iRow, m_iColumn) != 0)
		{
			break;
		}
		if (m_iRow <= 0)
		{
			////LOG_ERROR("no data, row: %d", m_iRow);
			break;
		}

		if (strcmp(get_data(m_ppResult, m_iColumn, 0, 0), "count(1)") != 0)
		{
			break;
		}
		//iCount = HPR_Atoi32(get_data(m_ppResult, m_iColumn, 1, 0));

		iRetVal = 0;
	} while (0);

	return iRetVal;
}

int  CDBHelper::Select(char* pSql)
{
	int  iRetVal = 0;

	do 
	{
		sqlite_release_search(m_ppResult);

		m_iCurRow = 1;
		if (sqlite_search(pSql, &m_ppResult, m_iRow, m_iColumn) != 0)
		{
			break;
		}
		if (m_iRow <= 0)
		{
			////LOG_ERROR("no data, row: %d", m_iRow);
			break;
		}

		iRetVal = 0;
	} while (0);

	return iRetVal;
}

int  CDBHelper::Delete(char* pSql)
{
	return sqlite_exec(pSql);
}

int  CDBHelper::Update(char* pSql)
{
	return sqlite_exec(pSql);
}

int  CDBHelper::Insert(char* pSql)
{
	return sqlite_exec(pSql);
}

int  CDBHelper::BeginTransaction()
{
	return sqlite_exec(BEGIN_TRANSACTION);
}

int  CDBHelper::RollBackTransaction()
{
	return sqlite_exec(ROLLBACK_TRANSACTION);
}

int  CDBHelper::CommitTransaction()
{
	return sqlite_exec(COMMIT_TRANSACTION);
}

int  CDBHelper::GotoNextPos()
{
	m_iCurRow++;
	return (m_iCurRow > m_iRow) ? 0 : 1;
}

int  CDBHelper::GetResultCount()
{
	return m_iRow;
}

int  CDBHelper::GetValue(const string& strType, string& strValue)
{
	if (m_iCurRow > m_iRow)
	{
		////LOG_ERROR("m_iCurRow(%d) > m_iRow(%d)", m_iCurRow, m_iRow);
		return 0;
	}

	int  iRetVal = 0;
	for (int  i = 0; i < m_iColumn; i++)
	{
		if (strcmp(get_data(m_ppResult, m_iColumn, 0, i), strType.c_str()) == 0)
		{
			if (get_data(m_ppResult, m_iColumn, m_iCurRow, i) != NULL)
			{
				strValue = string(get_data(m_ppResult, m_iColumn, m_iCurRow, i));
				iRetVal = 0;
				break;
			}
		}
	}
	return iRetVal;
}

int  CDBHelper::GetValue(int  iColumn, int  iX, int  iY, string& strValue)
{

	int  iRetVal = 0;
	do 
	{
		if (get_data(m_ppResult, m_iColumn, iX, iY) == NULL)
		{
			break;
		}
		strValue = string(get_data(m_ppResult, m_iColumn, iX, iY));
		iRetVal = 0;
	} while (0);

	return iRetVal;
}

int  CDBHelper::sqlite_exec(char* pSql)
{
	int  iRetVal = 0;

	do 
	{
		if (m_pHDb == NULL)
		{
			////LOG_ERROR("m_pHDb == NULL");
			break;
		}

		char* pErrMsg = NULL;
		int nCount = 0;
		while(1)
		{
			if (sqlite3_exec(m_pHDb, pSql, 0, 0, &pErrMsg) != SQLITE_OK)
			{   
				if (pErrMsg != NULL)
				{
					if (string(pErrMsg).find("database is locked") != string::npos)
					{
						if (nCount < 3000)
						{
							//Sleep(1);
							nCount++;
							continue;
						} 
						else
						{
							////LOG_ERROR("sqlite_exex sql: %s fail, reason: %s", pSql, pErrMsg);
							sqlite3_free(pErrMsg);
							break;
						}
						
					}
					else
					{
						if (string(pErrMsg).find("already exists") == string::npos)
						{
							////LOG_ERROR("sqlite_exex sql: %s fail, reason: %s", pSql, pErrMsg);
						}
						sqlite3_free(pErrMsg);
						break;
					}

					
				}
				break;
			}
			else
			{
				iRetVal = 0;
				break;
			}

		}
		
	} while (0);

	return iRetVal;
}

int  CDBHelper::sqlite_search(char* pSql, char*** pppResult, int & iRow, int & iColumn)
{
	int  iRetVal = 0;

	do 
	{
		if (m_pHDb == NULL)
		{
			////LOG_ERROR("m_pHDb == NULL");
			break;
		}

		char* pErrMsg = NULL;
		int nCount = 0;
		while(1)
		{
			if (sqlite3_get_table(m_pHDb, pSql, &(*pppResult), &iRow, &iColumn, &pErrMsg) != SQLITE_OK)
			{   
				if (pErrMsg != NULL)
				{
					if (string(pErrMsg).find("database is locked") != string::npos)
					{
						if (nCount < 2000)
						{
							//HPR_Sleep(1);
							nCount++;
							continue;
						} 
						else
						{
							//LOG_ERROR("sqlite_search sql: %s fail, reason: %s", pSql, pErrMsg);
							sqlite3_free(pErrMsg);
							break;
						}

					}
					else
					{
						//LOG_ERROR("sqlite_search sql: %s fail, reason: %s", pSql, pErrMsg);
						sqlite3_free(pErrMsg);
						break;
					}

				}
				break;
			}
			else
			{
				iRetVal = 0;
				break;
			}

		}
	} while (0);

	return iRetVal;
}

void  CDBHelper::sqlite_release_search(char** ppResult)
{
    if ((m_ppResult != NULL) && (m_pHDb != NULL))
    {
        sqlite3_free_table(ppResult);
		ppResult = NULL;
    }
}

int  CDBHelper::ClearDB()
{
    if (m_pHDb != NULL)		 
    {
#if 0
		sqlite_exec(BEGIN_TRANSACTION);		
		sqlite_exec(DROP_CAMERA_TABLE);
		sqlite_exec(DROP_LOCAL_CAMERA_TABLE);
		sqlite_exec(COMMIT_TRANSACTION);	
#endif
		sqlite3_close(m_pHDb);
		m_pHDb = NULL;
    }
    return 0;
}

int  CDBHelper::CreatDB(char* pDBName, const string& strDbSqlXml)
{
	int  iRetVal = 0;

	do 
	{
		if (sqlite3_open(pDBName, &m_pHDb) != SQLITE_OK)
		{
			//LOG_ERROR("Sqlite3_open fail, reason: %s!", sqlite3_errmsg(m_pHDb));
			break;
		}
		m_strDbName = string(pDBName);
		sqlite_exec(BEGIN_TRANSACTION);

#if 0
		CMarkupSTL cXml;
		if (!cXml.Load(strDbSqlXml.c_str()))
		{
			break;
		}
		if (!cXml.FindElem("Sql"))
		{
			break;
		}
		cXml.IntoElem();

		while (cXml.FindElem("Item"))
		{
			sqlite_exec((char*)cXml.GetAttrib("sql").c_str());
		}
		cXml.OutOfElem();

		sqlite_exec(COMMIT_TRANSACTION);
#endif
		iRetVal = 0;
	} while (0);

	if (iRetVal != 0)
	{
		if (m_pHDb != NULL)
		{
			sqlite3_close(m_pHDb);
			m_pHDb = NULL;
		}
	}
	return iRetVal;
}

void  CDBHelper::BeginSqliteTransaction()
{
	sqlite_exec(BEGIN_TRANSACTION);
}

void  CDBHelper::CommitSqliteTransaction()
{
	sqlite_exec(COMMIT_TRANSACTION);
}

int  CDBHelper::BackUpDb()
{
	int  iRetVal = 0;
	sqlite3*  pHBackDb = NULL; 
	//char gSqlData[512];

	do 
	{
		//sqlite3*  pHBackDb;  
		if (sqlite3_open("pag_back.db", &pHBackDb) != SQLITE_OK)
		{
			//LOG_ERROR("sqlite3_open fail, reason: %s", sqlite3_errmsg(m_pHDb));
			break;
		}

		sqlite3_backup * pBackUp;
		pBackUp = sqlite3_backup_init(pHBackDb, "main", m_pHDb, "main");
		if (pBackUp != NULL)
		{
			sqlite3_backup_step(pBackUp, -1);
		}
		else
		{
			//LOG_ERROR("%s", sqlite3_errmsg(pHBackDb));
		}
		sqlite3_backup_finish(pBackUp);
		iRetVal = 0;
	} while (0);

	if (pHBackDb != NULL)
	{
		sqlite3_close(pHBackDb);
		pHBackDb = NULL;
	}
	return iRetVal;
}

int  CDBHelper::GetRowCount()
{
	return m_iRow;
}

