#pragma once
#include <vector>
#include <string>
using namespace std;

#import "c:\program files\common files\system\ado\msado15.dll"  no_namespace rename("EOF", "adoEOF")
#define MAX_CONNECTION_NUM 50
class DBOperator
{
private:
	static vector<DBOperator*> connectionPool;
	_ConnectionPtr  connection;
	bool isBusy=false;
	DBOperator();
public:
	static DBOperator * GetInstance();
	void Release() { isBusy = false; };
	_RecordsetPtr ExecuteQuery(string sql);
	~DBOperator();
	// 执行更新语句，返回更新的行数，若出现异常，则返回-1
	int ExcuteUpdate(string sql);
	void Connect();
};

