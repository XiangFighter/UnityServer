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
	// �˳���Ϸ�������ӶϿ������µ�¼ʱӦ��ִ��
	static void Logout(string userName);
	//ͬ�����λ��
	static void NotifyChangeSpeed(string userName, string commandLine);
	//��������б�
	static void RequestUserList(string userName);
	// �ͻ��˶Ͽ�����
	static void Disconnect(SOCKET clientSocket);
};

