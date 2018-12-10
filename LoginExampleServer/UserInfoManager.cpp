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
		//应该首先查看是否已经有登录信息，如果有，则更新，防止重复登录
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
		//记录登录信息
		UserInfo info;
		info.connectSocket = connectSocket;
		info.loginState = true;
		info.userName = userName;
		info.loginTime = time(0);
		users.push_back(info);
		//返回消息
		/*string command = "LoginResult(Success)";
		send(connectSocket,command.c_str() ,command.length(), 0);*/
		vector < string> params;
		params.push_back("Success");
		CommandDispatcher::SendCommand("LoginResult", params, connectSocket);
		//通知其它玩家登录事件
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

// 退出游戏，当连接断开、重新登录时应该执行
void UserInfoManager::Logout(string userName)
{
	for (auto iter = users.begin(); iter != users.end(); iter++) {
		if ((*iter).userName == userName) {
			//断开连接
			closesocket(iter->connectSocket);
			//保存数据库记录（暂略）
			users.erase(iter);
			break;
		}
	}
	//通知其它玩家退出事件
	//string commandLine = "Logout(" + userName + ")";
	vector < string> params;
	params.push_back(userName);
	for (auto user : users) {
		CommandDispatcher::SendCommand("Logout", params, user.connectSocket);
//		send(user.connectSocket, commandLine.c_str(), commandLine.length(), 0);
	}
}
//同步玩家位置
void UserInfoManager::NotifyChangeSpeed(string userName, string commandLine) {
	for (int i = 0; i < users.size();i++) {
		if (users[i].userName != userName) {
			send(users[i].connectSocket, commandLine.c_str(), commandLine.length(), 0);
		}
		else {
			//更新当前玩家位置，方便以后防外挂和新用户玩家位置同步
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
		//找到当前玩家记录
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



	// 客户端断开连接
	void UserInfoManager::Disconnect(SOCKET clientSocket)
	{
		string userFind ="";
		for (auto user : users) {
			if (user.connectSocket == clientSocket) {
				userFind = user.userName;
				break;
			}
		}
		//如果用户已经登录，则退出，否则直接关闭套接字
		if (userFind != "")
			Logout(userFind);
		else
			closesocket(clientSocket);
	}
