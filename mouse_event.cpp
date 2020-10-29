// control_trigger.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <WinSock2.h>
#include <windows.h>
#include <wingdi.h>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 256

#pragma comment(lib,"Ws2_32.lib")
//#pragma comment(lib,"Gdi32")

struct MouseEvent
{
    unsigned __int32 type;
    float x;
    float y;
    __int32 mouse_data;
};

struct KeyboardEvent
{
    unsigned __int32 key;
    bool key_down;
};

int srcleft_ = 0;
int srctop_ = 0;
int srcwidth_ = 0;
int srcheight_ = 0;

double GetMainScreenParam(int& width, int& height, double& iScaleX, double& iScaleY)
{
    HDC hDDC = NULL;
    HDC desktopDc = GetDC(NULL);
    width = GetDeviceCaps(desktopDc, DESKTOPHORZRES);
    height = GetDeviceCaps(desktopDc, DESKTOPVERTRES);
    ReleaseDC(NULL, desktopDc);
    int logicWidth = GetSystemMetrics(SM_CXSCREEN);
    int logicHeight = GetSystemMetrics(SM_CYSCREEN);
    iScaleX = 65535.0 / (width - 1);
    iScaleY = 65535.0 / (height - 1);
    double mainScale = width * 1.0 / logicWidth;

    POINT pt;
    pt.x = srcleft_ + 1;
    pt.y = srctop_ + 1;
    HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

    MONITORINFOEX miex;
    miex.cbSize = sizeof(miex);
    GetMonitorInfo(hMonitor, &miex);
    int cxLogical = (miex.rcMonitor.right - miex.rcMonitor.left);
    int cyLogical = (miex.rcMonitor.bottom - miex.rcMonitor.top);

    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;
    EnumDisplaySettings(miex.szDevice, ENUM_CURRENT_SETTINGS, &dm);
    int cxPhysical = dm.dmPelsWidth;
    int cyPhysical = dm.dmPelsHeight;

    double horzScale = ((double)cxPhysical / (double)cxLogical);
    double vertScale = ((double)cyPhysical / (double)cyLogical);
    double ret = mainScale / horzScale;
    return ret;
}

void HandleMouseEvent(MouseEvent& item)
{
    double iScaleX, iScaleY, iX, iY;
    int iWidth, iHeight;
    DWORD dwX, dwY;
    int mainScreenWidth, mainScreenHeight;
    double rate = GetMainScreenParam(mainScreenWidth, mainScreenHeight, iScaleX, iScaleY);
    iWidth = srcwidth_ > 0 ? srcwidth_ : mainScreenWidth;
    iHeight = srcheight_ > 0 ? srcheight_ : mainScreenHeight;
    INPUT Input = { 0 };
    iX = srcleft_ * iScaleX * rate + item.x * iWidth * iScaleX;
    iY = srctop_ * iScaleY * rate + item.y * iHeight * iScaleY;
    dwX = (DWORD)iX;
    dwY = (DWORD)iY;
    ::ZeroMemory(&Input, sizeof(INPUT));
    Input.type = INPUT_MOUSE;
    Input.mi.dx = dwX;
    Input.mi.dy = dwY;
    switch (item.type)
    {
    case 10002:
    case WM_LBUTTONDOWN:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
        break;
    }
    case 10003:
    case WM_LBUTTONUP:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
        break;
    }

    case WM_MOUSEMOVE:
    case 10001:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        break;
    }

    case WM_RBUTTONDOWN:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN;
        break;
    }
    case WM_RBUTTONUP:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP;
        break;
    }
    case WM_MBUTTONDOWN:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEDOWN;
        break;
    }
    case WM_MBUTTONUP:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MIDDLEUP;
        break;
    }
    case WM_MOUSEWHEEL:
    {
        Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_WHEEL;
        Input.mi.mouseData = item.mouse_data;
        break;
    }
    default:
        break;
    }
    if (::SendInput(1, &Input, sizeof(INPUT)))
    {
        printf("error\n");
        std::cout << GetLastError();
    }
}

// BYTE GetKeyCode(unsigned int keysym)
// {
//     if ((keysym >= 32 && keysym <= 126) ||
//         (keysym >= 160 && keysym <= 255))
//     {
//         SHORT s = VkKeyScan(keysym);
//         BYTE vkCode = LOBYTE(s);
//         return vkCode;
//     }
//     else
//     {
//         if (vkMap.find(keysym) == vkMap.end())
//         {
//             return keysym;
//         }
//         BYTE vkCode = vkMap[keysym];
//         return vkCode;
//     }
// }

// void HandleKeyboardEvent(KeyboardEvent &ev)
// {

//     KEYBDINPUT kb = {0};
//     INPUT input = {0};
//     kb.wVk = static_cast<WORD>(Keymapper::instance().GetKeyCode(ev.key));

