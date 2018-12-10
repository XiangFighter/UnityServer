#pragma once
#include <string>
#include <vector>
#include <winsock2.h>
using namespace std;
class CommandDispatcher
{
public:
	CommandDispatcher();
	~CommandDispatcher();
	static void SplitString(const string& s, vector<string>& v, const string& c);
	static void DispatchCommand(string message,SOCKET client);
	static void SendCommand(string command,vector<string> & parameters, SOCKET client);
};

