#pragma comment (lib,"Ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS

#define CMD_START_SERVER 1000
#define CMD_STOP_SERVER 1001
#include "resource.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <wchar.h>
#include <stdio.h>
#include <time.h>
#include <string>

HINSTANCE hInst;

HWND grpEndPoint;
HWND serverLog;

HWND hIP;
HWND hPort;

HWND startServer;
HWND stopServer;

SOCKET listenSocket;

LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
DWORD CALLBACK CreateUI(LPVOID);
DWORD CALLBACK StartServer(LPVOID);
DWORD CALLBACK StopServer(LPVOID);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_     PWSTR cmdLine, _In_     int showMode) {
	hInst = hInstance;


	const WCHAR WIN_CLASS_NAME[] = L"ServerWindow";

	WNDCLASSW wc = { };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = hInst;
	wc.lpszClassName = WIN_CLASS_NAME;
	//wc.hbrBackground = CreateSolidBrush(RGB(0,136,204));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1)); //handle to a small icon

	ATOM mainWin = RegisterClassW(&wc);
	if (mainWin == FALSE) {
		MessageBoxW(NULL, L"Register class error", L"Register class error", MB_OK | MB_ICONSTOP);
		return -1;
	}

	HWND hwnd = CreateWindowExW(0, WIN_CLASS_NAME, L"TCP Chat - Server", WS_OVERLAPPEDWINDOW,50,50, 640, 480, NULL, NULL, hInst, NULL);
	if (hwnd == NULL) {
		MessageBoxW(NULL, L"Window create error", L"Window create error", MB_OK | MB_ICONSTOP);
		return -2;
	}

	ShowWindow(hwnd, showMode);
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE:

		CreateUI(&hWnd);
		break;

	case WM_COMMAND: {


		int cmd = LOWORD(wParam);
		int ntf = HIWORD(wParam);

		switch (cmd) {

		case CMD_START_SERVER:
			CreateThread(0, 0, StartServer,&hWnd,0,0);
			break;

		case CMD_STOP_SERVER:

			StopServer(&hWnd);

			break;
		}

		break;
	}

	case WM_PAINT: {

		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hWnd, &ps);

		FillRect(dc, &ps.rcPaint, CreateSolidBrush(RGB(0, 136, 204)));
		EndPaint(hWnd, &ps);

		break;
	}
	case WM_CTLCOLORSTATIC: {


		HDC dc = (HDC)wParam;
		HWND ctl = (HWND)lParam;

		if (ctl != grpEndPoint) {

			SetBkMode(dc, TRANSPARENT);
			SetTextColor(dc, RGB(20, 20, 50));

		}

		return (LRESULT)GetStockObject(NULL_BRUSH);
		break;
	}
	case WM_CTLCOLORLISTBOX: {

		HDC dc = (HDC)wParam;
		HWND ctl = (HWND)lParam;

	
		SetBkColor(dc, RGB(0, 136, 204));

		return (LRESULT)GetStockObject(NULL_BRUSH);
		break;
	}
	case WM_CTLCOLOREDIT: {

		HDC dc = (HDC)wParam;
		HWND ctl = (HWND)lParam;

	
		SetBkColor(dc, RGB(0, 136, 204));

		return (LRESULT)GetStockObject(NULL_BRUSH);
		break;
	}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}


DWORD CALLBACK CreateUI(LPVOID params) {

	HWND hWnd = *((HWND*)params);


	grpEndPoint = CreateWindowExW(0, L"Button", L"EndPoint", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 10, 180, 80, hWnd, NULL, hInst, NULL);

	CreateWindowExW(0, L"Static", L"IP:", WS_CHILD | WS_VISIBLE, 20, 30, 40, 15, hWnd, NULL, hInst, NULL);
	hIP = CreateWindowExW(0, L"Edit", L"127.0.0.1", WS_CHILD | WS_VISIBLE, 60, 30, 120, 17, hWnd, NULL, hInst, NULL);

	CreateWindowExW(0, L"Static", L"Port:", WS_CHILD | WS_VISIBLE, 20, 50, 40, 15, hWnd, NULL, hInst, NULL);
	hPort = CreateWindowExW(0, L"Edit", L"8888", WS_CHILD | WS_VISIBLE, 60, 50, 120, 17, hWnd, NULL, hInst, NULL);

	serverLog = CreateWindowExW(0, L"Listbox", L"", WS_CHILD | WS_VISIBLE | WS_BORDER , 200, 18, 400, 200, hWnd, NULL, hInst, NULL);

	startServer = CreateWindowExW(0, L"Button", L"Start", WS_CHILD | WS_VISIBLE, 10, 100, 75, 23, hWnd, (HMENU)CMD_START_SERVER, hInst, NULL);
	stopServer = CreateWindowExW(0, L"Button", L"Stop", WS_CHILD | WS_VISIBLE, 115, 100, 75, 23, hWnd, (HMENU)CMD_STOP_SERVER, hInst, NULL);

	EnableWindow(stopServer, FALSE);

	listenSocket = INVALID_SOCKET;



	return 0;
}



std::string* splitString(std::string str, char sym) {

	size_t pos = 0;
	int parts = 1;

	while ((pos = str.find(sym, pos + 1)) != std::string::npos) {
		parts++;
	}

	std::string* res = new std::string[parts];
	pos = 0;
	size_t pos2;

	for (int i = 0; i < parts - 1; i++) {

		pos2 = str.find(sym, pos + 1);
		res[i] = str.substr(pos, pos2 - pos);
		pos = pos2;


	}

	res[parts - 1] = str.substr(pos + 1);


	if (parts == 1) {

	}



	return res;
}



