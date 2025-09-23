#ifndef UNICODE
#define UNICODE
#endif

#include <iostream>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <winsock2.h>
#include <thread>
#include <windows.h>
#include <ws2tcpip.h>
#include <string>
#include <cstring>

#undef MB_SYSTEMMODAL
#undef HWND_TOPMOST
#define HWND_TOPMOST HWND_NOTOPMOST
#define MB_SYSTEMMODAL 0
#define BEEN_KICK (WM_USER+1)
using namespace std;
const int MS=65536;
SOCKET clientSock;
HFONT fontB,fontT;
HWND Main,ConSerB,setNameB,SendB;
HWND setIPT,setRT,setNameT,talkT,writeT;
HINSTANCE hIns;
string IP;
template<typename... Args>
void removeWindows(Args&...windows){
    ((DestroyWindow(windows),windows=NULL),...);
}
void flashWin(HWND hwnd) {
	if(IsIconic(hwnd)) FlashWindow(hwnd,TRUE);
	else if(hwnd!=GetForegroundWindow()) FlashWindow(hwnd,TRUE);
}

void removeWin(HWND &a){ DestroyWindow(a);a=NULL; }

void CreateButton(HWND hwnd,HINSTANCE hInstance) {
	LOGFONT lf = {};
	lf.lfHeight = -MulDiv(20,GetDeviceCaps(GetDC(NULL),LOGPIXELSY),140);
	lf.lfWeight = FW_NORMAL;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	wcscpy(lf.lfFaceName,L"Consolas");
	fontB = CreateFontIndirect(&lf);
	ConSerB=CreateWindow(L"Button",L"Connect",WS_CHILD|WS_VISIBLE,267,10,70,145,hwnd,(HMENU)1,hInstance,NULL);
	SendMessage(ConSerB,WM_SETFONT,(WPARAM)fontB,MAKELPARAM(TRUE,0));
}

