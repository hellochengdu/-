// DHIPCDLL.h : main header file for the DHIPCDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif
#define WM_CLOSETHREAD WM_USER+100
#include "resource.h"		// ������
#include "IPCGlobeFun.h"
#include "GlobeClass.h"
#include "DllClass.h"
#include <list>
#include "resource.h"		// main symbols

//2017-08-01
typedef struct  struct_Conenct
{
	char DH[2];//����DH
	char HK[2];//����HK
	char TD[2];//����TD
	char HB[2];//����HB
	char GT[2];//����GT
	char O1[2];//����O1
	char O2[2];//����O2
}struct_Conenct;
//2017-05-08
typedef struct DevNetType
{	
	LONG DH;//0x20170501 ��ͨ 0x20170808 ���� 
	LONG HK;//0x20170601 ��ͨ 0x20170108 ����
	LONG TDWY;//0x20170201 ��ͨ 0x20170408 ����
	LONG HB;//0x20170301   ��ͨ 0x20171208
	LONG loginnoget;//0=����ȡ 1=��ȡ
	LONG Other1;//·��  1024
	LONG Other2;	
}DevNetType;

//2017-12-12 ���к�
typedef struct DevSerNum
{
	BYTE SEE[1024][24];
	BYTE bsernum[1024][48];
	BYTE SE[1024][24];
}DevSerNum;
DevSerNum    m_DevSerNum;


// CDHIPCDLLApp
// See DHIPCDLL.cpp for the implementation of this class
//
CMapStringToOb m_map_NVR_IPC;//NVR��IPC��Ϣ
CMapStringToOb m_chanNameMap;   //�����豸ͨ����ǩ
CString str_SeverType;    //����������3Ϊ����һ 4Ϊ�ĺ�һ��NVR
HWND AgentWnd;
std::list<void*> pUserList;
IHikDVRUser HikDVRUser[MAX_DEVS];   //2017-11-03
char buf[2000];
LONG nTimeCount;
typedef void  (CALLBACK*DVRLoginCallBack)(void* pDeviceInfo,IDVRUser* pUser,DWORD dwUserData,DWORD dwError);
HANDLE LoginEvent;
HANDLE AlarmEvent = CreateEvent(NULL,TRUE,TRUE,NULL);  //2013/1/18
//2013-10-8 
char *lp_stuOutParam;
class ConnectIPCThread
{
public:

	bool __stdcall Init(char* sDVRIP,WORD wDVRPort,char* sUserName,char* sPassword,int nIPCType,DWORD dwUserData,DVRLoginCallBack fDVRLoginCallBack);
	HANDLE __stdcall GetThreadHandle();
private:
	char DVRIP[32];
	WORD DVRPort;
	char UserName[64];
	char Password[64];
	int  IPCType;
	DWORD UserData;
	DWORD dwError;
	LONG lID;
	LONG lErrorID;
	LONG SerialID;
	IDVRUser* pDVRUser;
	DVRLoginCallBack fpDVRLoginCallBack;
	HANDLE hThread;
	unsigned threadID;
	static unsigned int  __stdcall Execute(void * pParam);


};
class CDHIPCDLLApp : public CWinApp
{
public:
	CDHIPCDLLApp();

// Overrides
public:
	virtual BOOL InitInstance();
	
	DECLARE_MESSAGE_MAP()
};