//     if (!ev.key_down)
//     {
//         kb.dwFlags = KEYEVENTF_KEYUP;
//     }
//     input.type = INPUT_KEYBOARD;
//     input.ki = kb;
//     SendInput(1, &input, sizeof(input));

//     if (ev.key == VK_RSHIFT || ev.key == VK_LSHIFT)
//     {
//         if (!ev.key_down)
//         {
//             INPUT input[2];
//             ::ZeroMemory(input, sizeof(input));
//             input[0].type = input[1].type = INPUT_KEYBOARD;
//             input[0].ki.wVk = input[1].ki.wVk = ev.key;
//             input[1].ki.dwFlags = KEYEVENTF_KEYUP; // THIS IS IMPORTANT
//             ::SendInput(2, input, sizeof(INPUT));
//         }
//     }
// }
SOCKET Client;

BOOL Connection(char* lpszServerIp, int iServerPort);
void SendMsg(char* pszSend);// 发送数据
UINT Receive_Data(LPVOID lpVoid);//多线程接收数据

BOOL Connection(char* lpszServerIp, int iServerPort)
{
	// 初始化 Winsock 库
	WSADATA wsaData = { 0 };
	::WSAStartup(MAKEWORD(2, 2), &wsaData);
	// 创建流式套接字
	Client = ::socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == Client)
	{
        std::cout << "error1";
		return FALSE;
	}
	// 设置服务端地址和端口信息
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = ::htons(iServerPort);
	addr.sin_addr.S_un.S_addr = ::inet_addr(lpszServerIp);
	// 连接到服务器
	if (0 != ::connect(Client, (sockaddr*)(&addr), sizeof(addr)))
	{
        std::cout << "error2";
		return FALSE;
	}
	// 创建接收数据多线程
	::CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Receive_Data, NULL, NULL, NULL);

	return TRUE;

}

// 发送数据
void SendMsg(char* pszSend)
{
	// 发送数据
	::send(Client, pszSend, ::lstrlen(pszSend), 0);
	printf("[我]%s\n", pszSend);
}

// 接收数据多线程
UINT Receive_Data(LPVOID lpVoid)
{
	char szBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
		// 接收数据
		int iRet = ::recv(Client, szBuf, MAX_PATH, 0);
		if (0 >= iRet)
		{
			continue;
		}
		printf("[她]%s\n", szBuf);
	}
	return 0;
}


int main()
{
    char addr[] = "192.168.252.39";
	if (FALSE == Connection(addr, 12488))
	{
		printf("Connection Error.\n");
	}
	printf("Connection OK.\n");

	// 发送信息
	char szSendBuf[MAX_PATH] = { 0 };
	while (TRUE)
	{
        std::cin >> szSendBuf;
		// 发送数据
		SendMsg(szSendBuf);
	}

	return 0;
    //初始化DLL
    //WSADATA wsaData;
    //WSAStartup(MAKEWORD(2, 2), &wsaData);
    ////创建套接字
    //SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    ////向服务器发起请求
    //sockaddr_in sockAddr;
    //memset(&sockAddr, 0, sizeof(sockAddr)); //每个字节都用0填充
    //sockAddr.sin_family = PF_INET;
    //sockAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //// sockAddr.sin_port = htons(12050);
    //sockAddr.sin_port = htons(12488);
    ////connect(sock, (SOCKADDR*)&sockAddr, sizeof(SOCKADDR));
    ////{
    ////    char bufSend[BUF_SIZE] = { 0 };
    ////    sprintf_s(bufSend, "test");
    ////    send(sock, bufSend, BUF_SIZE, 0);
    ////}
    //while (true)
    //{
    //    //接收服务器传回的数据
    //    char bufRecv[BUF_SIZE] = { 0 };
    //    recv(sock, bufRecv, BUF_SIZE, 0);
    //    //输出接收到的数据
    //    printf("Message form server: %s\n", bufRecv);
    //    char* type = strtok(bufRecv, ",");
    //    char* x = strtok(NULL, ",");
    //    char* y = strtok(NULL, ",");
    //    int type1;
    //    float x1, y1;
    //    type1 = atoi(type);
    //    x1 = atof(x);
    //    y1 = atof(y);
    //    MouseEvent me;
    //    me.type = type1;
    //    me.x = x1;
    //    me.y = y1;
    //    me.mouse_data = 0;
    //    HandleMouseEvent(me);
    //}
    ////关闭套接字
    //closesocket(sock);
    ////终止使用 DLL
    //WSACleanup();
    //system("pause");
    //return 0;
}

/*
build cmd
    g++ mouse_event.cpp -l Gdi32 -l wsock32 -o mouse_event.exe
*/

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
