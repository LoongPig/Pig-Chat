#ifndef UNICODE
#define UNICODE
#endif
#include <winsock2.h>
#include <thread>
#include <windows.h>
#include <ws2tcpip.h>
#include <commctrl.h>
#include <bits/stdc++.h>

#undef MB_SYSTEMMODAL
#undef HWND_TOPMOST
#define HWND_TOPMOST HWND_NOTOPMOST
#define MB_SYSTEMMODAL 0

#define KICK 5
#define CLEAR 6
#define CHAT 7
using namespace std;
const int MS=65536;
HWND setPortT,CmdT,talkT;
HWND setPortB,userListB,gIPB,gIDB,uListB,cleanB,chatB/*,clearB*/,kickB,warnB;
HWND Main,ListW,uListL,chatW,chatTalk,chatSend;
HINSTANCE hIns;
SOCKET serverSock;
HFONT fontB,fontT;
vector<SOCKET> clients;
map<int,string> names;
map<int,string> IP;
map<int,int> line;
int roomID;
struct{
	const wchar_t* t;
	int width;
}col[]={
	{L"Name",150},
	{L"IP",150},
	{L"ID",100}
};
int gSelID=-1;
//string->wstring
wstring StrToWstr(const string& str){
    if(str.empty()) return L"";
    int size_needed=MultiByteToWideChar(CP_UTF8,0,str.c_str(),(int)str.size(),NULL,0);
    if(size_needed==0) return L"";
    wstring wstr(size_needed,0);
    MultiByteToWideChar(CP_UTF8,0,str.c_str(),(int)str.size(),&wstr[0],size_needed);
    return wstr;
}
int gWidth(HWND hList,int colID){ return ListView_GetColumnWidth(hList,colID); }
int addItem(HWND hList,const wchar_t* name,const wchar_t* ip,int id){
	LVITEM lvi={0};
	lvi.mask=LVIF_TEXT;
	int row=ListView_GetItemCount(hList);
	lvi.iItem=row;
	lvi.iSubItem=0;
	lvi.pszText=(LPWSTR)name;
	ListView_InsertItem(hList,&lvi);
	ListView_SetItemText(hList,row,1,(LPWSTR)ip);
	ListView_SetItemText(hList,row,2,(LPWSTR)((id!=-1)?to_wstring(id).c_str():L"NULL"));
	for(int i=0;i<sizeof(col)/sizeof(col[0]);i++){
		ListView_SetColumnWidth(hList,i,LVSCW_AUTOSIZE);
        int currentWidth=ListView_GetColumnWidth(hList,i);
        if(currentWidth<col[i].width)
            ListView_SetColumnWidth(hList,i,col[i].width);
	}
	return row;
}
wstring getItem(HWND LV, int row, int colID) {
    wchar_t buf[MS]={0};
    ListView_GetItemText(LV,row,colID,buf,sizeof(buf)/sizeof(buf[0]));
    return wstring(buf);
}
void KickItem(HWND hList,int ID){
    if(ID!=-1){
		wstring s=getItem(uListL,ID,2);
		// if(s==L"NULL"){
		// 	if(MessageBox(NULL,L"",L"",MB_YESNO)==IDYES){
		// 		for(auto k:clients){
		// 			KickItem(uListL,line[k]);
		// 		}
		// 	}else return;
		// }
		if(ID==gSelID) gSelID=-1;
		int id=stoi(getItem(uListL,ID,2));
		closesocket(id);
		line.erase(id);
		for(auto &k:line) 
			if(k.second>ID) k.second--;
		ListView_DeleteItem(uListL,ID);
		ID=-1;
	}
	else MessageBox(NULL,L"Please select a user",L"Prompt",MB_OK);
}
void ClearList(HWND hList){
    ListView_DeleteAllItems(hList);
    gSelID=-1;
}
string GetIP(){
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2,2),&wsaData);
	char hostname[256];
	gethostname(hostname,sizeof(hostname));
	struct hostent* host=gethostbyname(hostname);
	if(host!=NULL&&host->h_addr_list[0]!=NULL)
		return string(inet_ntoa(*(struct in_addr*)host->h_addr_list[0]));
	WSACleanup();
	return "127.0.0.1";
}
void flashWin(HWND hwnd){
	if(IsIconic(hwnd)) FlashWindow(hwnd,TRUE);
	else if(hwnd!=GetForegroundWindow()) FlashWindow(hwnd,TRUE);
}
template<typename... Args>
void removeWindows(Args&...windows){
    ((DestroyWindow(windows),windows=NULL),...);
}
void CreateButton(HWND hwnd,HINSTANCE hInstance) {
	LOGFONT lf={};
	lf.lfHeight=-MulDiv(20,GetDeviceCaps(GetDC(NULL),LOGPIXELSY),140);
	lf.lfWeight=FW_NORMAL;
	lf.lfCharSet=DEFAULT_CHARSET;
	lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
	wcscpy(lf.lfFaceName,L"Consolas");
	fontB=CreateFontIndirect(&lf);
	setPortB=CreateWindow(
		L"Button",L"Create",
		WS_CHILD|WS_VISIBLE,
		267,10,65,55,hwnd,(HMENU)1,hInstance,NULL);
	SendMessage(setPortB,WM_SETFONT,(WPARAM)fontB,MAKELPARAM(TRUE,0));
}

