/**	@file sqlite3_db_helper.h
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

#ifndef _SQLITE3_DB_HELPER_H_
#define _SQLITE3_DB_HELPER_H_

#include <string>
#include "sqlite3.h"
//#include "markup.h"
//#include "pag_def.h"
using namespace std;

#define BEGIN_TRANSACTION		"begin transaction"
#define COMMIT_TRANSACTION		"commit transaction"
#define ROLLBACK_TRANSACTION	"rollback transaction"

#define DROP_CAMERA_TABLE		"Drop Table camera;"
#define DROP_LOCAL_CAMERA_TABLE "Drop Table local_camera;"

class CDBHelper
{
public:
    CDBHelper(sqlite3* pHDb = NULL);
    ~CDBHelper();

	int	CreatDB(char* pDBName, const string& strDbSqlXml);			
	int	ClearDB();
	sqlite3*	GetDB()												{ return m_pHDb; }

	int	Select(char* pSql);
	int	Delete(char* pSql);
	int	Update(char* pSql);
	int	Insert(char* pSql);
	int	Count(char* pSql, int& iCount);
	int	GotoNextPos();
	int	GetResultCount();
	int	GetValue(const string& strType, string& strValue);
	int	GetValue(int iColumn, int iX, int iY, string& strValue);

	int	BeginTransaction();
	int	RollBackTransaction();
	int	CommitTransaction();

	int	BackUpDb();
	int	GetRowCount();

	void 	BeginSqliteTransaction();
	void 	CommitSqliteTransaction();

private:
	int	sqlite_search(char* pSql, char*** pppResult, int& iRow, int& iColumn);	//查询
	void 	sqlite_release_search(char** ppResult);												//释放查询资源	
	int	sqlite_exec(char* pSql);															//执行SQL命令
	char* get_data(char** ppResult, int iColumn, int iX, int iY)	 
	{	
		return *(ppResult + iX * iColumn + iY);		/* x:第多少行,y:第多少列,查询后,取得表内某个单元值 */
	}
	
private:
    sqlite3*	m_pHDb;                      
    int	m_iRow;								/* 行数 */
    int	m_iColumn;							/* 列数 */
    int	m_iCurRow;							/* 当前行数 */	
	string		m_strDbName;
	char**		m_ppResult;
};

#endif
