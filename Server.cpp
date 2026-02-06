#include<iostream>
#include<WinSock2.h>		// Socket API
#include<WS2tcpip.h>		// Conversion of IP into binary
#include<tchar.h>
#include<thread>
#include<vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

bool Initialize()
{
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// Send / Received Interactions
void InteractWithClient(SOCKET clientSocket, vector<SOCKET> &Clients)
{
	char buffer[4096];

	cout << "Client connected" << endl;

	while (1)
	{
		int BytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);	// Receive msg from the client

		if (BytesReceived <= 0)
		{
			cout << "Client disconnected..!" << endl; 
			break;
		}

		string Message(buffer, BytesReceived);

		cout << "Message from client: " << Message << endl;

		for (auto client : Clients)
		{
			if (client != clientSocket)
			{
				send(client, Message.c_str(), Message.length(), 0);
			}
			
		}
	}

	auto it = find(Clients.begin(), Clients.end(), clientSocket);

	if (it != Clients.end())
	{
		Clients.erase(it);
	}

	closesocket(clientSocket);
}

int main()
{
	if (!Initialize())
	{
		cout << "winsock initialization failed" << endl;
		return 1;
	}

	cout << "Server Program" << endl;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET)
	{
		cout << "Socket Creation Failed..!" << endl;
		return 1;
	}

	// Create address structure
	int Port = 12345;

	sockaddr_in serveraddr;

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(Port);		// htons = Host to network shot API

	// Convert the IP address (0.0.0.0) put it inside the sin_family in binary format
	if(InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1)
	{
		cout << "Setting-Address structured failed..!" << endl;
		closesocket(listenSocket);		// Close Socket
		WSACleanup();					// Clean up 
		return 1;
	}

	// Bind IP port with the Socket
	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR)
	{
		cout << "Bind failed" << endl;
		closesocket(listenSocket);		
		WSACleanup();					
		return 1;
	}

	// Listen on the socket 
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)	// SOMAXCONN = Amount of clients have in the queue
	{
		cout << "Listen Failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Server has started listening on port: " << Port << endl;
	vector<SOCKET> Clients;

	while (1)
	{
		// Start accepting connection from the client
		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);

		if (clientSocket == INVALID_SOCKET)
		{
			cout << "Invalid client socket" << endl;
		}

		Clients.push_back(clientSocket);

		thread t1(InteractWithClient, clientSocket, std::ref(Clients));

		t1.detach();
	}

	closesocket(listenSocket);


	WSACleanup();

	return 0;
}
