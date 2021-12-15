#pragma comment (lib,"Ws2_32.lib")

#define _CRT_SECURE_NO_WARNINGS

#define CMD_START_SERVER 1000
#define CMD_STOP_SERVER 1001

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <wchar.h>
#include <stdio.h>

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
	WNDCLASS wc = { };
	wc.lpfnWndProc = WinProc;
	wc.hInstance = hInst;
	wc.lpszClassName = WIN_CLASS_NAME;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);

	ATOM mainWin = RegisterClassW(&wc);
	if (mainWin == FALSE) {
		MessageBoxW(NULL, L"Register class error", L"Register class error", MB_OK | MB_ICONSTOP);
		return -1;
	}

	HWND hwnd = CreateWindowExW(0, WIN_CLASS_NAME,
		L"TCP Chat - Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 640, 480, NULL, NULL, hInst, NULL);
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

			StartServer(&hWnd);
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

		FillRect(dc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
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

	serverLog = CreateWindowExW(0, L"Listbox", L"", WS_CHILD | WS_VISIBLE | WS_BORDER, 200, 18, 250, 200, hWnd, NULL, hInst, NULL);

	startServer = CreateWindowExW(0, L"Button", L"Start", WS_CHILD | WS_VISIBLE, 10, 100, 75, 23, hWnd, (HMENU)CMD_START_SERVER, hInst, NULL);
	stopServer = CreateWindowExW(0, L"Button", L"Stop", WS_CHILD | WS_VISIBLE, 115, 100, 75, 23, hWnd, (HMENU)CMD_STOP_SERVER, hInst, NULL);

	EnableWindow(stopServer, FALSE);

	listenSocket = INVALID_SOCKET;



	return 0;
}



DWORD CALLBACK StartServer(LPVOID params) {
	HWND hWnd = *((HWND*)params);

	const size_t MAX_LEN = 100;
	WCHAR str[MAX_LEN];



	WSADATA wsaData;

	int err;

	err = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (err) {

		_snwprintf_s(str, MAX_LEN, L"Startup error: %d", err);
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -10;
	}

	listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (listenSocket == INVALID_SOCKET) {

		_snwprintf_s(str, MAX_LEN, L"Socket error: %d", WSAGetLastError());
		WSACleanup();
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -20;

	}

	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;

	char ip[20];
	LRESULT ipLen = SendMessageA(hIP, WM_GETTEXT, 29, (LPARAM)ip);
	ip[ipLen] = '\0';

	inet_pton(AF_INET, ip, &addr.sin_addr);


	char port[8];
	LRESULT portLen = SendMessageA(hPort, WM_GETTEXT, 7, (LPARAM)port);
	port[portLen] = '\0';

	addr.sin_port = htons(atoi(port));

	err = bind(listenSocket, (SOCKADDR*)&addr, sizeof(addr));

	if (err == SOCKET_ERROR) {

		if (WSAGetLastError() == 10048) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", WSAGetLastError(), L"Address already in use");
		}
		else if (WSAGetLastError() == 10049) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", WSAGetLastError(), L"Cannot assign requested address");
		}
		else if (WSAGetLastError() == 10050) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", WSAGetLastError(), L"Network is down");
		}
		else if (WSAGetLastError() == 10051) {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d %s", WSAGetLastError(), L"Network is unreachable");
		}
		else {

			_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d", WSAGetLastError());
		}

		closesocket(listenSocket);
		WSACleanup();
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -30;

	}

	err = listen(listenSocket, SOMAXCONN);

	if (err == SOCKET_ERROR) {

		_snwprintf_s(str, MAX_LEN, L"Socket bind error: %d", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		SendMessageW(serverLog, LB_ADDSTRING, 0, (LPARAM)str);
		return -40;

	}
	char strartMsg[MAX_LEN];

	_snprintf_s(strartMsg, MAX_LEN, MAX_LEN, "Server starts IP: %s Port: %s", ip, port);

	SendMessageA(serverLog, LB_ADDSTRING, 0, (LPARAM)strartMsg);

	EnableWindow(stopServer, TRUE);
	EnableWindow(startServer, FALSE);

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