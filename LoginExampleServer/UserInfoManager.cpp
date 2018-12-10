#include "stdafx.h"
#include "UserInfoManager.h"
#include "DBOperator.h"
#include "CommandDispatcher.h"
vector<UserInfo> UserInfoManager::users;
UserInfoManager::UserInfoManager()
{
}


UserInfoManager::~UserInfoManager()
{
}
void UserInfoManager::Login(string userName, string password, SOCKET connectSocket) {
	
	
	DBOperator * db = DBOperator::GetInstance();
	auto recordset = db->ExecuteQuery("select Top 1 userName from userInfo where userName='" + userName + "' and password='" + password + "'");
	try{
	if (recordset != nullptr&&recordset->MoveNext()==S_OK) {
		//Ӧ�����Ȳ鿴�Ƿ��Ѿ��е�¼��Ϣ������У�����£���ֹ�ظ���¼
		for (auto iter = users.begin(); iter != users.end();iter++) {
			if (iter->userName == userName) {
				if (connectSocket != iter->connectSocket) {
					Logout(userName);
					
				}
				else {
					users.erase(iter);
				}
				break;
			}
		}
		//��¼��¼��Ϣ
		UserInfo info;
		info.connectSocket = connectSocket;
		info.loginState = true;
		info.userName = userName;
		info.loginTime = time(0);
		users.push_back(info);
		//������Ϣ
		/*string command = "LoginResult(Success)";
		send(connectSocket,command.c_str() ,command.length(), 0);*/
		vector < string> params;
		params.push_back("Success");
		CommandDispatcher::SendCommand("LoginResult", params, connectSocket);
		//֪ͨ������ҵ�¼�¼�
		params.clear();
		params.push_back(userName); params.push_back(info.posX); params.push_back(info.posY); params.push_back(info.posZ); 
		params.push_back(info.speedX); params.push_back(info.speedY); params.push_back(info.speedZ);
	//	string commandLine = "AddUser(" + userName +","+ info.posX+","+info.posY+","+info.posZ + "," + info.speedX + "," + info.speedY + "," + info. +")";
		for (auto user : users) {
			if(user.userName!=userName)
				CommandDispatcher::SendCommand("AddUser", params, user.connectSocket);
			//send(user.connectSocket, commandLine.c_str(), commandLine.length(), 0);
		}
		recordset->Close();
	}
	else {
		vector < string> params;
		params.push_back("Failure");
		CommandDispatcher::SendCommand("LoginResult", params, connectSocket);
		//send(connectSocket, "LoginResult(Failure)", strlen("LoginResult(Failure)"), 0);
		if (recordset != nullptr) {
			recordset->Close();
		}
	}
	}
	catch (_com_error e) {
		vector < string> params;
		params.push_back("ServerException");
		CommandDispatcher::SendCommand("LoginResult", params, connectSocket);
//		send(connectSocket, "LoginResult(ServerException)", strlen("LoginResult(ServerException)"), 0);
		printf("com error:%s",(const char*)e.ErrorMessage());
	}
	db->Release();
}

// �˳���Ϸ�������ӶϿ������µ�¼ʱӦ��ִ��
void UserInfoManager::Logout(string userName)
{
	for (auto iter = users.begin(); iter != users.end(); iter++) {
		if ((*iter).userName == userName) {
			//�Ͽ�����
			closesocket(iter->connectSocket);
			//�������ݿ��¼�����ԣ�
			users.erase(iter);
			break;
		}
	}
	//֪ͨ��������˳��¼�
	//string commandLine = "Logout(" + userName + ")";
	vector < string> params;
	params.push_back(userName);
	for (auto user : users) {
		CommandDispatcher::SendCommand("Logout", params, user.connectSocket);
//		send(user.connectSocket, commandLine.c_str(), commandLine.length(), 0);
	}
}
//ͬ�����λ��
void UserInfoManager::NotifyChangeSpeed(string userName, string commandLine) {
	for (int i = 0; i < users.size();i++) {
		if (users[i].userName != userName) {
			send(users[i].connectSocket, commandLine.c_str(), commandLine.length(), 0);
		}
		else {
			//���µ�ǰ���λ�ã������Ժ����Һ����û����λ��ͬ��
			vector<string> params;
			CommandDispatcher::SplitString(commandLine, params, ",");
			users[i].speedX = params[1];
			users[i].speedY = params[2];
			users[i].speedZ = params[3];
			users[i].posX = params[4];
			users[i].posY = params[5];
			users[i].posZ = params[6];
		}
	}
}
	void UserInfoManager::RequestUserList(string userName) {
		//�ҵ���ǰ��Ҽ�¼
		UserInfo info;
		for (auto user : users) {
			if (user.userName == userName) {
				info = user;
				break;
			}
		}
		if (info.userName != "") {
			vector<string> params;
			for (auto user : users) {
				if (user.userName != userName) {
					params.clear();
					params.push_back(user.userName); params.push_back(user.posX); params.push_back(user.posY); params.push_back(user.posZ);
					params.push_back(user.speedX); params.push_back(user.speedY); params.push_back(user.speedZ);
					//string commandLine = "AddUser(" + user.userName + "," + user.posX + "," + 
					//	user.posY + "," + user.posZ + "," + user.speedX + "," + user.speedY + "," + user.speedZ /*+ ")"*/;
					//send(info.connectSocket, commandLine.c_str(), commandLine.length(), 0);
					CommandDispatcher::SendCommand("AddUser", params, info.connectSocket);
				}
			}
					
		}
	}



	// �ͻ��˶Ͽ�����
	void UserInfoManager::Disconnect(SOCKET clientSocket)
	{
		string userFind ="";
		for (auto user : users) {
			if (user.connectSocket == clientSocket) {
				userFind = user.userName;
				break;
			}
		}
		//����û��Ѿ���¼�����˳�������ֱ�ӹر��׽���
		if (userFind != "")
			Logout(userFind);
		else
			closesocket(clientSocket);
	}