void CreateText(HWND hwnd,HINSTANCE hInstance) {
	LOGFONT lf={};
	lf.lfHeight=-MulDiv(30,GetDeviceCaps(GetDC(NULL),LOGPIXELSY),140);
    lf.lfWeight=FW_NORMAL;
    lf.lfCharSet=DEFAULT_CHARSET;
    lf.lfOutPrecision=OUT_DEFAULT_PRECIS;
    wcscpy(lf.lfFaceName,L"Consolas");
    fontT=CreateFontIndirect(&lf);
	setPortT=CreateWindow(
		L"Edit",L"Input Room ID here.",
		WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL,
		10,10,250,55,hwnd,(HMENU)100,hInstance,NULL);
	SendMessage(setPortT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
	return ;
}

void StartSocket(int port){
	WSADATA wsaData;
	if(WSAStartup(MAKEWORD(2,2),&wsaData)){
		MessageBox(NULL,L"Winsock set up failed!",L"Failed",MB_OK|MB_SYSTEMMODAL);
		exit(0);
	}
	serverSock=socket(AF_INET,SOCK_STREAM,0);
	if(serverSock==INVALID_SOCKET){
		MessageBox(NULL,L"Socket create failed!",L"Failed",MB_OK|MB_SYSTEMMODAL);
		exit(0);
	}
	sockaddr_in addrIn={};
	addrIn.sin_family=AF_INET;
	addrIn.sin_addr.s_addr=ADDR_ANY;
	addrIn.sin_port=htons(port);
	if(bind(serverSock,(sockaddr*)(&addrIn),sizeof(addrIn))==SOCKET_ERROR) {
		MessageBox(NULL,L"Socket bind failed.",L"Failed",MB_OK|MB_SYSTEMMODAL);
		exit(0);
	}
	if(listen(serverSock,5)==INVALID_SOCKET) {
		MessageBox(NULL,L"Socket listen failed.",L"Failed",MB_OK|MB_SYSTEMMODAL);
		exit(0);
	}
	roomID=port;
}

LRESULT CALLBACK WindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam) {
	switch(uMsg) {
		case WM_CREATE: {
			SetWindowPos(hwnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
			//StartSocket();
			return 0;
		}
		case WM_DESTROY: {
			if(hwnd==Main){
				closesocket(serverSock);
				WSACleanup();
				PostQuitMessage(0);
			}
			if(hwnd==ListW){
				//DestroyWindow(ListW);
				ListW=NULL;
				removeWindows(kickB/*,clearB*/,chatB);
				line.clear();
				return 0;
			}
			break;
		}
		case WM_SIZE:{
			if(wParam==SIZE_MAXIMIZED){
				
			}
			else if(wParam==SIZENORMAL){
				
			}
			break;
		}
		case WM_NOTIFY:{
			LPNMHDR nmh=(LPNMHDR)lParam;
			if(nmh->idFrom==4)
				if(nmh->code==LVN_ITEMCHANGED){
					LPNMLISTVIEW lv=(LPNMLISTVIEW)lParam;
					if(lv->uNewState&LVIS_SELECTED) 
						gSelID=lv->iItem;
				}
		}
		case WM_COMMAND: {
			switch(wParam) {
				case 1:{
					char portT[MS];
					GetWindowTextA(setPortT,portT,MS);
					string s;
					bool flag=false;
					for(int i=0;i<strlen(portT);i++)
						if(!isalnum(portT[i])) flag=true;
					if(flag){
						MessageBox(NULL,L"Create Failed.",L"Failed",MB_OK);
						break;
					}
					if(strlen(portT)>=10){
						MessageBox(NULL,L"Room ID is too big.",L"Failed",MB_OK);
						break;		
					}
					StartSocket(stoi(string(portT)));
					MessageBox(NULL,L"Create room successfully",L"Prompt",MB_OK);
					removeWindows(setPortT,setPortB);
					RECT WinRect;
					GetWindowRect(Main,&WinRect);
					MoveWindow(Main,WinRect.left,WinRect.top,550,460,TRUE);
					talkT=CreateWindow(L"Edit",L"",
						WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL|
						ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|ES_READONLY,
						25,20,500,165,hwnd,(HMENU)100,hIns,NULL);
					CmdT=CreateWindow(L"Edit",L"",
						WS_CHILD|WS_VISIBLE|WS_BORDER|ES_AUTOHSCROLL|WS_HSCROLL|
						ES_MULTILINE|ES_AUTOVSCROLL|WS_VSCROLL|ES_READONLY,
						25,190,500,165,hwnd,(HMENU)100,hIns,NULL);
					gIPB=CreateWindow(L"Button",L"Get IP",
						WS_CHILD|WS_VISIBLE,
						25,360,100,60,hwnd,(HMENU)2,hIns,NULL);
					uListB=CreateWindow(L"Button",L"User List",
						WS_CHILD|WS_VISIBLE,
						150,360,100,60,hwnd,(HMENU)3,hIns,NULL);
					SendMessage(talkT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					SendMessage(CmdT,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					SendMessage(gIPB,WM_SETFONT,(WPARAM)fontB,MAKELPARAM(TRUE,0));
					SendMessage(uListB,WM_SETFONT,(WPARAM)fontB,MAKELPARAM(TRUE,0));
					thread t([&](){
						while(true){
							sockaddr_in clientAddr={};
							socklen_t sizeAddr=sizeof(sockaddr_in);
							SOCKET clientSock=accept(serverSock,(sockaddr*)(&clientAddr),&sizeAddr);
							clients.push_back(clientSock);
							char PutC[MS];
							GetWindowTextA(CmdT,PutC,MS);
							string Put=string(PutC);
							IP[clientSock]=string(inet_ntoa(clientAddr.sin_addr));
							Put+="Connect a client,socket is "+to_string(clientSock)+",IP is "+IP[clientSock]+"\r\n";
							SetWindowTextA(CmdT,Put.c_str());
							thread chat([&](){
								SOCKET client=clients.back();
								int T=1;
								while(true){
									char cTalk[MS]={};
									if(recv(client,cTalk,MS,0)<0) break;
									if(T==1){
										names[client]=string(cTalk); T++;
										if(uListL!=NULL) line[clientSock]=addItem(uListL,StrToWstr(names[client]).c_str(),StrToWstr(IP[client]).c_str(),client);
										continue;
									}
									flashWin(Main);
									string W="["+names[client]+"] : "+string(cTalk);
									for(auto i:clients) send(i,W.c_str(),W.size(),0);
									char tmp[MS];
									GetWindowTextA(talkT,tmp,MS);
									string w;
									if(string(tmp)=="") w=W;
									else w=string(tmp)+"\r\n"+W;
									SetWindowTextA(talkT,w.c_str());
								}
								closesocket(client);
								char temp[MS];
								GetWindowTextA(CmdT,temp,MS);
								SetWindowTextA(CmdT,(string(temp)+names[client]+" leave this server.\r\n").c_str());
								clients.erase(remove(clients.begin(),clients.end(),client),clients.end());
								names.erase(client),IP.erase(client);
								if(line.count(client)){
									ListView_DeleteItem(uListL,line[client]);
									line.erase(client);
								}
								// line.erase(client);
							}); chat.detach();
						}
					}); t.detach();
					break;
				}
				case 2:{
					char tmp[MS];
					GetWindowTextA(CmdT,tmp,MS);
					SetWindowTextA(CmdT,(string(tmp)+"Your IP is "+GetIP()+"\r\n").c_str());
					break;
				}
				case 3:{
					if(ListW!=NULL) break;
					ListW=CreateWindow(L"Window",L"User List",
						WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE,
						250,250,430,335,NULL,NULL,hIns,NULL);
					uListL=CreateWindow(WC_LISTVIEW,L"",
						WS_VISIBLE|WS_CHILD|LVS_REPORT|LVS_SINGLESEL|LVS_AUTOARRANGE|WS_BORDER,
						10,10,400,220,ListW,(HMENU)4,NULL,NULL);
					ListView_SetExtendedListViewStyle(ListW,
						LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_ONECLICKACTIVATE);
					LVCOLUMN lvc={0};
					lvc.mask=LVCF_FMT|LVCF_WIDTH|LVCF_TEXT|LVCF_TEXT|LVCF_SUBITEM;
					lvc.fmt=LVCFMT_LEFT;
					for(int i=0;i<sizeof(col)/sizeof(col[0]);i++){
						lvc.iSubItem=i;
						lvc.cx=col[i].width;
						lvc.pszText=(LPWSTR)col[i].t;
						ListView_InsertColumn(uListL,i,&lvc);
					}
					kickB=CreateWindow(L"Button",L"Kick",
						WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
						10,235,185,60,ListW,(HMENU)KICK,NULL,NULL);
					// clearB=CreateWindow(L"Button",L"Clear",
					// 	WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
					// 	150,235,120,60,ListW,(HMENU)CLEAR,NULL,NULL);
					chatB=CreateWindow(L"Button",L"Chat",
						WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
						210,235,185,60,ListW,(HMENU)CHAT,NULL,NULL);
					SendMessage(uListL,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					SendMessage(kickB,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					//SendMessage(clearB,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					SendMessage(chatB,WM_SETFONT,(WPARAM)fontT,MAKELPARAM(TRUE,0));
					// if(clients.size()) addItem(uListL,L"Everyone",L"NULL",-1);
					for(auto k:clients) line[k]=addItem(uListL,StrToWstr(names[k]).c_str(),StrToWstr(IP[k]).c_str(),(int)k);
					break;
				}
				case KICK:{
					KickItem(uListL,gSelID);
					break;
				}
				case CHAT:{
					
					break;
				}
			}
			break;
		} 
		default:
			return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}
	return 0; 
}

HWND Create(HINSTANCE hInstance) {
	WNDCLASS wc={};
    wc.hInstance=hInstance;
    wc.lpszClassName=L"Window";
    wc.lpfnWndProc=WindowProc;
    wc.hCursor=LoadCursor(NULL,IDC_ARROW);
    wc.hIcon=LoadIcon(NULL,IDI_WINLOGO);
    wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
    RegisterClass(&wc);
    
    HWND hwnd=CreateWindow(
		L"Window",L"Pig Chat",
		WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_VISIBLE,
		300,300,350,100,
		NULL,NULL,hInstance,NULL);
	if(hwnd==NULL) exit(0);
	CreateButton(hwnd,hInstance);
	CreateText(hwnd,hInstance);
	return Main=hwnd;
}

void MessageLoop(){
	MSG msg={};
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nShowCmd) {
	Create(hIns=hInstance);
	MessageLoop();
	return 0;
}