#pragma comment (lib,"Ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS

#define CMD_SEND_MESSAGE 1000
#define CMD_DDOS 1001

#include "resource.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <wchar.h>
#include <stdio.h>
#include <list>
#include "../Server/ChatMessage.h"

HINSTANCE hInst;

HWND chatLog;

HWND sendMessage;
HWND hPort;
HWND hIP;
HWND DDOS;
HWND hName;
HWND grpEndPoint;
HWND editMessage;

bool ddosFlag=false;

std::list<ChatMessage> Messages;

SOCKET clientSocket;


LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);
DWORD CALLBACK CreateUI(LPVOID);
DWORD CALLBACK SendChatMessage(LPVOID);

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_     PWSTR cmdLine, _In_     int showMode) {
	hInst = hInstance;

	const WCHAR WIN_CLASS_NAME[] = L"ClientWindow";
	WNDCLASS wc = { };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = hInst;
	wc.lpszClassName = WIN_CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));

	ATOM mainWin = RegisterClassW(&wc);
	if (mainWin == FALSE) {
		MessageBoxW(NULL, L"Register class error", L"Register class error", MB_OK | MB_ICONSTOP);
		return -1;
	}

	HWND hwnd = CreateWindowExW(0, WIN_CLASS_NAME, L"TCP Chat - Client", WS_OVERLAPPEDWINDOW, 700, 50, 640, 480, NULL, NULL, hInst, NULL);
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

		case CMD_SEND_MESSAGE:


			CreateThread(0, 0, SendChatMessage, &hWnd, 0, 0);
			break;
		case CMD_DDOS:

			ddosFlag ? ddosFlag = false : ddosFlag = true;

			if (ddosFlag) {

				SetTimer(hWnd,CMD_DDOS,1,NULL);

			}
			else {
				KillTimer(hWnd, CMD_DDOS);
			}

			break;

		}

		break;
	}


	case WM_TIMER:

		if (wParam == CMD_DDOS) {

			SendChatMessage(&hWnd);

		}

		break;
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
			SetBkColor(dc, RGB(0, 136, 204));
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

	chatLog = CreateWindowExW(0, L"Listbox", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 200, 18, 400, 200, hWnd, NULL, hInst, NULL);

	sendMessage = CreateWindowExW(0, L"Button", L"Send", WS_CHILD | WS_VISIBLE, 10, 190, 75, 23, hWnd, (HMENU)CMD_SEND_MESSAGE, hInst, NULL);
	
	DDOS = CreateWindowExW(0, L"Button", L"DDOS", WS_CHILD | WS_VISIBLE, 115, 190, 75, 23, hWnd, (HMENU)CMD_DDOS, hInst, NULL);

	editMessage = CreateWindowExW(0, L"Edit", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 100, 180, 50, hWnd, NULL, hInst, NULL);

	hName = CreateWindowExW(0, L"Edit", L"Danya", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 160, 180, 23, hWnd, NULL, hInst, NULL);

	return 0;
}

DWORD CALLBACK SendChatMessage(LPVOID params) {

	HWND hWnd = *((HWND*)params);

	const size_t MAX_LEN = 100;
	WCHAR str[MAX_LEN];


	WSADATA wsaData;
	int err;

	//WinSock API initializing ( ~wsaData = new WSA(2.2) )
	err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err) {

		_snwprintf_s(str, MAX_LEN, L"Startup error: %d", err);
		SendMessageW(chatLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -10;
	}

	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Socket preparing
	if (clientSocket == INVALID_SOCKET) {

		_snwprintf_s(str, MAX_LEN, L"Socket error: %d", WSAGetLastError());
		WSACleanup();
		SendMessageW(chatLog, LB_ADDSTRING, 0, (LPARAM)str);
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

	//Connect to server
	err = connect(clientSocket, (SOCKADDR*)&addr, sizeof(addr));

	if (err == SOCKET_ERROR) {

		int lastError = WSAGetLastError();

		if (lastError == 10048) {

			_snwprintf_s(str, MAX_LEN, L"Socket connect error: %d %s", lastError, L"Address already in use");
		}
		else if (lastError == 10049) {

			_snwprintf_s(str, MAX_LEN, L"Socket connect error: %d %s", lastError, L"Cannot assign requested address");
		}
		else if (lastError == 10050) {

			_snwprintf_s(str, MAX_LEN, L"Socket connect error: %d %s", lastError, L"Network is down");
		}
		else if (lastError == 10051) {

			_snwprintf_s(str, MAX_LEN, L"Socket connect error: %d %s", lastError, L"Network is unreachable");
		}
		else {

			_snwprintf_s(str, MAX_LEN, L"Socket connect error: %d", lastError);
		}

		closesocket(clientSocket);
		WSACleanup();
		SendMessageW(chatLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -30;

	}

	const size_t EDITMSG_LEN = 512;

	char editMsg[EDITMSG_LEN];
	SendMessageA(editMessage, WM_GETTEXT, EDITMSG_LEN, (LPARAM)editMsg);

	char name[30];
	SendMessageA(hName, WM_GETTEXT, 30, (LPARAM)name);

	const size_t MSG_LEN = 542;
	char message[542];

	_snprintf_s(message, MSG_LEN, MSG_LEN, "%s\t%s", name, editMsg);

	int sent = send(clientSocket, message, MSG_LEN + 1, 0);



	if (sent == SOCKET_ERROR) {

		_snwprintf_s(str, MAX_LEN, L"Sending error: %d %s", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		clientSocket = INVALID_SOCKET;
		SendMessageW(chatLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -40;
	}


	int receveCnt = recv(clientSocket, message, MSG_LEN, 0);


	if (receveCnt > 0) {

		

		ChatMessage MSG;
		if (MSG.parseStringDT(message)) {

			Messages.push_back(MSG);

			SendMessageA(chatLog, LB_ADDSTRING, 0, (LPARAM)MSG.toDateString());

		}


	}

	shutdown(clientSocket, SD_BOTH);
	closesocket(clientSocket);
	WSACleanup();

	//SendMessageW(chatLog, LB_ADDSTRING, 0, (LPARAM)L"-End-");
	return 0;
}