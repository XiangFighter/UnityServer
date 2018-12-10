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
	string posX, posY, posZ;//当前位置，使用string减少类型转换
	string speedX, speedY, speedZ;//当前速度
};

