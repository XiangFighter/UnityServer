#pragma once
#include <WinSock2.h>
#include <string>
#include <time.h>
using namespace std;
class UserInfo
{
public:
	UserInfo();
	~UserInfo();
	SOCKET connectSocket;
	string userName;
	bool loginState;
	time_t loginTime;
	string posX, posY, posZ;//��ǰλ�ã�ʹ��string��������ת��
	string speedX, speedY, speedZ;//��ǰ�ٶ�
};

