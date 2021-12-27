#pragma comment (lib,"Ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS

#define MAX_MESSAGES 100
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
#include <iomanip>
#include <exception>
#include <list>
#include "ChatMessage.h"

HINSTANCE hInst;

HWND grpEndPoint;
HWND serverLog;

HANDLE mutex = NULL;

HWND hIP;
HWND hPort;
long long mid;
HWND startServer;
HWND stopServer;

SOCKET listenSocket;

std::list <ChatMessage>Messages;

std::list<const char*> users;

LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
DWORD CALLBACK CreateUI(LPVOID);
DWORD CALLBACK StartServer(LPVOID);
DWORD CALLBACK StopServer(LPVOID);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_     PWSTR cmdLine, _In_     int showMode) {
	hInst = hInstance;
	mid = time(NULL);

	const WCHAR WIN_CLASS_NAME[] = L"ServerWindow";

	WNDCLASSW wc = { };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = hInst;
	wc.lpszClassName = WIN_CLASS_NAME;
	wc.hbrBackground = CreateSolidBrush(RGB(0, 136, 204));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1)); //handle to a small icon

	ATOM mainWin = RegisterClassW(&wc);
	if (mainWin == FALSE) {
		MessageBoxW(NULL, L"Register class error", L"Register class error", MB_OK | MB_ICONSTOP);
		return -1;
	}

	HWND hwnd = CreateWindowExW(0, WIN_CLASS_NAME, L"TCP Chat - Server", WS_OVERLAPPEDWINDOW, 50, 50, 640, 480, NULL, NULL, hInst, NULL);
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
		mutex = CreateMutex(NULL, FALSE, NULL);
		if (mutex == 0) {

			MessageBoxA(hWnd, "Mutex is not created", "Mutex is not created", MB_OK | MB_ICONERROR);
			PostQuitMessage(0);
			return 0;
		}

		break;

	case WM_COMMAND: {


		int cmd = LOWORD(wParam);
		int ntf = HIWORD(wParam);

		switch (cmd) {

		case CMD_START_SERVER:
			CreateThread(0, 0, StartServer, &hWnd, 0, 0);
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

	serverLog = CreateWindowExW(0, L"Listbox", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 200, 18, 400, 200, hWnd, NULL, hInst, NULL);

	startServer = CreateWindowExW(0, L"Button", L"Start", WS_CHILD | WS_VISIBLE, 10, 100, 75, 23, hWnd, (HMENU)CMD_START_SERVER, hInst, NULL);
	stopServer = CreateWindowExW(0, L"Button", L"Stop", WS_CHILD | WS_VISIBLE, 115, 100, 75, 23, hWnd, (HMENU)CMD_STOP_SERVER, hInst, NULL);

	EnableWindow(stopServer, FALSE);

	listenSocket = INVALID_SOCKET;



	return 0;
}

DWORD CALLBACK StartServer(LPVOID params) {
	HWND hWnd = *((HWND*)params);




		DWORD resp = WaitForSingleObject(mutex, INFINITE);

		if (resp == WAIT_OBJECT_0) {


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
			char buff[BUFF_LEN + 1];
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

				} while (strlen(buff) == BUFF_LEN); // '\0' - end of data

				//data is sum of chuncks

				ChatMessage MSG;

				int authorization = 0;
				int userExists = 0;


				
				if (data[0] == '\b') {

					

						for (auto i = users.begin(); i != users.end(); i++) {

							if (strlen(*i)==strlen(data)) {

								for (int j = 0; j < strlen(data); j++) {

									if ((*i)[j] == data[j]) {

										userExists++;

									}


								}
								if (userExists == strlen(data)) {

									authorization++;

								}
								userExists = 0;

							}


						}

						if (authorization == 0) {

							char* dataCopy = new char[strlen(data) + 1]; 

							for (int i = 0; i < strlen(data); i++) {

								dataCopy[i] = data[i];
								dataCopy[i + 1] = '\0';
							}
							users.push_back(dataCopy);
							//this was made because data deletes every iteration
							//and I need to store a copy 

							send(acceptSocket, "201", 4, 0);


						}
						else {

							send(acceptSocket, "401", 4, 0);

						}
						authorization = 0;


						


					

				}
			
				else {



					if (strlen(data) == 0) {

						if (Messages.size() > 0) {
							MSG.setId(mid++);
							const char* mst = MSG.fromListToString(Messages);

							send(acceptSocket, mst, strlen(mst) + 1, 0);

						}

					}
					else
					{



						if (MSG.parseString(data)) {
							MSG.setId(mid++);
							Messages.push_back(MSG);

							if (Messages.size() > MAX_MESSAGES) {

								Messages.pop_front();

							}

							const size_t MAX_LOGDATA = 543;
							char logData[MAX_LOGDATA];


							//send message to log

							SendMessageA(serverLog, LB_ADDSTRING, 0, (LPARAM)MSG.toDateString());
							SendMessageA(serverLog, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), NULL);

							//send answer to client - write in socket



							const char* mst = MSG.fromListToString(Messages);

							send(acceptSocket, mst, strlen(mst) + 1, 0);
							//delete[]mst;

						}
						else {


							SendMessageA(serverLog, LB_ADDSTRING, 0, (LPARAM)data);
							send(acceptSocket, "500", 4, 0);
						}

					}
				}
				shutdown(acceptSocket, SD_BOTH);
				closesocket(acceptSocket);

			}


		}
		ReleaseMutex(mutex);
	
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