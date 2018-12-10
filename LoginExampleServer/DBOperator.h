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
	// ִ�и�����䣬���ظ��µ��������������쳣���򷵻�-1
	int ExcuteUpdate(string sql);
	void Connect();
};