void CreateText(HWND hwnd,HINSTANCE hInstance) {
	LOGFONT lf = {};
	lf.lfHeight = -MulDiv(30,GetDeviceCaps(GetDC(NULL),LOGPIXELSY),140);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    wcscpy(lf.lfFaceName,L"Consolas");
    fontT = CreateFontIndirect(&lf);
	setIPT=CreateWindow(L"Edit",L"Input server's IP.",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,10,10,250,45,hwnd,(HMENU)10,hInstance,NULL);
	setRT=CreateWindow(L"Edit",L"Input Room ID.",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,10,60,250,45,hwnd,(HMENU)10,hIns,NULL);
	setNameT=CreateWindow(L"Edit",L"Input name(len<11).",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,10,110,250,45,hwnd,(HMENU)10,hIns,NULL);
	SendMessage(setNameT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
	SendMessage(setRT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
	SendMessage(setIPT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
}

void StartSocket() {
	WSAData wsaData;
    if(WSAStartup(MAKEWORD(2,2),&wsaData)) {
        MessageBox(NULL,L"Winsock setup failed!",L"Failed!",MB_OK|MB_SYSTEMMODAL);
		exit(0);
    } clientSock = socket(AF_INET,SOCK_STREAM,0);
    if(clientSock == INVALID_SOCKET) {
        MessageBox(NULL,L"Socket create failed!",L"Failed!",MB_OK|MB_SYSTEMMODAL);
		exit(0);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE: {
			SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			StartSocket();
			return 0;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			return 0;
		}
		case WM_SIZE:{
			if(wParam==SIZE_MAXIMIZED){
				
			}
			else if(wParam==SIZENORMAL){
				
			}
			break;
		}
		case BEEN_KICK:{
			MessageBox(NULL,L"You are been kicked of the server!",L"Warning",MB_OK|MB_SYSTEMMODAL);
			removeWin(SendB),removeWin(talkT),removeWin(writeT);
			MoveWindow(Main,300,300,350,195,TRUE);
			ConSerB=CreateWindow(L"Button",L"Connect",WS_CHILD|WS_VISIBLE,267,10,70,145,hwnd,(HMENU)1,hIns,NULL);
			setIPT=CreateWindow(L"Edit",L"Input server's IP.",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,10,10,250,45,hwnd,(HMENU)10,hIns,NULL);
			setRT=CreateWindow(L"Edit",L"Input Room ID.",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,10,60,250,45,hwnd,(HMENU)10,hIns,NULL);
			setNameT=CreateWindow(L"Edit",L"Input name(len<20).",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,10,110,250,45,hwnd,(HMENU)10,hIns,NULL);
			SendMessage(setRT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
			SendMessage(setIPT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
			SendMessage(setNameT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
			SendMessage(ConSerB,WM_SETFONT,(WPARAM)fontB,MAKELPARAM(TRUE,0));
			closesocket(clientSock);
			StartSocket();
			break;
		}
		case WM_COMMAND: {
			switch(wParam) {
				case 1:{//连接 Socket
					char tIP[MS],tID[MS];
					GetWindowTextA(setIPT,tIP,1024);
					GetWindowTextA(setRT,tID,1024);
					int flag1=0;
					for(int i=0;i<strlen(tIP);i++) 
						if(tIP[i]!='.'&&'0'<=tIP[i]&&tIP[i]<='9') flag1=1;
					for(int i=0;i<strlen(tID);i++) 
						if('0'<=tID[i]&&tID[i]<='9') flag1=2;
					if(!flag1){
						MessageBox(NULL,L"Connect failed!",L"Failed!",MB_OK);
						SetWindowText(setIPT,L"Input server's IP.");
						SetWindowText(setRT,L"Input Room ID.");
						SetWindowText(setNameT,L"Input name(len<20).");
						break;
					}
					sockaddr_in addrIn = {};
					addrIn.sin_family = AF_INET;
					addrIn.sin_addr.s_addr = inet_addr(tIP);
					addrIn.sin_port = htons(stoi(tID));
					if(connect(clientSock,(sockaddr*)(&addrIn),sizeof(addrIn)) == INVALID_SOCKET) {
						MessageBox(NULL,L"Connect server failed!",L"Failed!",MB_OK|MB_SYSTEMMODAL);
						SetWindowText(setIPT,L"Input server's IP");
						SetWindowText(setRT,L"Input Room ID");
						break;
					}
					MessageBox(NULL,L"Connect Success.",L"Prompt",MB_OK);
					char uName[MS];
					GetWindowTextA(setNameT,uName,11);
					SetWindowText(setNameT,L"");
					send(clientSock,uName,strlen(uName),0);
					MoveWindow(Main,300,300,550,450,TRUE);
					//创建聊天框
					talkT=CreateWindow(L"Edit",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|ES_READONLY,25,20,500,260,hwnd,(HMENU)10,hIns,NULL);
					writeT=CreateWindow(L"Edit",L"",WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL|ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL,25,285,400,130,hwnd,(HMENU)10,hIns,NULL);
					SendB=CreateWindow(L"Button",L"Send",WS_CHILD|WS_VISIBLE,440,285,100,130,hwnd,(HMENU)3,hIns,NULL);
					SendMessage(talkT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					SendMessage(writeT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					SendMessage(SendB,WM_SETFONT,(WPARAM)fontB,MAKELPARAM(TRUE,0));
					//开启接收
					thread t([&]() {
						while(true) {
							char buf[MS] = {};
							if(recv(clientSock,buf,MS,0)<0)
								break;
							flashWin(Main);
							char temp[MS] = {};
							GetWindowTextA(talkT,temp,MS);
							SetWindowTextA(talkT,(string(temp)+string(buf)+"\r\n").c_str());
							SendMessage(talkT,EM_LINESCROLL,0,99999);
						} 
						closesocket(clientSock);
						PostMessage(Main,BEEN_KICK,0,0);
					}); t.detach();
					removeWindows(ConSerB,setIPT,setRT,setNameT);
					break;
				}
				case 3:{
					char temp[MS]; 
					GetWindowTextA(writeT,temp,MS);
					SetWindowText(writeT,L"");
					send(clientSock,temp,strlen(temp),0);
					// 	MessageBoxA(NULL,"Send Success!","Send",MB_OK|MB_SYSTEMMODAL); 
					break;
				}
			}
			break;
		} 
	}
	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

HWND Create(HINSTANCE hInstance) {
	WNDCLASS wc = {};
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Window";
    wc.lpfnWndProc = WindowProc;
    wc.hCursor = LoadCursor(NULL,IDC_ARROW);
    wc.hIcon = LoadIcon(NULL,IDI_WINLOGO);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindow(
		L"Window",L"Pig Chat",
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE,
		300,300,350,195,
		NULL,NULL,hInstance,NULL);
	if(hwnd == NULL) exit(0);
	Main=hwnd;
	CreateButton(hwnd,hInstance);
	CreateText(hwnd,hInstance);
	return hwnd;
}

void MessageLoop() {
	MSG msg = {};
	while(GetMessage(&msg,NULL,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd) {
	hIns=hInstance;
	Create(hInstance);
	MessageLoop();
	return 0;
}
