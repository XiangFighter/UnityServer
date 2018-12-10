#include "stdafx.h"
#include "DBOperator.h"

vector<DBOperator*> DBOperator::connectionPool;

DBOperator::DBOperator()
{
	Connect();
}


DBOperator::~DBOperator()
{
	connection->Close();
}
DBOperator * DBOperator::GetInstance() {
	for (int i = 0; i < connectionPool.size(); i++) {
		if (!connectionPool[i]->isBusy) {
			connectionPool[i]->isBusy = true;
			return connectionPool[i];
		}
	}
	if (connectionPool.size() < MAX_CONNECTION_NUM) {
		DBOperator * newConnection = new DBOperator();
		connectionPool.push_back(newConnection);
		newConnection->isBusy = true;
		return newConnection;
	}
	return nullptr;
}
_RecordsetPtr DBOperator::ExecuteQuery(string sql) {
	try {
		//������ܳ��ֵ������ж����
		if (connection->GetState() != adStateOpen) {
			Connect();
		}
		_RecordsetPtr record;
		if (FAILED(record.CreateInstance(_uuidof(Recordset))))
		{
			printf("��¼������ָ��ʵ����ʧ�ܣ�");
			return nullptr;
		}
		record->Open(sql.c_str(), (IDispatch*)connection, adOpenDynamic, adLockOptimistic, adCmdText);
		return record;
	}
	catch (_com_error e) {
		printf((const char*)e.ErrorMessage());
		return nullptr;
	}
}

// ִ�и�����䣬���ظ��µ��������������쳣���򷵻�-1
int DBOperator::ExcuteUpdate(string sql)
{
	try {
		//������ܳ��ֵ������ж����
		if (connection->GetState() != adStateOpen) {
			Connect();
		}
		VARIANT affected;
		connection->Execute(sql.c_str(), &affected, adOptionUnspecified);
		return affected.intVal;
	}
	catch (_com_error e) {
		printf(e.Description());
		return -1;
	}
}


void DBOperator::Connect()
{
	try {
		CoInitialize(nullptr);
		HRESULT hr = connection.CreateInstance(_uuidof(Connection));

		if (FAILED(hr))
		{
			printf("_ConnectionPtr����ָ��ʵ����ʧ�ܣ�����");
			return;
		}
		_bstr_t strConnect = "Driver={sql server};server=.;uid=sa;pwd=123456;database=LoginExampleDB;";
		connection->Open(strConnect, "", "", adModeUnknown);
	}
	catch (_com_error &e)
	{
		printf(e.Description());
		return;
	}
}
