#include<iostream>
#include<WinSock2.h>
#include<ws2tcpip.h>
#include<thread>
#include<string>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

bool Initialize()
{
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void SendMsg(SOCKET s)
{
	cout << "Enter your chat name: " << endl;
	string Name;
	getline(cin, Name);
	string Message;

	while (1)
	{
		getline(cin, Message);
		string msg = Name + " : " + Message;

		int BytesSent = send(s, msg.c_str(), msg.length(), 0);

		if (BytesSent == SOCKET_ERROR)
		{
			cout << "Error sending message" << endl;
			break;
		}

		if (Message == "Quit")
		{
			cout << "Stopping the application" << endl; 
			break;
		}
	}

	closesocket(s);
	WSACleanup();
}

void ReceivedMessage(SOCKET s)
{
	char Buffer[4096];
	int RecvLength;
	string Message = "";

	while (1)
	{
		RecvLength = recv(s, Buffer, sizeof(Buffer), 0);

		if (RecvLength <= 0)
		{
			cout << "Disconnect from the server..!" << endl;
			break;
		}
		else
		{
			Message = string(Buffer, RecvLength);
			cout << Message << endl;
		}
	}

	closesocket(s);
	WSACleanup();
}

int main()
{
	if (!Initialize())
	{
		cout << "initialize winsock failed..!" << endl; 
		return -1;
	}

	// Creating Socket
	SOCKET s;

	s = socket(AF_INET, SOCK_STREAM, 0);	// Its a socket

	if (s == INVALID_SOCKET)
	{
		cout << "Invalid socket created" << endl;
		return 1;
	}

	int Port = 12345;
	string ServerAddress = "127.0.0.1";

	sockaddr_in ServerAddr;

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	inet_pton(AF_INET, ServerAddress.c_str(), &(ServerAddr.sin_addr));

	if (connect(s, reinterpret_cast<sockaddr*>(&ServerAddr), sizeof(ServerAddr)) == SOCKET_ERROR)
	{
		cout << "Not able to connect server..!" << endl;
		cout << ": " << WSAGetLastError();
		closesocket(s);
		WSACleanup();
		return 1;
	}

	cout << "Successfully connected to server" << endl;

	thread SenderThread(SendMsg, s);
	thread ReceiverThread(ReceivedMessage, s);

	SenderThread.join();
	ReceiverThread.join();


	return 0;
}
