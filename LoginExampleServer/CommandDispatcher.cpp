#include "stdafx.h"
#include "CommandDispatcher.h"
#include "UserInfoManager.h"

CommandDispatcher::CommandDispatcher()
{
}


CommandDispatcher::~CommandDispatcher()
{
}
void CommandDispatcher::SplitString(const string& s, vector<string>& v, const string& c)
{
	string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}
	if (pos1 != s.length())
		v.push_back(s.substr(pos1));
}
void CommandDispatcher::DispatchCommand(string message, SOCKET client) {
	//解析指令
	vector<string> commands;
	vector<string> parametors;
	SplitString(message, commands, "(");
	if (commands.size() >1)
		SplitString(commands[1],parametors,",");
	//根据不同的指令调用不同的函数
	if (commands[0] == "Login") {
		UserInfoManager manager;
		manager.Login(parametors[0], parametors[1], client);
	}
	else if (commands[0] == "NotifyChangeSpeed") {
		UserInfoManager::NotifyChangeSpeed(parametors[0],message);
	}
	else if (commands[0] == "RequestUserList") {
		UserInfoManager::RequestUserList(parametors[0]);
	}
	else if (commands[0] == "Disconnect") {
		UserInfoManager::Disconnect(client);
	}
}
void CommandDispatcher::SendCommand(string command, vector<string> &parameters, SOCKET client) {
	command += "(";
	if (parameters.size() > 0) {
		for (string param : parameters) {
			command += param + ",";
		}
		command = command.substr(0, command.length() - 1);
	}
	//command += ")";
	send(client, command.c_str(), command.length(), 0);
}