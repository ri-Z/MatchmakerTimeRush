#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <list>
#include <string>
#include <vector>
#include <memory>

using namespace std;
//ws2_32.lib
#define MAXPENDING 5
#define RCVBUFSIZE 1024

typedef struct playerinfo {
	SOCKET client;
	int id;
} PlayerInfo;

typedef struct sessioninfo {
	int id;
	string name;
	string serverip;
	int serverport;
} SessionInfo;

list<PlayerInfo> players;
list<SessionInfo> sessions;
list<SessionInfo> twpsessions;
list<SessionInfo> frpsessions;
int frpPlayers;
list<SessionInfo> egpPsessions;
int egPlayers;

int playercount = 0;
int sessioncount = 0;
string path = "/c C:/Users/timerush/Desktop/WindowsNoEditor/TimeRush/Binaries/Win64/TimeRushServer.exe";
string LevelEntry = " /Game/Maps/FinalLevel";

string port = " -PORT =";
string Alog = " -log";
SessionInfo get(list<SessionInfo> _list, int _i);
list<SessionInfo>::iterator getIteratorS(list<SessionInfo> _list, int _i);
list<PlayerInfo>::iterator getIteratorP(list<PlayerInfo> _list, int _i);

void InterpreteMessage(char* buffer, PlayerInfo pInfo)
{
	string temp;
	vector<string> params;
	char cmd = 0;

	for (int i = 0; buffer[i] != '#'; i++)
	{
		if ((buffer[i] == '|') && (cmd == 0))
		{
			cmd = temp[0];
			temp = "";
		}
		else if ((buffer[i] == '|') && (cmd != 0))
		{
			params.push_back(temp);
			temp = "";
		}
		else
		{
			temp = temp + buffer[i];
		}
	}
	if (cmd == 'g')
	{
		string message = "s|null|";
		if (send(pInfo.client, message.c_str(),	message.length(), 0) == SOCKET_ERROR)
		{
			cout << "send failed!" << endl;
		}
	}
	else if (cmd == 'h')
	{
		if (stoi(params.at(3)) == 4) {
			if (frpsessions.size() > 0)
			{
				frpPlayers++;
				SessionInfo s;
				for (list<SessionInfo>::iterator it = frpsessions.begin();
					it != frpsessions.end(); it++)
				{
					s = *it;
					string message = "s|" + to_string(it->id) + "|"
						+ it->name + "|" + it->serverip + "|"
						+ to_string(it->serverport) + "|" + to_string(frpsessions.size() + 1) + "|";
					cout << message << endl;
					if (send(pInfo.client, message.c_str(), message.length(), 0) == SOCKET_ERROR)
					{
						cout << "send failed!" << endl;
					}
				}
				if (frpPlayers == 4) {
					sessions.push_back(frpsessions.front());
					frpsessions.clear();
					frpPlayers = 0;
				}
			}
			else {
				SessionInfo session;
				session.id = pInfo.id;
				session.name = params.at(0);
				session.serverip = params.at(1);
				session.serverport = stoi(params.at(2)) + sessioncount;
				sessioncount++;
				frpsessions.push_back(session);
				string Nport = port + to_string(session.serverport);
				string fullpath = "";
				if (stoi(params.at(4)) == 0) {
					fullpath = (path + LevelEntry + Nport + Alog);
				}
				ShellExecute(0, "open", "cmd.exe", fullpath.c_str(), 0, SW_NORMAL);
				//Sleep(2000);
				string message = "o|" + to_string(session.serverport) + "|";
				cout << message << endl;

				if (send(pInfo.client, message.c_str(),
					message.length(), 0) == SOCKET_ERROR)
				{
					cout << "send failed!" << endl;
				}
			}
		}
	}
	else
	{
		cout << "Unknown message: " << buffer << endl;
	}
}

//g|#
//h|SESSION_NAME|SESSION_IP|SESSION_PORT|MAXNUM_PLAYERS|MAP#

//s|SESSION_ID|SESSION_NAME|SESSION_IP|SESSION_PORT|CUNRNUM_PLAYERS
//o|SESSION_PORT|
void HandleClientThread(PlayerInfo pInfo)
{
	char buffer[RCVBUFSIZE];

	while (recv(pInfo.client, buffer, sizeof(buffer), 0) > 0)
	{
		cout << buffer << endl;
		InterpreteMessage(buffer, pInfo);
		memset(buffer, 0, sizeof(buffer));
	}

	if (closesocket(pInfo.client) == SOCKET_ERROR)
	{
		cout << "closesocket() failed" << endl;
	}
}

int main()
{
	SOCKET server;
	SOCKADDR_IN server_addr, client_addr;
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != NO_ERROR)
	{
		cout << "WSAStartup() failed" << endl;
		exit(EXIT_FAILURE);
	}
	if ((server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		cout << "socket() failed" << endl;
		exit(EXIT_FAILURE);
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(8856);

	if (::bind(server, (struct sockaddr*) & server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		cout << "bind() failed" << endl;
		exit(EXIT_FAILURE);
	}

	if (listen(server, MAXPENDING) == SOCKET_ERROR)
	{
		cout << "listen() failed" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Server Started!" << endl;

	while (true)
	{
		SOCKET client;
		int clientlen = sizeof(client_addr);

		if ((client = accept(server, (struct sockaddr*) & client_addr, &clientlen)) == INVALID_SOCKET)
		{
			cout << "accept() failed" << endl;
		}
		else {

			char addrstr[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client_addr.sin_addr), addrstr, INET_ADDRSTRLEN);
			cout << "Connection from " << addrstr << endl;

			PlayerInfo pInfo;
			pInfo.client = client;
			pInfo.id = playercount++;
			players.push_back(pInfo);

			thread* clientthread = new std::thread(HandleClientThread, pInfo);
		}
	}
}

SessionInfo get(list<SessionInfo> _list, int _i) 
{
	list<SessionInfo>::iterator it = _list.begin();
	for (int i = 0; i < _i; i++) 
	{
		++it;
	}
	return *it;
}

list<SessionInfo>::iterator getIteratorS(list<SessionInfo> _list, int _i)
{
	list<SessionInfo>::iterator it = _list.begin();
	for (int i = 0; i < _i; i++) 
	{
		++it;
	}
	return it;
}

list<PlayerInfo>::iterator getIteratorP(list<PlayerInfo> _list, int _i)
{
	list<PlayerInfo>::iterator it = _list.begin();
	for (int i = 0; i < _i; i++) 
	{
		++it;
	}
	return it;
}