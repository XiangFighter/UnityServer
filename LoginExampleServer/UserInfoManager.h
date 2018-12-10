#pragma once
#include "UserInfo.h"
#include <vector>
using namespace std;
class UserInfoManager
{
private:
	static vector<UserInfo> users;
public:
	UserInfoManager();
	~UserInfoManager();
	static void Login(string userName, string password, SOCKET connectSocket);
	// 退出游戏，当连接断开、重新登录时应该执行
	static void Logout(string userName);
	//同步玩家位置
	static void NotifyChangeSpeed(string userName, string commandLine);
	//请求玩家列表
	static void RequestUserList(string userName);
	// 客户端断开连接
	static void Disconnect(SOCKET clientSocket);
};