//struct USER {
//
//public:
//	char name[30];
//	char message[512];
//	char time[6];
//
//
//
//};






DWORD CALLBACK StartServer(LPVOID params) {
	HWND hWnd = *((HWND*)params);

	const size_t MAX_LEN = 100;
	WCHAR str[MAX_LEN];


	WSADATA wsaData;
	int err;

	//WinSock API initializing ( ~wsaData = new WSA(2.2) )
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err) {

		_snwprintf_s(str, MAX_LEN, L"Startup error: %d", err);
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -10;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	//Socket preparing
	if (listenSocket == INVALID_SOCKET) {

		_snwprintf_s(str, MAX_LEN, L"Socket error: %d", WSAGetLastError());
		WSACleanup();
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -20;

	}
	//----Socket configuration------
	SOCKADDR_IN addr; //Config structure

	addr.sin_family = AF_INET; // 1.Network type (family)

	char ip[20];
	LRESULT ipLen = SendMessageA(hIP, WM_GETTEXT, 29, (LPARAM)ip);
	ip[ipLen] = '\0';

	inet_pton(AF_INET, ip, &addr.sin_addr); //2.IP


	char port[8];
	LRESULT portLen = SendMessageA(hPort, WM_GETTEXT, 7, (LPARAM)port);
	port[portLen] = '\0';

	addr.sin_port = htons(atoi(port)); //3.Port
	//----end configuration------

	//Socket binding - config [addr] applying to socket
	err = bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr));
	
	if (err == SOCKET_ERROR) {

		int lastError = WSAGetLastError();

		if (lastError == 10048) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", lastError, L"Address already in use");
		}
		else if (lastError == 10049) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", lastError, L"Cannot assign requested address");
		}
		else if (lastError == 10050) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", lastError, L"Network is down");
		}
		else if (lastError == 10051) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", lastError, L"Network is unreachable");
		}
		else {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d", lastError);
		}

		closesocket(listenSocket);
		WSACleanup();
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -30;

	}
	
	//Start of listening - from this point Socket recives data form OS
	err = listen(listenSocket, SOMAXCONN);


	if (err == SOCKET_ERROR) {

		_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -40;

	}
	//Log start message
	char strartMsg[MAX_LEN];
	_snprintf_s(strartMsg, MAX_LEN, MAX_LEN, "Server starts IP: %s Port: %s", ip, port);
	SendMessageA(serverLog, LB_ADDSTRING, 0, (LPARAM)strartMsg);
	EnableWindow(stopServer, TRUE);
	EnableWindow(startServer, FALSE);

	//listening loop
	SOCKET acceptSocket; //second socket - for communication

	const size_t BUFF_LEN = 8;
	const size_t DATA_LEN = 2048;
	char buff[BUFF_LEN+1];
	char data[DATA_LEN]; // big buffer for all transfered chunks
	int receivedCnt; //chunck size

	while (true) {
		// wait for network activity
		acceptSocket = accept(listenSocket, NULL, NULL);
		if (acceptSocket == INVALID_SOCKET) {

			_snwprintf_s(str, MAX_LEN, L"Accept socket error: %d", WSAGetLastError());
			SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
			closesocket(acceptSocket);
			return -60;
		}
		//communication begins
		data[0] = '\0';
		do {
			receivedCnt = recv(acceptSocket, buff, BUFF_LEN, 0);

			if (receivedCnt == 0) { // 0 - connection closed by client

				closesocket(acceptSocket);
				SendMessageA(serverLog, LB_ADDSTRING, 0, (LPARAM)"Connection closed");
				break;
			}

			if (receivedCnt < 0) { //receiving error

				closesocket(acceptSocket);
				_snwprintf_s(str, MAX_LEN, L"Communication socket error: %d", WSAGetLastError());
				SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
				break;

			}

			buff[receivedCnt] = '\0';
			strcat_s(data, buff); //data+= chunk (buff)

		} while (strlen(buff)==BUFF_LEN); // '\0' - end of data
		
		//data is sum of chuncks

		std::string* userData = splitString(data, '\t');
		
		//USER user;
		//user.name = userData[0].c_str();

		char name[30];
		char message[512];
		strcpy_s(name, userData[0].c_str());
		strcpy_s(message, userData[1].c_str());


		SYSTEMTIME  time;
		GetLocalTime(&time);


		const size_t MAX_LOGDATA = 543;


		char logData[MAX_LOGDATA];

		_snprintf_s(logData, MAX_LOGDATA, MAX_LOGDATA, "%s %d:%d", data, time.wHour, time.wMinute);


		SendMessageA(serverLog, LB_ADDSTRING, 0, (LPARAM)logData);

		//send answer to client - write in socket

		send(acceptSocket,"200",4,0);

		shutdown(acceptSocket,SD_BOTH);
		closesocket(acceptSocket);

	}




	return 0;
}

DWORD CALLBACK StopServer(LPVOID params) {
	HWND hWnd = *((HWND*)params);

	closesocket(listenSocket);
	WSACleanup();
	listenSocket = INVALID_SOCKET;


	SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)L"Server stops");

	EnableWindow(stopServer, FALSE);
	EnableWindow(startServer, TRUE);

	return 0;
}