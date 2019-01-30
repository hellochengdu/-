// DHIPCDLL.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "DHIPCDLL.h"
#include <process.h> 
//#include "StreamConvertor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WEB_CFG_CMD_ENCODE                   "Encode" 
#define  H_BufLen 256*1024

int g_logintype=1;//特殊 0-普通
int g_loginfirst=1;//登录时获取
//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CDHIPCDLLApp

BEGIN_MESSAGE_MAP(CDHIPCDLLApp, CWinApp)
END_MESSAGE_MAP()

//2013-12-21
HANDLE   hThread;
// CDHIPCDLLApp construction

CDHIPCDLLApp::CDHIPCDLLApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	str_SeverType = _T("4");
	
}


// The one and only CDHIPCDLLApp object

CDHIPCDLLApp theApp;
void GetOneChannelName(AV_CFG_ChannelName *pstChannelName, int nCurChannel,long m_LoginID);

// CDHIPCDLLApp initialization
AV_CFG_ChannelName *m_pstChannelName;
BOOL CDHIPCDLLApp::InitInstance()
{
	CWinApp::InitInstance();
	
	m_pstChannelName = new AV_CFG_ChannelName[256];
	if(NULL != m_pstChannelName)
	{
		memset(m_pstChannelName, 0, 256*sizeof(AV_CFG_ChannelName));
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}
void CALLBACK fSerialDataCallBack(LONG lSerialHandle,char *pRecvDataBuffer,DWORD dwBufSize,DWORD dwUser)
//串口透明数据回调
{
}
//*******************************UT8转换*******************************************************
//*****************2013-10-22************字符转换**********************************************
size_t Utf_Unicode(wchar_t &temp, unsigned char *pUTF8)
{
	int count_bytes = 0;
	unsigned char byte_one = 0, byte_other = 0x3f; // 用于位与运算以提取位值 0x3f-->00111111
	wchar_t tmp_wchar = L'0';

	if (!pUTF8)
		return -1;

	for (;;) // 检测字节序列长度,根据第一个字节头的1个个数
	{
		if (pUTF8[0] <= 0x7f)
		{
			count_bytes = 1; // ASCII字符: 0xxxxxxx( ~ 01111111)
			byte_one = 0x7f; // 用于位与运算, 提取有效位值, 下同 01111111
			break;
		}
		if ( (pUTF8[0] >= 0xc0) && (pUTF8[0] <= 0xdf) )
		{
			count_bytes = 2; // 110xxxxx(110 00000 ~ 110 111111)
			byte_one = 0x1f; //00011111 第一字节有效位的个数
			break;
		}
		if ( (pUTF8[0] >= 0xe0) && (pUTF8[0] <= 0xef) )
		{
			count_bytes = 3; // 1110xxxx(1110 0000 ~ 1110 1111)
			byte_one = 0x0f; //00001111
			break;
		}
		if ( (pUTF8[0] >= 0xf0) && (pUTF8[0] <= 0xf7) )
		{
			count_bytes = 4; // 11110xxx(11110 000 ~ 11110 111)
			byte_one = 0x07;
			break;
		}
		if ( (pUTF8[0] >= 0xf8) && (pUTF8[0] <= 0xfb) )
		{
			count_bytes = 5; // 111110xx(111110 00 ~ 111110 11)
			byte_one = 0x03;
			break;
		}
		if ( (pUTF8[0] >= 0xfc) && (pUTF8[0] <= 0xfd) )
		{
			count_bytes = 6; // 1111110x(1111110 0 ~ 1111110 1)
			byte_one = 0x01;
			break;
		}
		return 8; // 以上皆不满足则为非法序列
	}
	// 以下几行析取UTF-8编码字符各个字节的有效位值
	//先得到第一个字节的有效位数据
	tmp_wchar = pUTF8[0] & byte_one;
	for (int i=1; i < count_bytes; i++)
	{
		tmp_wchar <<= 6; // 左移6位后与后续字节的有效位值"位或"赋值
		tmp_wchar = tmp_wchar | (pUTF8[i] & byte_other);//先与后或
	}
	// 位值析取__End!
	temp = tmp_wchar;
	return count_bytes;
}
void W2A_mine(wchar_t *lpszW, char *lpszA)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, lpszW, 1, NULL, 0, NULL, NULL); 
	int nRet = WideCharToMultiByte(CP_ACP, 0, lpszW, 1, lpszA, nLen, NULL, NULL);
	if (!nRet) 
	{
		int error = GetLastError();
	}
}

void Change_Utf8_Unicode(unsigned char *pUTF8, char *destbuf)
	{
	size_t num = 0;
	WCHAR  temp = 0;
	int index = 0;
	while (1)
		{ 
		if(*pUTF8 == NULL/* || *pUTF8== '0'*/)
			{
			break;
			}
		num = Utf_Unicode(temp, pUTF8);
		char *buffer = new char[2];
		if(num == 8)
			{
			pUTF8 = pUTF8 + 1;
			}
		else
			{
			memset(buffer,0,2);
			W2A_mine(&temp, buffer);
			if(num < 2)
				{
				destbuf[index] = buffer[0];
				index ++;
				}
			else
				{
				destbuf[index] = buffer[0];
				destbuf[index+1] = buffer[1];
				index += 2;
				}
			pUTF8 = pUTF8 + num;
			}
		delete[] buffer;
		}

	return ;
	}



VOID CALLBACK OnTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{


	DWORD dwChannel = 0;
	CString temp;
	if (idEvent == 2)
	{
		WaitForSingleObject(AlarmEvent,5000);
		ResetEvent(AlarmEvent);
		DVRListNode *DVRNode = NULL;
		std::list<void*>::iterator DVRListIt = DVRList.begin();
		while(DVRListIt!=DVRList.end())
		{
			DVRNode = (DVRListNode*)(*DVRListIt);
			if (DVRNode->pDVRUser->preViLost>0)
			{
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,102,NULL,DVRNode->pDVRUser->preViLost);
				dwChannel = (DVRNode->pDVRUser->preViLost<<16);
				temp.Format(_T("\r\nDH%S视频丢失报警结束通道：%lu，开始通道：%lu"),DVRNode->sDVRIP,DVRNode->pDVRUser->preViLost,dwChannel);
				OutputDebugString(temp);
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,102,NULL,dwChannel);
			}
			if(DVRNode->pDVRUser->preShelter>0)
			{
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,103,NULL,DVRNode->pDVRUser->preShelter);
				dwChannel = (DVRNode->pDVRUser->preShelter<<16);
				temp.Format(_T("\r\nDH%S视频遮挡报警结束通道：%lu，开始通道：%lu"),DVRNode->sDVRIP,DVRNode->pDVRUser->preShelter,dwChannel);
				OutputDebugString(temp);
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,103,NULL,dwChannel);
			}
			if(DVRNode->pDVRUser->preAlarm>0)
			{
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,104,NULL,DVRNode->pDVRUser->preAlarm);
				dwChannel = (DVRNode->pDVRUser->preAlarm<<16);
				temp.Format(_T("\r\nDH%S外部报警结束通道：%lu，开始通道：%lu"),DVRNode->sDVRIP,DVRNode->pDVRUser->preAlarm,dwChannel);
				OutputDebugString(temp);
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,104,NULL,dwChannel);
			}
			if(DVRNode->pDVRUser->preDiskFull>0)
			{
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,105,NULL,DVRNode->pDVRUser->preDiskFull);
				dwChannel = (DVRNode->pDVRUser->preDiskFull<<16);
				temp.Format(_T("\r\nDH%S硬盘满报警结束通道：%lu，开始通道：%lu"),DVRNode->sDVRIP,DVRNode->pDVRUser->preDiskFull,dwChannel);
				OutputDebugString(temp);
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,105,NULL,dwChannel);
			}
			if(DVRNode->pDVRUser->preHardDisk>0)
			{
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,106,NULL,DVRNode->pDVRUser->preHardDisk);
				dwChannel = (DVRNode->pDVRUser->preHardDisk<<16);
				temp.Format(_T("\r\nDH%S硬盘坏报警结束通道：%lu，开始通道：%lu"),DVRNode->sDVRIP,DVRNode->pDVRUser->preHardDisk,dwChannel);
				OutputDebugString(temp);
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,106,NULL,dwChannel);
			}
			DVRListIt++;
		}
		SetEvent(AlarmEvent);
		DVRNode = NULL;
	}
	else if(idEvent == 3)
	{
		WaitForSingleObject(AlarmEvent,5000);
		ResetEvent(AlarmEvent);
		DVRListNode *DVRNode = NULL;
		std::list<void*>::iterator DVRListIt = DVRList.begin();
		while(DVRListIt!=DVRList.end())
		{
			DVRNode = (DVRListNode*)(*DVRListIt);
			if(DVRNode->pDVRUser->preMotion>0)
			{
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,101,NULL,DVRNode->pDVRUser->preMotion);
				dwChannel = (DVRNode->pDVRUser->preMotion<<16);
				temp.Format(_T("\r\nDH%S移动侦测报警结束通道：%lu，开始通道：%lu"),DVRNode->sDVRIP,DVRNode->pDVRUser->preMotion,dwChannel);
				OutputDebugString(temp);
				DVRNode->pDVRUser->lpFMessCallBack(DVRNode->pDVRUser,101,NULL,dwChannel);
			}
			DVRListIt++;
		}
		SetEvent(AlarmEvent);
		DVRNode = NULL;
	}
	
}
//////////////////////////////////////////////////////////////////////
//
// ConnectIPCThread   连接IPC线程
//
/////////////////////////////////////////////////////////////////////////
bool __stdcall ConnectIPCThread::Init(char* sDVRIP,WORD wDVRPort,char* sUserName,char* sPassword,int nIPCType,DWORD dwUserData,DVRLoginCallBack fDVRLoginCallBack)
{

  strcpy_s(DVRIP,sDVRIP);
  DVRPort = wDVRPort;
  strcpy_s(UserName,sUserName);
  strcpy_s(Password,sPassword);
  IPCType = nIPCType;
  UserData = dwUserData;
  fpDVRLoginCallBack = fDVRLoginCallBack;
  hThread = (HANDLE)_beginthreadex( NULL, 0, &ConnectIPCThread::Execute, (LPVOID)this, 0, &threadID );
//   int iLogin = -1;
//   iLogin = Execute((LPVOID)this);
//   if (iLogin == 0)
// 		return FALSE;
//   else
// 	  return TRUE;
  //2013-10-8
  //lp_stuOutParam = new char[H_BufLen];


  if(hThread == NULL)
    return false;
  else
    return true;



}
HANDLE __stdcall ConnectIPCThread::GetThreadHandle()
{
	return hThread;
}
/////////////////////////////////////////////////////////////////////////

void InitAVCFGChannelName(AV_CFG_ChannelName *pstChannelName)
{
	if(NULL == pstChannelName)
	{
		return;	
	}
	else
	{
		pstChannelName->nStructSize = sizeof(AV_CFG_ChannelName);
	}

}

void GetOneChannelName(AV_CFG_ChannelName *pstChannelName, int nCurChannel,long m_LoginID)
{
	if(0 != m_LoginID)
	{
		if(NULL == pstChannelName)
		{
			return;
		}

		char *szOutBuffer = new char[32*1024];
		if(NULL == szOutBuffer)
		{
			return;
		}		

		memset(pstChannelName, 0, sizeof(AV_CFG_ChannelName));

		InitAVCFGChannelName(pstChannelName);

		int nerror = 0;

		BOOL bSuccess = CLIENT_GetNewDevConfig(m_LoginID, CFG_CMD_CHANNELTITLE, nCurChannel, szOutBuffer, 32*1024, &nerror, 5000);
		if(bSuccess)
		{
			int nRetLen = 0;
			BOOL bRet = CLIENT_ParseData(CFG_CMD_CHANNELTITLE, szOutBuffer, pstChannelName, sizeof(AV_CFG_ChannelName),&nRetLen);
			if(!bRet)
			{
				OutputDebugString(_T("Channel Title Parse data error Prompt"));
			}
		}
		else
		{
			OutputDebugString(_T("Get channel title failed! Prompt"));
		}

		if(szOutBuffer)
		{
			delete[] szOutBuffer;
			szOutBuffer = NULL;
		}

	}
	else
	{
		OutputDebugString(_T("Login first! Prompt"));
	}

}

//ConnectIPCThread  end
//////////////////////////////////////////////////////////////////////////
unsigned __stdcall  ConnectIPCThread::Execute(void * pParam)
{


	bool r=true;
	DEVICEDES* lpDeviceInfo=new DEVICEDES;
	RtlZeroMemory(lpDeviceInfo,sizeof(DEVICEDES));
	//NET_DEVICEINFO  DHDeviceInfo;
	NET_DEVICEINFO_Ex DHDeviceInfo;
	ConnectIPCThread* pConnectIPCThread = (ConnectIPCThread*)pParam;
	long lLoginBack = 0;  //glfu  2013/3/16
	char lp_stuOutParam[H_BufLen]={0};
	CString tmp;
	int    Devtype =2;//2-巡查  1- 平安  0- 普通

	while(true)
	{
		int nerror;  		
		tmp.Format(_T("\r\ntype:%S %S %d %S %d "),pConnectIPCThread->UserName ,pConnectIPCThread->Password,pConnectIPCThread->IPCType,pConnectIPCThread->DVRIP,pConnectIPCThread->DVRPort);
		//int nSpecialValue = 1;
		//pConnectIPCThread->lID = CLIENT_LoginEx2(pConnectIPCThread->DVRIP,pConnectIPCThread->DVRPort,pConnectIPCThread->UserName,pConnectIPCThread->Password ,(EM_LOGIN_SPAC_CAP_TYPE)8,&nSpecialValue,&DHDeviceInfo,&nerror);
		//if(pConnectIPCThread->lID!=0)
		//{
		//	if(DHDeviceInfo.bReserved[0]=='j' && DHDeviceInfo.bReserved[1]=='f')//2016-06-15 平安设备
		//	{
			//	tmp= tmp + _T("DL:PA ok");
			//	Devtype = 1;
			//}
			//else
			//{
		//		tmp= tmp + _T("DL:XC1 ok");
		//		Devtype = 2;
	//		}
		//}
		//else
		//{
			int nSpecialValue = 90106;
			pConnectIPCThread->lID = CLIENT_LoginEx2(pConnectIPCThread->DVRIP,pConnectIPCThread->DVRPort,pConnectIPCThread->UserName,pConnectIPCThread->Password ,(EM_LOGIN_SPAC_CAP_TYPE)28,&nSpecialValue,&DHDeviceInfo,&nerror);
			if(pConnectIPCThread->lID!=0)
			{
				if(DHDeviceInfo.bReserved[0]=='j' && DHDeviceInfo.bReserved[1]=='f')//2016-06-15 平安设备
				{
					tmp= tmp + _T("DL:PA ok");
					Devtype = 1;
				}
				else
				{
					tmp= tmp + _T("DL:XC ok");
					Devtype = 2;
				}
				
			}
			else
			{
				tmp = tmp+ _T("DL:jf  false");
			}
		//}
		BOOL bcheckok = FALSE;
		if(pConnectIPCThread->lID<1)//通版登陆
		{
			pConnectIPCThread->lID = CLIENT_LoginEx2(pConnectIPCThread->DVRIP,pConnectIPCThread->DVRPort,pConnectIPCThread->UserName,pConnectIPCThread->Password ,(EM_LOGIN_SPAC_CAP_TYPE)0, NULL,&DHDeviceInfo,&nerror);
			if (pConnectIPCThread->lID!=0 && g_logintype ==TRUE)//连接成功
			{
				tmp= tmp + _T("DL:P OK");
				Devtype = 0;
				//2017-12-13
				for (int i=0;i<1024;i++)
				{
					if(strcmp((char*)m_DevSerNum.bsernum[i],"")==0)
					{
						break;
					}
					if(strcmp((char*)m_DevSerNum.bsernum[i],(char*)DHDeviceInfo.sSerialNumber)==0)
					{
						bcheckok = TRUE;
						break;
					}
				}
				if (!bcheckok)//没有找到
				{
					CLIENT_Logout(pConnectIPCThread->lID);
					pConnectIPCThread->lID =0;
					OutputDebugString(_T("\r\ndrop!!"));
				}
				
			}
			
		}
		
	
		OutputDebugString(tmp);
		lLoginBack = pConnectIPCThread->lID;
		if(pConnectIPCThread->lID!=0)
		{    
			int nCount;
			for(nCount=0;nCount<MAX_DEVS;nCount++)
			{
				if(HikDVRUser[nCount].bUsed==false)
				{
					pConnectIPCThread->pDVRUser = &HikDVRUser[nCount];
					HikDVRUser[nCount].bUsed = true;
					HikDVRUser[nCount].lUserID = pConnectIPCThread->lID;
					//2013-10-23
					HikDVRUser[nCount].dwTypeID = pConnectIPCThread->IPCType;
					break;
				}
			}
			if(pConnectIPCThread->pDVRUser!=NULL && nCount<MAX_DEVS)   //2013  1  5 如果设备数超过128 输出错误号1288  ,2013/3/31 又改为256
			{
				pConnectIPCThread->dwError = IPC_LOCAL_NOERROR;
				DHDEV_CHANNEL_CFG piccfg;
				//NET_DVR_PICCFG piccfg;
				DWORD BytesReturned;
				//AV_CFG_RemoteDevice 

				//DHDEV_VERSION_INFO DevStateType;
				// int nReturnLen;
				//CLIENT_QueryDevState(pConnectIPCThread->lID,DH_DEVSTATE_SOFTWARE,(char*)&DevStateType,sizeof(DHDEV_VERSION_INFO),&nReturnLen,1000);

				//lpDeviceInfo->VideoNum =DHDeviceInfo.byChanNum; 
				lpDeviceInfo->VideoNum =DHDeviceInfo.nChanNum;
				//2014-12-12  n7 全是数字是31 变为28
				//if (pConnectIPCThread->IPCType!=201 && DHDeviceInfo.byDVRType==31) //not nvr
				if (pConnectIPCThread->IPCType!=201 && pConnectIPCThread->IPCType!=501 && DHDeviceInfo.nDVRType==31) //not nvr
				{
					lpDeviceInfo->AlarmNum = 28;
				}
				else
				{
					lpDeviceInfo->AlarmNum = DHDeviceInfo.nDVRType;
				}
				
				char putstr[64];
				// lpDeviceInfo->AlarmNum = HikDeviceInfo.byAlarmInPortNum;
				int nError;
				//2013-10-8
				int nvr_maxnum=lpDeviceInfo->VideoNum;
				if ((pConnectIPCThread->IPCType == 201 ||pConnectIPCThread->IPCType == 501))//NVR 2017-05-03取消通道路数限制
				{

					WaitForSingleObject(LoginEvent,INFINITE);
					ResetEvent(LoginEvent);
					//DWORD nBytesRet ;
					
					CString m_strResult,strTemp;
					//-------------
					
					//CFG_ENCODE_INFO  strgBuf;
					CString	strbug;
					char	tempDeivece[128];
					int  pppo=0;
					for (int i=0; i<nvr_maxnum; i++)
					{								
						sprintf_s(lpDeviceInfo->VideoLable[i], 32, "%s","无设备");
						//memset(lp_stuOutParam, 0, H_BufLen); 
						//if(CLIENT_GetNewDevConfigForWeb(pConnectIPCThread->lID, WEB_CFG_CMD_ENCODE, i, lp_stuOutParam, H_BufLen, &nError,5000) )//2013-10-22       
						//{
						//	strbug.Format(_T("%S"),lp_stuOutParam);
						//	pppo = strbug.Find(_T("Name"));
						//	if (pppo>0)
						//	{
						//		strbug = strbug.Mid(pppo+9);
						//		pppo = strbug.Find(_T("\","));  
						//		if (pppo>0)
						//		{
						//			strbug = strbug.Left(pppo);

						//			if(strbug==_T("CIF"))  //判断是4kNVR
						//			{
						//				
						//				HikDVRUser[nCount].dwTypeID=2014;//4KNVR
						//				break;
						//			}

						//			sprintf_s(lpDeviceInfo->VideoLable[i], 32, "%S",strbug);
						//			OutputDebugString(_T("\r\n"));
						//			OutputDebugString(strbug);

						//			ZeroMemory(tempDeivece,128);
						//			Change_Utf8_Unicode((unsigned char * )lpDeviceInfo->VideoLable[i], tempDeivece);
						//			memcpy(lpDeviceInfo->VideoLable[i],tempDeivece,32*sizeof(char));

						//		}									

						//	}										

						//}
						
					}
						//-----------------------------------------
					if (CLIENT_GetNewDevConfig(pConnectIPCThread->lID, CFG_CMD_VIDEOINDEVGROUP, 0, lp_stuOutParam, H_BufLen, &nError,1000))
					{
						//2015-05-31 登录时获取通道标签
						if (g_loginfirst ==1)
						{
							CFG_VIDEOINDEVGROUP_INFO stuDevs = {0};
							if (CLIENT_ParseData(CFG_CMD_VIDEOINDEVGROUP, lp_stuOutParam, &stuDevs, sizeof(stuDevs), NULL))
							{
								memset(lp_stuOutParam, 0, H_BufLen);
								for (int i=0; i<stuDevs.nVideoDevNum; ++i)//i++)
								{								
									const CFG_VIDEOINDEV_INFO& stuDev = stuDevs.stuVideoInDevInfo[i];
									strTemp = stuDev.szDevID;								
									strTemp = strTemp.Mid(4);
									//0-无设备 1-成功 2-不成功
									if(stuDev.byStatus ==1)
									{			
										bool bSuccess = CLIENT_GetNewDevConfigForWeb(pConnectIPCThread->lID, WEB_CFG_CMD_ENCODE, _ttoi(strTemp)-1, lp_stuOutParam, H_BufLen, &nError, 5000);
										if(bSuccess)
										{
											strbug.Format(_T("%S"),lp_stuOutParam);
											pppo = strbug.Find(_T("Name"));
											if (pppo>0)
											{
												strbug = strbug.Mid(pppo+9);
												pppo = strbug.Find(_T("\","));  
												if (pppo>0)
												{
													strbug = strbug.Left(pppo);											

													sprintf_s(lpDeviceInfo->VideoLable[i], 63, "%S",strbug);						

													ZeroMemory(tempDeivece,128);
													Change_Utf8_Unicode((unsigned char * )lpDeviceInfo->VideoLable[i], tempDeivece);
													memcpy(lpDeviceInfo->VideoLable[i],tempDeivece,63*sizeof(char));

												}									

											}	
										}
										else
										{									

											sprintf_s(lpDeviceInfo->VideoLable[_ttoi(strTemp)-1], 32, "%s","OFF");

										}

									}								
									else  if(stuDev.byStatus ==2)
									{
										sprintf_s(lpDeviceInfo->VideoLable[_ttoi(strTemp)-1], 32, "%s","OFF");
									}
									else if(stuDev.byStatus ==0)
									{
										sprintf_s(lpDeviceInfo->VideoLable[_ttoi(strTemp)-1], 32, "%s","无设备");
									}

								}//end	


							}//if
						}
						else
						{
							for (int i=0; i<nvr_maxnum; i++)
							{								
								sprintf_s(lpDeviceInfo->VideoLable[i], 32, "%s%d","通道",i+1);
							}
						}
						
					}
					else//6000-4k
					{
						HikDVRUser[nCount].dwTypeID=2014;
					}
				//	//-------------------------------------------------------

					SetEvent(LoginEvent);
				}
				if( (pConnectIPCThread->IPCType != 201 && pConnectIPCThread->IPCType != 501) || HikDVRUser[nCount].dwTypeID==2014)//dvr
				{
					WaitForSingleObject(LoginEvent,INFINITE);
					ResetEvent(LoginEvent);
					
					if (g_loginfirst ==1)
					{
						BOOL r;
						CString strcounts;
						DHDEV_VIRTUALCAMERA_STATE_INFO stCameraState = {sizeof(DHDEV_VIRTUALCAMERA_STATE_INFO)};

						for(int i = 0;i<(int)lpDeviceInfo->VideoNum;i++)    //得到通道名
						{

							//2014-12-11  查询IPC的状态
							//strcounts.Format(_T("\r\nfor %d"),i);
							//OutputDebugString(strcounts);
							if (lpDeviceInfo->AlarmNum==28 || lpDeviceInfo->AlarmNum==31) //n7 设备
							{
								stCameraState.nChannelID = i;
								r = CLIENT_QueryDevState(pConnectIPCThread->lID,DH_DEVSTATE_VIRTUALCAMERA,(char *)(&stCameraState), sizeof(DHDEV_VIRTUALCAMERA_STATE_INFO),&nError,3000);
							}
							else
							{
								r=FALSE;
							}
							DWORD   destat = stCameraState.emConnectState;
							if(r==TRUE && destat!=2)
							{
								//是IPC 通道
								//if(stCameraState.emConnectState!=2)
								//	{
								sprintf_s(lpDeviceInfo->VideoLable[i],"%s","OFF");//2011-08-26

								//}
							}
							else
							{
								GetOneChannelName(&m_pstChannelName[i],i,pConnectIPCThread->lID);
								char szBuf[AV_CFG_Channel_Name_Len + 1] = {0};
								memcpy(szBuf, m_pstChannelName[i].szName, AV_CFG_Channel_Name_Len);
								if (strcmp(szBuf,"") ==0)//2011-09-01
								{
									sprintf_s(lpDeviceInfo->VideoLable[i],"%s%d","通道",i+1);//2011-08-26							
								}
								else
								{
									memcpy_s(lpDeviceInfo->VideoLable[i],64,szBuf,63);
								}

							}

						}
						//OutputDebugString(_T("\r\nOut for"));
					}
					else
					{
						for (int i=0; i<nvr_maxnum; i++)
						{								
							sprintf_s(lpDeviceInfo->VideoLable[i], 32, "%s%d","通道",i+1);
						}
					}
					//20170808 
					CString strtmp;
					NVR_IPC *pa=NULL;
					if(HikDVRUser[nCount].dwTypeID==2014)
					{
						DH_IN_MATRIX_GET_CAMERAS stInParam = {sizeof(stInParam)};
						DH_OUT_MATRIX_GET_CAMERAS stOutParam = {sizeof(stOutParam)};
						stOutParam.nMaxCameraCount = lpDeviceInfo->VideoNum;                            // 设置需要获取的最大前端个数
						stOutParam.pstuCameras = new DH_MATRIX_CAMERA_INFO[stOutParam.nMaxCameraCount];
						memset(stOutParam.pstuCameras,0,stOutParam.nMaxCameraCount*sizeof(DH_MATRIX_CAMERA_INFO));
						for (int i = 0; i < stOutParam.nMaxCameraCount; i++)
						{
							stOutParam.pstuCameras[i].dwSize = sizeof(DH_MATRIX_CAMERA_INFO);
							stOutParam.pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
						}

						BOOL bRet =  CLIENT_MatrixGetCameras(pConnectIPCThread->lID,&stInParam,&stOutParam,5000);
						// 打印前端登陆信息
						if(bRet)
						{
							strtmp.Format(_T("%d"),(char*)pConnectIPCThread->lID);
							if(!m_map_NVR_IPC.Lookup(strtmp,(CObject*&)pa))
							{
								pa = new NVR_IPC();
								//strtmp.Format(_T("%S"),(char*)pConnectIPCThread->UserData);
								sprintf_s(pa->ID,"%d",(char*)pConnectIPCThread->lID);								
								m_map_NVR_IPC.SetAt(strtmp,(CObject*)pa);
							}							
							
							for (int i = 0; i < __min(stOutParam.nRetCameraCount,stOutParam.nMaxCameraCount); i++)
							{
								/*if (!stOutParam.pstuCameras[i].stuRemoteDevice.bEnable)
								{
								continue;
								}	*/
								
								strcpy_s(pa->ip[i],23,stOutParam.pstuCameras[i].stuRemoteDevice.szIp);
								pa->port[i] = stOutParam.pstuCameras[i].stuRemoteDevice.nPort;
								strcpy_s(pa->user[i],31,stOutParam.pstuCameras[i].stuRemoteDevice.szUserEx);
								strcpy_s(pa->passwd[i],31,stOutParam.pstuCameras[i].stuRemoteDevice.szPwdEx);
								//2018-05-07
								memcpy_s(pa->SerNO[i],48,stOutParam.pstuCameras[i].stuRemoteDevice.szSerialNo,48);	 
								pa->devtype = Devtype;

								//2018-05-26
								if (strcmp(stOutParam.pstuCameras[i].stuRemoteDevice.szIp,"192.168.0.0")==0 ||strcmp(stOutParam.pstuCameras[i].stuRemoteDevice.szIp,"0.0.0.0")==0 || strcmp(stOutParam.pstuCameras[i].stuRemoteDevice.szIp,"")==0)//没有设备
								{
									strcpy(lpDeviceInfo->VideoLable[i],"无设备");
								}
							}
						}
						
					}
					

					SetEvent(LoginEvent);
				}
				
				/* SerialID = NET_DVR_SerialStart(lID,1,&fSerialDataCallBack,0); //建立透明通道   //建立透明通道后就不能用NET_DVR_TransPTZ_Other.
				//透明通道要分开写，最好只建立232的透明通道。
				pDVRUser->lSerialID = SerialID;*/
				DVRListNode *Node = new DVRListNode(pConnectIPCThread->lID,pConnectIPCThread->pDVRUser,pConnectIPCThread->DVRIP);
				if(nCount<MAX_DEVS)
				{
					Node->pDVRUser  = &HikDVRUser[nCount];
				}
				// DVRList->Add(Node);
				DVRList.push_front((void*)Node);
			}
			else
			{
				CLIENT_Logout(pConnectIPCThread->lID);  //
				pConnectIPCThread->dwError = 2049;  //设备数超过128个 2013、3、31 又改为256
			}
		//NET_DVR_SetDVRMessCallBack(&fMessCallBack);
		}
		else
		{

			pConnectIPCThread->dwError = nerror;
			
		}

		pConnectIPCThread->fpDVRLoginCallBack((void*)lpDeviceInfo, pConnectIPCThread->pDVRUser, pConnectIPCThread->UserData, pConnectIPCThread->dwError);
		if( pConnectIPCThread->dwError==0)
		{
			/************************************************************************/
			/* 保留通道标签  2014-1-8 TGX                                           */
			CString str_tmp;
			str_tmp.Format(_T("%ld"), lLoginBack);
			m_chanNameMap.SetAt(str_tmp, (CObject *&)lpDeviceInfo);
			/************************************************************************/			

			WaitForSingleObject(LoginEvent,500);
			ResetEvent(LoginEvent);
			pUserList.push_front((void*) pConnectIPCThread->pDVRUser);
			int maxnum = pUserList.size();
			str_tmp.Format(_T("\r\npUserList++ num:%d   lid:%u"), maxnum,pConnectIPCThread->pDVRUser->lUserID);
			OutputDebugString(str_tmp);
			SetEvent(LoginEvent);
		}//2014-05-10
		else
		{
			if (lpDeviceInfo!=NULL)
			{
				delete lpDeviceInfo;
				lpDeviceInfo = NULL;
			}
		}
		// LoginEvent->SetEvent();
		PostMessage(AgentWnd,WM_CLOSETHREAD,(DWORD)pConnectIPCThread,0);
		r=false;
		break;
		//this->Suspend();

	}
	return lLoginBack;   //glfu 2013/3/16 更改通过返回值来判断是否登录成功 0表示失败，大于0表示成功
}

 LRESULT CALLBACK AgentWndProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
 {
	 ConnectIPCThread* pConnectIPCThread;
	 switch(uMsg)
	 {
	   case WM_CLOSETHREAD:
           pConnectIPCThread = (ConnectIPCThread*)wParam;
		//   _endthreadex(pConnectIPCThread->GetThreadHandle() );
		   CloseHandle(pConnectIPCThread->GetThreadHandle());
		   //_endthreadex();
/*		   pConnectIPCThread = (ConnectIPCThread*)pConnectIPCThread;
		   if(pConnectIPCThread!=NULL)
			{
			// pConnectIPCThread->Exit();
			pConnectIPCThread->Free();
			// delete pConnectIPCThread;
			}                           */
		   
		 break;
	   default:
		  return DefWindowProc(hwnd,uMsg,wParam,lParam);
	 }
	 return 0;
 }
 //void CALLBACK pfDisConnect( //大华断线回调
	// LONG  lLoginID,
	// char  *pchDVRIP,
	// LONG  nDVRPort,
	// DWORD dwUser
	// )
 //设备断开时回调函数，可以用来处理断开后设备列表的更新及设备的删除操作
 void CALLBACK DisConnectFunc(LONG lLoginID, char *pchDVRIP, LONG nDVRPort, DWORD dwUser)
 {
	 return; //2013-10-25 三合一收不到回调 暂不用


	 CString tmpIP;         //glfu 20130504增加断线回调的信息
	 tmpIP.Format(_T("\r\nnvr大华断线回调ip%S"),pchDVRIP);
	 OutputDebugString(tmpIP);
	 IDVRUser* pDVRUser;
//	 int ret;
	// unsigned long RetCount;
	 DWORD* pBuf = (DWORD*)buf;
	 WaitForSingleObject(LoginEvent,500);
	 ResetEvent(LoginEvent);
	 try{
		 std::list<void*>::iterator pUserListIt = pUserList.begin();
		 while(pUserListIt!=pUserList.end())
		 {
			 pDVRUser = (IDVRUser*)(*pUserListIt);
			 if(pDVRUser!=NULL)
			 {

				 if(pDVRUser->lUserID==lLoginID)
				 {
					 if(pDVRUser->lpFMessCallBack!=NULL)
					 {
						 pDVRUser->lpFMessCallBack(pDVRUser,0,NULL,0);
						 (*pUserListIt)=NULL;
					 }
					 break;
				 }

			 }
			 pUserListIt++;
		 }
loop:  pUserListIt = pUserList.begin(); //int pUserListCount=0;
		 while(pUserListIt!=pUserList.end())
		 {
			 if((*pUserListIt)==NULL)
			 {
				 pUserList.erase(pUserListIt);
				 goto loop;
			 }
			 pUserListIt++;
		 }
		 nTimeCount = 0;
		 SetEvent(LoginEvent);

		 /************************************************************************/
		 /* 设备掉线，删除保存的设备通道标签信息   2014-1-8 TGX                  */
		 tmpIP.Format(_T("%ld"), lLoginID);
		 DEVICEDES* pd = NULL;
		 if (m_chanNameMap.Lookup(tmpIP, (CObject *&)pd))
		 {
			 m_chanNameMap.RemoveKey(tmpIP);
			 if (pd != NULL)
			 {
				 delete pd;
				 pd = NULL;
			 }
		 }
		 //2017-08-09
		 NVR_IPC *pa=NULL;
		 if(m_map_NVR_IPC.Lookup(tmpIP,(CObject *&)pa))
		 {
			 m_map_NVR_IPC.RemoveKey(tmpIP);
			 if (pa != NULL)
			 {
				 delete pa;
				 pa = NULL;
			 }
		 }
		 /************************************************************************/
		 
	 }
	 catch(...)
	 {
		 OutputDebugString(_T("\r\n 为什么断线会掉会发生异常了！！！"));
	 }
 }
 //---------------------------------timer 200ms-----------------------
 VOID   CALLBACK   TimerProc(HWND   hwnd, 	UINT   uMsg, 	UINT_PTR   idEvent, 	DWORD   dwTime 	)
 {
	//if (str_SeverType == _T("3") || str_SeverType == _T("33") ||  str_SeverType == _T("333"))//33 nvr
	{
		static int i_t=0;
		i_t+=200;
		if (i_t>1000*60*3)//3分钟
		//if (i_t>1000*10*3)//test
		{
			i_t =0;
			IDVRUser* pDVRUser;
			int nError; 
			DWORD dw_err,BytesReturned;
			DHDEV_CHANNEL_CFG piccfg;
			DWORD* pBuf = (DWORD*)buf;
			CString		strTemp;
			DEVICEDES lpDeviceInfo;
			DEVICEDES* lpdi = NULL;
			BOOL isUpdate = FALSE;  //判断通道标签是否发生改变
			BOOL b_r=TRUE;
			NET_TIME lpnettime;
			WaitForSingleObject(LoginEvent,500);
			ResetEvent(LoginEvent);
			OutputDebugString(_T("\r\n开始检查设备状态in"));
			try
			{
				std::list<void*>::iterator pUserListIt = pUserList.begin();
				while(pUserListIt!=pUserList.end())
				{
					pDVRUser = (IDVRUser*)(*pUserListIt);
					if(pDVRUser!=NULL)
					{
						

						isUpdate = FALSE;
						strTemp.Format(_T("%ld"), pDVRUser->lUserID);
						OutputDebugString(_T("\r\n<<<<设备名:"));
						OutputDebugString(strTemp);
						if (!m_chanNameMap.Lookup(strTemp, (CObject *&)lpdi))
						{
							lpdi = NULL;
						}
						ZeroMemory(&lpDeviceInfo,sizeof(DEVICEDES));
						if ((pDVRUser->dwTypeID==201 || pDVRUser->dwTypeID==501))//nvr
						{
							CString	strbug;
							char	tempDeivece[128];
							int  pppo=0;
							dw_err=0;
							//-----------------------------------------
							if(!CLIENT_QueryDeviceTime(pDVRUser->lUserID,&lpnettime,5000))
							{
								dw_err = CLIENT_GetLastError();   
							}
							if (dw_err == 0x90002008 ||dw_err == 0x80000025 || dw_err==NET_OPEN_CHANNEL_ERROR)//not online
							{
								pDVRUser->lpFMessCallBack(pDVRUser,0,NULL,0);
								(*pUserListIt)=NULL;	

								/************************************************************************/
								/* 设备掉线清楚通道信息  2014-1-8 TGX                                   */
								CString str_ID;
								DEVICEDES* pd = NULL;
								str_ID.Format(_T("%ld"), pDVRUser->lUserID);
								if (m_chanNameMap.Lookup(str_ID, (CObject *&)pd))
								{
									m_chanNameMap.RemoveKey(str_ID);
									if (pd != NULL)
									{
										delete pd;
										pd = NULL;
									}
								}
								//2017-08-09
								NVR_IPC *pa=NULL;
								if(m_map_NVR_IPC.Lookup(str_ID,(CObject *&)pa))
								{
									m_map_NVR_IPC.RemoveKey(str_ID);
									if (pa != NULL)
									{
										delete pa;
										pa = NULL;
									}
								}

							}
							else if(str_SeverType == _T("33") || str_SeverType == _T("333"))//2014-3-27)
							{
								for (int i=0; i<128; i++)
								{								
									sprintf_s(lpDeviceInfo.VideoLable[i],"%s","无设备");		//表示没有获取到名称					
								}
								//-----------------------------------------------
								char lp_stuOutParam[H_BufLen];
								memset(lp_stuOutParam, 0, H_BufLen); 
								if (CLIENT_GetNewDevConfig(pDVRUser->lUserID, CFG_CMD_VIDEOINDEVGROUP, 0, lp_stuOutParam, H_BufLen, &nError,3000))
								{
									
																	
										CFG_VIDEOINDEVGROUP_INFO stuDevs = {0};
										if (CLIENT_ParseData(CFG_CMD_VIDEOINDEVGROUP, lp_stuOutParam, &stuDevs, sizeof(stuDevs), NULL))
										{
											memset(lp_stuOutParam, 0, H_BufLen); 
											//lpDeviceInfo.VideoNum = stuDevs.nVideoDevNum;
											for (int i=0; i<stuDevs.nVideoDevNum; i++)
											{								
												const CFG_VIDEOINDEV_INFO& stuDev = stuDevs.stuVideoInDevInfo[i];
												strTemp = stuDev.szDevID;								
												strTemp = strTemp.Mid(4);
												//0-无设备 1-成功 2-不成功
												if(stuDev.byStatus ==1)
												{																																											
													
													if(CLIENT_GetNewDevConfigForWeb(pDVRUser->lUserID, WEB_CFG_CMD_ENCODE, _ttoi(strTemp)-1, lp_stuOutParam, H_BufLen, &nError,3000) )//2013-10-22       
													{
														strbug.Format(_T("%S"),lp_stuOutParam);
														pppo = strbug.Find(_T("Name"));
														if (pppo>0)
														{
															strbug = strbug.Mid(pppo+9);
															pppo = strbug.Find(_T("\","));  
															if (pppo>0)
															{
																strbug = strbug.Left(pppo);
																if (strbug!=""&&strbug!=" "&&strbug!="  ")
																{
																	sprintf_s(lpDeviceInfo.VideoLable[_ttoi(strTemp)-1],"%S",strbug);
																	ZeroMemory(tempDeivece,128);
																	Change_Utf8_Unicode((unsigned char * )lpDeviceInfo.VideoLable[_ttoi(strTemp)-1], tempDeivece);
																	memcpy(lpDeviceInfo.VideoLable[_ttoi(strTemp)-1],tempDeivece,32*sizeof(char));
																}
														
															}						
														}									

													}
												}
												if(stuDev.byStatus ==2)
												{
													sprintf_s(lpDeviceInfo.VideoLable[_ttoi(strTemp)-1],"%s","OFF");
													//strcpy_s(lpdi->VideoLable[_ttoi(strTemp)-1], lpDeviceInfo.VideoLable[_ttoi(strTemp)-1]);
													//isUpdate = TRUE;
												}

										}//end	
									}//if
								
										pDVRUser->lpFMessCallBack(pDVRUser,2,(char*)&lpDeviceInfo,sizeof(DEVICEDES));//回调刷新
										

								}//if
							}											
							
						}
						//---------------不是6000------------------------------------------------------------------------
						if(pDVRUser->dwTypeID!=201 && pDVRUser->dwTypeID!=501)//dvr .ipc
						{
							char *szOutBuffer = new char[32*1024];
							if(NULL == szOutBuffer)
							{
								return;
							}
							dw_err=0;
							//if(!CLIENT_GetNewDevConfig(pDVRUser->lUserID,CFG_CMD_CHANNELTITLE,0,szOutBuffer,32*1024,&nError,5000))
							
							if(!CLIENT_QueryDeviceTime(pDVRUser->lUserID,&lpnettime,5000))
							{
								dw_err = CLIENT_GetLastError();   
							}
							
							if (dw_err == 0x90002008 ||dw_err == 0x80000025 || dw_err==NET_OPEN_CHANNEL_ERROR)//not online
							{
									pDVRUser->lpFMessCallBack(pDVRUser,0,NULL,0);
									(*pUserListIt)=NULL;	

									/************************************************************************/
									/* 设备掉线清楚通道信息  2014-1-8 TGX                                   */
									CString str_ID;
									DEVICEDES* pd = NULL;
									str_ID.Format(_T("%ld"), pDVRUser->lUserID);
									if (m_chanNameMap.Lookup(str_ID, (CObject *&)pd))
									{
										m_chanNameMap.RemoveKey(str_ID);
										if (pd != NULL)
										{
											delete pd;
											pd = NULL;
										}
									}
									//2017-08-09
									NVR_IPC *pa=NULL;
									if(m_map_NVR_IPC.Lookup(str_ID,(CObject *&)pa))
									{
										m_map_NVR_IPC.RemoveKey(str_ID);
										if (pa != NULL)
										{
											delete pa;
											pa = NULL;
										}
									}
									/************************************************************************/
							}					
							else if(str_SeverType == _T("3") || str_SeverType == _T("33")|| str_SeverType == _T("333"))//2014-3-27)*/
							{
								//strcpy(lpDeviceInfo.VideoLable[0],piccfg.szChannelName);
								DHDEV_VIRTUALCAMERA_STATE_INFO stCameraState = {sizeof(DHDEV_VIRTUALCAMERA_STATE_INFO)};
								int m = 16;
								if (lpdi != NULL)
								{
									m = lpdi->VideoNum;
								}
								for (int i=0;i<m;i++)
								{
									 
									lpDeviceInfo.VideoNum = i+1;
									//CLIENT_GetDevConfig(pDVRUser->lUserID,DH_DEV_CHANNELCFG,i,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned,1000);
									//2014-12-11  查询IPC的状态
									if(lpdi->AlarmNum==28 || lpdi->AlarmNum==31)//n7
									{
										stCameraState.nChannelID = i;
										b_r = CLIENT_QueryDevState(pDVRUser->lUserID,DH_DEVSTATE_VIRTUALCAMERA,(char *)(&stCameraState), sizeof(DHDEV_VIRTUALCAMERA_STATE_INFO),&nError,3000);
									}
									else
									{
										b_r = FALSE;
									}
									
									if(b_r == TRUE && stCameraState.emConnectState!=2)
									{
										//是IPC 通道
										//if()
										//{
											sprintf_s(lpDeviceInfo.VideoLable[i],"%s","OFF");//2011-08-26

										//}
									}
									else
									{
										GetOneChannelName(&m_pstChannelName[i],i,pDVRUser->lUserID);
										char szBuf[AV_CFG_Channel_Name_Len + 1] = {0};
										memcpy(szBuf, m_pstChannelName[i].szName, AV_CFG_Channel_Name_Len);
										if (strcmp(szBuf,"") ==0)//2011-09-01
										{
											sprintf_s(lpDeviceInfo.VideoLable[i],"%s%d","通道",i+1);//2011-08-26							
										}
										else
										{
											strcpy(lpDeviceInfo.VideoLable[i],szBuf);
										}
									}				
									

									
								}
								//2018-05-26
								if(pDVRUser->dwTypeID==2014)
								{
									DH_IN_MATRIX_GET_CAMERAS stInParam = {sizeof(stInParam)};
									DH_OUT_MATRIX_GET_CAMERAS stOutParam = {sizeof(stOutParam)};
									stOutParam.nMaxCameraCount = lpDeviceInfo.VideoNum;                            // 设置需要获取的最大前端个数
									stOutParam.pstuCameras = new DH_MATRIX_CAMERA_INFO[stOutParam.nMaxCameraCount];
									memset(stOutParam.pstuCameras,0,stOutParam.nMaxCameraCount*sizeof(DH_MATRIX_CAMERA_INFO));
									for (int i = 0; i < stOutParam.nMaxCameraCount; i++)
									{
										stOutParam.pstuCameras[i].dwSize = sizeof(DH_MATRIX_CAMERA_INFO);
										stOutParam.pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
									}

									BOOL bRet =  CLIENT_MatrixGetCameras(pDVRUser->lUserID,&stInParam,&stOutParam,5000);
									// 打印前端登陆信息
									if(bRet)
									{													

										for (int i = 0; i < __min(stOutParam.nRetCameraCount,stOutParam.nMaxCameraCount); i++)
										{																					

											//2018-05-26
											if (strcmp(stOutParam.pstuCameras[i].stuRemoteDevice.szIp,"192.168.0.0")==0)//没有设备
											{
												strcpy(lpDeviceInfo.VideoLable[i],"无设备");
											}
										}
									}

								}
								//end
								for (int i=0;i<m;i++)
								{
									if (lpdi != NULL)
									{
										if (strcmp(lpdi->VideoLable[i], lpDeviceInfo.VideoLable[i]) != 0)
										{
											strcpy(lpdi->VideoLable[i], lpDeviceInfo.VideoLable[i]);
											isUpdate = TRUE;
										}
									}
									else
										isUpdate = TRUE;
								}
								
								//////////////////////////////////////////////////////////////////////////
								if (isUpdate)
									pDVRUser->lpFMessCallBack(pDVRUser,2,(char*)&lpDeviceInfo,sizeof(DEVICEDES));//回调刷新

							}

							//-------------------------------
							if(szOutBuffer)
							{
								delete[] szOutBuffer;
								szOutBuffer = NULL;
							}
						}
						
						OutputDebugString(_T("\r\n设备名:"));
						OutputDebugString(strTemp);
						OutputDebugString(_T(">>>>"));
					}
					pUserListIt++;
				}
loop:			pUserListIt = pUserList.begin(); //int pUserListCount=0;
				while(pUserListIt!=pUserList.end())
				{
					if((*pUserListIt)==NULL)
					{
						pUserList.erase(pUserListIt);
						goto loop;
					}
					pUserListIt++;
				}
				nTimeCount = 0;
				SetEvent(LoginEvent);
				OutputDebugString(_T("\r\n开始检查设备状态out"));
			}
			catch(...)
			{
				SetEvent(LoginEvent);
				OutputDebugString(_T("\r\n 为什么断线会掉会发生异常了！！！"));
			}
		}
	}

 }
 DWORD CALLBACK   Thread(PVOID   pvoid)   
 {   
	 MSG  msg;   
	 PeekMessage(&msg,NULL,WM_USER,WM_USER,PM_NOREMOVE);   
	 UINT  timerid=SetTimer(NULL,1,200,TimerProc);   
	 BOOL  bRet;   

	 while(   (bRet = GetMessage(&msg,NULL,0,0))!=   0)   
	 {     
		 if(bRet==-1)   
		 {   
			 OutputDebugStringW(_T(" handle   the   error   and   possibly   exit"));   
		 }   
		 else   
		 {    
			 TranslateMessage(&msg);     
			 DispatchMessage(&msg);     
		 }   
	 }   
	 KillTimer(NULL,timerid);   
	 OutputDebugStringW(_T("thread   end   here\n"));   
	 return   0;   
 }
 void GetCurrentPath(TCHAR* pPath)
 {
	 CString strPath;
	 //get entire path of running application and its title
	 GetModuleFileName(NULL,pPath,MAX_PATH);
	 strPath.Format(pPath);
	 int iPos=strPath.ReverseFind('\\');
	 if ((unsigned)iPos == -1)
	 {
		 return ;
	 }
	 memset(pPath, 0, MAX_PATH);
	 _stprintf(pPath,L"%s",strPath.Left(iPos));
	 //_strncpy(pPath, strPath.Left(iPos).GetBuffer(0), iPos);
	 pPath[iPos]='\0';
 }
 //////////////////////////////////////////////////////////////////////////
 extern "C"
 {
	 DWORD  __stdcall  IPC_DH_Local_Init() //__declspec(dllexport)
	 {

		 BOOL ret;
		 /************************************************************************/
		 
		 /************************************************************************/
		 TCHAR strPath[MAX_PATH] = {0};
		 TCHAR striniPath[MAX_PATH] = {0};
		 
		 GetCurrentPath(strPath);
		 //2017-12-12
		 CString strdevsernumpath(strPath); 
		 strdevsernumpath = strdevsernumpath + _T("\\KDSN.dll");
		 CFile  cfff;
		 RtlZeroMemory(&m_DevSerNum,sizeof(m_DevSerNum));
		 if (cfff.Open(strdevsernumpath,CFile::modeReadWrite))
		 {
			 cfff.Read(&m_DevSerNum,sizeof(m_DevSerNum));
			 cfff.Close();
			 OutputDebugString(_T("\r\n get KDSN.dll ok"));
		 }

		 //2017-05-08
		 TCHAR    c_pathBuf[MAX_PATH] = {0};
		 CString strdevlogintype;
		 if (GetWindowsDirectory(c_pathBuf,MAX_PATH))
		 {
			 strdevlogintype = c_pathBuf;
		 }
		 strdevlogintype = strdevlogintype + _T("\\system32\\drivers\\devnet.sys");
		 CFile mfile;
		 //DevNetType lpDevNetType;
		 struct_Conenct lp_connect;
		 RtlZeroMemory(&lp_connect,sizeof(struct_Conenct));
		 if (mfile.Open(strdevlogintype,CFile::modeReadWrite))
		 {
			 mfile.Read(&lp_connect,sizeof(struct_Conenct));
			 mfile.Close();

			 
		 }
		 else
		 {
			 strdevlogintype = c_pathBuf;
			 strdevlogintype = strdevlogintype + _T("\\SysWOW64\\drivers\\devnet.sys");
			 if (mfile.Open(strdevlogintype,CFile::modeReadWrite))
			 {
				 mfile.Read(&lp_connect,sizeof(struct_Conenct));
				 mfile.Close();
			 }
			 
		 }
		 if(strcmp(lp_connect.DH,"P") ==0)//普通
		 {
			// g_logintype=0;//普通
			 OutputDebugStringW(_T("\r\n<DHIPC>g_logintype=0"));
		 }
		 if (strcmp(lp_connect.O1,"1") == 0)
		 {
			 g_loginfirst=0;//不获取
			 OutputDebugStringW(_T("\r\n<DHIPC>g_loginfirst=0"));
		 }
		 CString		str_ssippath;
		 str_ssippath.Format(_T("%s\\SSIP.DLL"),strPath);
		 if(GetFileAttributes(str_ssippath) == -1)//no SSIP  表示部是SIP服务器
		 {
			 g_logintype=0;//普通
			 g_loginfirst=0;//不获取
			 OutputDebugStringW(_T("\r\n<DHIPC>g_logintype=0"));
			 OutputDebugStringW(_T("\r\n<DHIPC>g_loginfirst=0"));
		 }		 
		 /*_stprintf(striniPath,L"%s\\DLT.dll",strPath);
		 char c_buf[10]={0};
		 CFile mfile;
		 if (mfile.Open(striniPath,CFile::modeReadWrite))
		 {
			 mfile.Read(c_buf,7);
			 mfile.Close();
		 }
		 if (strcmp(c_buf,"jfkj")==0)
		 {
			 g_logintype=0;//普通
			 OutputDebugStringW(_T("\r\ng_logintype=0"));
		 }
		 else if(strcmp(c_buf,"jfkjyfb")==0)
		 {
			 g_loginfirst=0;//不获取
			 OutputDebugStringW(_T("\r\ng_loginfirst=0"));
		 }
		 else if(strcmp(c_buf,"jfkjjsb")==0)
		 {
			 g_logintype=0;//普通
			 g_loginfirst=0;//不获取
			 OutputDebugStringW(_T("\r\ng_logintype=0"));
			 OutputDebugStringW(_T("\r\ng_loginfirst=0"));
		 }*/
		 /* 获取加载IPC库的服务器类型    2014-1-8 TGX                            */
		 _stprintf(striniPath,L"%s\\DataBase.ini",strPath);
		 GetPrivateProfileString(L"StreamNO",L"dhStreamType",L"",str_SeverType.GetBuffer(20),20,striniPath);
		 if (str_SeverType == _T(""))
		 {
			 OutputDebugString(_T("\r\n DHIPCDLLType is 0"));
			 str_SeverType = _T("0");
		 }
		 else
		 {
			 OutputDebugString(_T("\r\n DHIPCDLLType is "));
			 OutputDebugString(str_SeverType);
		 }
		 // LoginEvent = new TEvent(NULL,false,true,"IPCGlobelLoginEvent",false);
		 LoginEvent = CreateEvent(NULL,FALSE,TRUE,L"LoginEvent");		 

		 ret = CLIENT_Init(NULL,(DWORD)2008);
		//  NETSDK_INIT_PARAM lpInitParam;
		  
		//  lpInitParam.nThreadNum =2;
		 //ret = CLIENT_InitEx(DisConnectFunc,(DWORD)2008,&lpInitParam);
		 CLIENT_SetConnectTime(3000,1);   //把以前的2000改为3000


		 WNDCLASS wcs;
		 memset(&wcs,0,sizeof(wcs));
		 wcs.lpfnWndProc = AgentWndProc;
		 wcs.lpszClassName = L"AgentHIKIPCDLL";
		 wcs.hInstance = (HINSTANCE)GetCurrentProcess();
		 RegisterClass(&wcs);
		 AgentWnd = CreateWindow(wcs.lpszClassName,L"",0,0,0,0,0,0,0,wcs.hInstance,NULL);

		 nTimeCount = 0;
		 //2013-10-25
		 srand((unsigned)time(0));
		 DWORD dwTimes = 600+rand()%1800;
		 SetTimer(AgentWnd,1,dwTimes*100,OnTimer);  //3~4分钟刷新标签和状态
		  
		 //////////////////////////////////
		 PTZControlChange[6][0] = DH_PTZ_UP_CONTROL;//向上调整
		 PTZControlChange[7][0] = DH_PTZ_DOWN_CONTROL;//向下调整
		 PTZControlChange[8][0] = DH_PTZ_LEFT_CONTROL;//向左调整
		 PTZControlChange[9][0] = DH_PTZ_RIGHT_CONTROL;//向右调整
		 PTZControlChange[10][0] = DH_PTZ_ZOOM_ADD_CONTROL;//聚焦远
		 PTZControlChange[11][0] = DH_PTZ_ZOOM_DEC_CONTROL;//聚焦近
		 PTZControlChange[12][0] = DH_PTZ_APERTURE_ADD_CONTROL;//光圈大
		 PTZControlChange[13][0] = DH_PTZ_APERTURE_DEC_CONTROL;//光圈小
		 PTZControlChange[14][0] = DH_PTZ_FOCUS_ADD_CONTROL;//焦距长
		 PTZControlChange[15][0] = DH_PTZ_FOCUS_DEC_CONTROL;//焦距短
		 PTZControlChange[16][0] = 3;//雨刷控制
		 PTZControlChange[17][0]  =2;//灯光控制
		 PTZControlChange[18][0]  =4;//风扇开关
		 PTZControlChange[19][0]  =5;//加热器开关
		 PTZControlChange[20][0]  =6;//辅助设备开关
		 PTZControlChange[5][0]   =29;//云台自动旋转
		 PTZControlChange[21][0] = DH_EXTPTZ_LEFTTOP;//左上
		 PTZControlChange[22][0] = DH_EXTPTZ_RIGHTTOP;//右上
		 PTZControlChange[23][0] = DH_EXTPTZ_LEFTDOWN;//左下
		 PTZControlChange[24][0] = DH_EXTPTZ_RIGHTDOWN;//右下
		 PTZControlChange[30][0] = 28;//power on

		 ///////////////////////////////////
		 

		 //启动Time  2013-12-21
		 DWORD   dwThreadId;
		 hThread =::CreateThread(NULL,0,Thread,0,0,&dwThreadId);

		 return ret;
	 }
	 /*DWORD __declspec(dllexport) __stdcall  IPC_HIK_Local_SetConnectTime(DWORD dwWaitTime,DWORD dwTryTimes)
	 {
	 bool ret;
	 ret = NET_DVR_SetConnectTime(dwWaitTime,dwTryTimes);  //设置连接超时时间
	 return ret;
	 }*/
	 DWORD   __stdcall IPC_DH_DVR_Login(char* dwDVRIP,WORD wDVRPort,char* sUserName,char* sPassword,int IPCType,DWORD dwUserData,DVRLoginCallBack lpDVRLoginCallBack)
	 {
		 DWORD dwLogin = 0;
		 ConnectIPCThread* pConnectThread = new ConnectIPCThread();
		 dwLogin = pConnectThread->Init(dwDVRIP,wDVRPort,sUserName,sPassword,IPCType,dwUserData,lpDVRLoginCallBack);
		  Sleep(10);
		 return dwLogin;
		 //   return 0;
	 }
	 DWORD   __stdcall  IPC_DH_Local_CleanUp()
	 {
		 //DWORD ret;
		 
		 //timer句柄 2013-12-21
		 CloseHandle(hThread);

		 //ret = NET_DVR_StopListen();
		 /* if (lp_stuOutParam!=NULL)
		 {
		 delete[] lp_stuOutParam;
		 }*/


		 CLIENT_Cleanup();

		 /************************************************************************/
		 /* 清除设备通道信息 2014-1-8 TGX                                        */
		 CString str_tmp;
		 DEVICEDES* ps = NULL;
		 POSITION pos = m_chanNameMap.GetStartPosition();
		 while (pos != NULL)
		 {
			 m_chanNameMap.GetNextAssoc(pos, str_tmp, (CObject *&)ps);
			 m_chanNameMap.RemoveKey(str_tmp);
			 if (ps != NULL)
			 {
				 delete ps;
				 ps = NULL;
			 }
		 }
		 //2017-08-09
		 NVR_IPC *pa=NULL;
		 pos = m_map_NVR_IPC.GetStartPosition();
		 while (pos != NULL)
		 {
			 m_map_NVR_IPC.GetNextAssoc(pos, str_tmp, (CObject *&)pa);
			 m_map_NVR_IPC.RemoveKey(str_tmp);
			 if (pa != NULL)
			 {
				 delete pa;
				 pa = NULL;
			 }
		 }
		 /************************************************************************/		 
		 
		 return 0;
	 }
	 DWORD    __stdcall IPC_DH_Cheek_State()
	 {
			IDVRUser* pDVRUser;
			int nError; 
			DWORD dw_err,BytesReturned;
			DHDEV_CHANNEL_CFG piccfg;
			DWORD* pBuf = (DWORD*)buf;
			CString		strTemp;
			DEVICEDES lpDeviceInfo;
			DEVICEDES* lpdi = NULL;
			BOOL isUpdate = FALSE;  //判断通道标签是否发生改变
			BOOL b_r=TRUE;
			NET_TIME lpnettime;
			WaitForSingleObject(LoginEvent,500);
			ResetEvent(LoginEvent);
			OutputDebugString(_T("\r\n开始检查设备状态in"));
			try
			{
				std::list<void*>::iterator pUserListIt = pUserList.begin();
				while(pUserListIt!=pUserList.end())
				{
					pDVRUser = (IDVRUser*)(*pUserListIt);
					if(pDVRUser!=NULL)
					{
						isUpdate = FALSE;
						strTemp.Format(_T("%ld"), pDVRUser->lUserID);
						OutputDebugString(_T("\r\n<<<<设备名:"));
						OutputDebugString(strTemp);
						if (!m_chanNameMap.Lookup(strTemp, (CObject *&)lpdi))
						{
							lpdi = NULL;
						}
						ZeroMemory(&lpDeviceInfo,sizeof(DEVICEDES));
						if ((pDVRUser->dwTypeID==201 || pDVRUser->dwTypeID==501))//nvr
						{
							CString	strbug;
							char	tempDeivece[128];
							int  pppo=0;
							dw_err=0;
							//-----------------------------------------
							if(!CLIENT_QueryDeviceTime(pDVRUser->lUserID,&lpnettime,5000))
							{
								dw_err = CLIENT_GetLastError();   
							}
							if (dw_err == 0x90002008 ||dw_err == 0x80000025 || dw_err==NET_OPEN_CHANNEL_ERROR)//not online
							{
								pDVRUser->lpFMessCallBack(pDVRUser,0,NULL,0);
								(*pUserListIt)=NULL;	
								/************************************************************************/
								/* 设备掉线清楚通道信息  2014-1-8 TGX                                   */
								CString str_ID;
								DEVICEDES* pd = NULL;
								str_ID.Format(_T("%ld"), pDVRUser->lUserID);
								if (m_chanNameMap.Lookup(str_ID, (CObject *&)pd))
								{
									m_chanNameMap.RemoveKey(str_ID);
									if (pd != NULL)
									{
										delete pd;
										pd = NULL;
									}
								}
								//2017-08-09
								NVR_IPC *pa=NULL;
								if(m_map_NVR_IPC.Lookup(str_ID,(CObject *&)pa))
								{
									m_map_NVR_IPC.RemoveKey(str_ID);
									if (pa != NULL)
									{
										delete pa;
										pa = NULL;
									}
								}

							}else if(str_SeverType == _T("33") || str_SeverType == _T("333"))//2014-3-27)
							{
								for (int i=0; i<128; i++)
								{								
									sprintf_s(lpDeviceInfo.VideoLable[i],"%s","无设备");		//表示没有获取到名称					
								}
								//-----------------------------------------------
								char lp_stuOutParam[H_BufLen];
								memset(lp_stuOutParam, 0, H_BufLen); 
								if (CLIENT_GetNewDevConfig(pDVRUser->lUserID, CFG_CMD_VIDEOINDEVGROUP, 0, lp_stuOutParam, H_BufLen, &nError,3000))
								{
									CFG_VIDEOINDEVGROUP_INFO stuDevs = {0};
									if (CLIENT_ParseData(CFG_CMD_VIDEOINDEVGROUP, lp_stuOutParam, &stuDevs, sizeof(stuDevs), NULL))
									{
										memset(lp_stuOutParam, 0, H_BufLen); 
										//lpDeviceInfo.VideoNum = stuDevs.nVideoDevNum;
										for (int i=0; i<stuDevs.nVideoDevNum; i++)
										{								
											const CFG_VIDEOINDEV_INFO& stuDev = stuDevs.stuVideoInDevInfo[i];
											strTemp = stuDev.szDevID;								
											strTemp = strTemp.Mid(4);
											//0-无设备 1-成功 2-不成功
											if(stuDev.byStatus ==1)
											{
												if(CLIENT_GetNewDevConfigForWeb(pDVRUser->lUserID, WEB_CFG_CMD_ENCODE, _ttoi(strTemp)-1, lp_stuOutParam, H_BufLen, &nError,3000) )//2013-10-22       
												{
													strbug.Format(_T("%S"),lp_stuOutParam);
													pppo = strbug.Find(_T("Name"));
													if (pppo>0)
													{
														strbug = strbug.Mid(pppo+9);
														pppo = strbug.Find(_T("\","));  
														if (pppo>0)
														{
															strbug = strbug.Left(pppo);
															if (strbug!=""&&strbug!=" "&&strbug!="  ")
															{
																sprintf_s(lpDeviceInfo.VideoLable[_ttoi(strTemp)-1],"%S",strbug);
																ZeroMemory(tempDeivece,128);
																Change_Utf8_Unicode((unsigned char * )lpDeviceInfo.VideoLable[_ttoi(strTemp)-1], tempDeivece);
																memcpy(lpDeviceInfo.VideoLable[_ttoi(strTemp)-1],tempDeivece,32*sizeof(char));
															}
														}						
													}									
												}
											}
											if(stuDev.byStatus ==2)
											{
												sprintf_s(lpDeviceInfo.VideoLable[_ttoi(strTemp)-1],"%s","OFF");
											}
										}//end	
									}//if
									pDVRUser->lpFMessCallBack(pDVRUser,2,(char*)&lpDeviceInfo,sizeof(DEVICEDES));//回调刷新
								}//if
							}											
						}
						//---------------不是6000------------------------------------------------------------------------
						if(pDVRUser->dwTypeID!=201 && pDVRUser->dwTypeID!=501)//dvr .ipc
						{
							char *szOutBuffer = new char[32*1024];
							if(NULL == szOutBuffer)
							{
								return 0;
							}
							dw_err=0;
							if(!CLIENT_QueryDeviceTime(pDVRUser->lUserID,&lpnettime,5000))
							{
								dw_err = CLIENT_GetLastError();   
							}
							if (dw_err == 0x90002008 ||dw_err == 0x80000025 || dw_err==NET_OPEN_CHANNEL_ERROR)//not online
							{
								pDVRUser->lpFMessCallBack(pDVRUser,0,NULL,0);
								(*pUserListIt)=NULL;	
								/************************************************************************/
								/* 设备掉线清楚通道信息  2014-1-8 TGX                                   */
								CString str_ID;
								DEVICEDES* pd = NULL;
								str_ID.Format(_T("%ld"), pDVRUser->lUserID);
								if (m_chanNameMap.Lookup(str_ID, (CObject *&)pd))
								{
									m_chanNameMap.RemoveKey(str_ID);
									if (pd != NULL)
									{
										delete pd;
										pd = NULL;
									}
								}
								//2017-08-09
								NVR_IPC *pa=NULL;
								if(m_map_NVR_IPC.Lookup(str_ID,(CObject *&)pa))
								{
									m_map_NVR_IPC.RemoveKey(str_ID);
									if (pa != NULL)
									{
										delete pa;
										pa = NULL;
									}
								}
								/************************************************************************/
							}					
							else if(str_SeverType == _T("3") || str_SeverType == _T("33")|| str_SeverType == _T("333"))//2014-3-27)*/
							{
								DHDEV_VIRTUALCAMERA_STATE_INFO stCameraState = {sizeof(DHDEV_VIRTUALCAMERA_STATE_INFO)};
								int m = 16;
								if (lpdi != NULL)
								{
									m = lpdi->VideoNum;
								}
								for (int i=0;i<m;i++)
								{
									lpDeviceInfo.VideoNum = i+1;
									//CLIENT_GetDevConfig(pDVRUser->lUserID,DH_DEV_CHANNELCFG,i,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned,1000);
									//2014-12-11  查询IPC的状态
									if(lpdi->AlarmNum==28 || lpdi->AlarmNum==31)//n7
									{
										stCameraState.nChannelID = i;
										b_r = CLIENT_QueryDevState(pDVRUser->lUserID,DH_DEVSTATE_VIRTUALCAMERA,(char *)(&stCameraState), sizeof(DHDEV_VIRTUALCAMERA_STATE_INFO),&nError,3000);
									}else
									{
										b_r = FALSE;
									}
									if(b_r == TRUE && stCameraState.emConnectState!=2)
									{
										//是IPC 通道
										sprintf_s(lpDeviceInfo.VideoLable[i],"%s","OFF");//2011-08-26
									}else
									{
										GetOneChannelName(&m_pstChannelName[i],i,pDVRUser->lUserID);
										char szBuf[AV_CFG_Channel_Name_Len + 1] = {0};
										memcpy(szBuf, m_pstChannelName[i].szName, AV_CFG_Channel_Name_Len);
										if (strcmp(szBuf,"") ==0)//2011-09-01
										{
											sprintf_s(lpDeviceInfo.VideoLable[i],"%s%d","通道",i+1);//2011-08-26							
										}
										else
										{
											strcpy(lpDeviceInfo.VideoLable[i],szBuf);
										}
									}				
								}
								//2018-05-26
								if(pDVRUser->dwTypeID==2014)
								{
									DH_IN_MATRIX_GET_CAMERAS stInParam = {sizeof(stInParam)};
									DH_OUT_MATRIX_GET_CAMERAS stOutParam = {sizeof(stOutParam)};
									stOutParam.nMaxCameraCount = lpDeviceInfo.VideoNum;                            // 设置需要获取的最大前端个数
									stOutParam.pstuCameras = new DH_MATRIX_CAMERA_INFO[stOutParam.nMaxCameraCount];
									memset(stOutParam.pstuCameras,0,stOutParam.nMaxCameraCount*sizeof(DH_MATRIX_CAMERA_INFO));
									for (int i = 0; i < stOutParam.nMaxCameraCount; i++)
									{
										stOutParam.pstuCameras[i].dwSize = sizeof(DH_MATRIX_CAMERA_INFO);
										stOutParam.pstuCameras[i].stuRemoteDevice.dwSize = sizeof(DH_REMOTE_DEVICE);
									}
									BOOL bRet =  CLIENT_MatrixGetCameras(pDVRUser->lUserID,&stInParam,&stOutParam,5000);
									// 打印前端登陆信息
									if(bRet)
									{													
										for (int i = 0; i < __min(stOutParam.nRetCameraCount,stOutParam.nMaxCameraCount); i++)
										{
											//2018-05-26
											if (strcmp(stOutParam.pstuCameras[i].stuRemoteDevice.szIp,"192.168.0.0")==0)//没有设备
											{
												strcpy(lpDeviceInfo.VideoLable[i],"无设备");
											}
										}
									}
								}
								//end
								for (int i=0;i<m;i++)
								{
									if (lpdi != NULL)
									{
										if (strcmp(lpdi->VideoLable[i], lpDeviceInfo.VideoLable[i]) != 0)
										{
											strcpy(lpdi->VideoLable[i], lpDeviceInfo.VideoLable[i]);
											isUpdate = TRUE;
										}
									}
									else
										isUpdate = TRUE;
								}
								//////////////////////////////////////////////////////////////////////////
								if (isUpdate)
									pDVRUser->lpFMessCallBack(pDVRUser,2,(char*)&lpDeviceInfo,sizeof(DEVICEDES));//回调刷新
							}
							//-------------------------------
							if(szOutBuffer)
							{
								delete[] szOutBuffer;
								szOutBuffer = NULL;
							}
						}
						OutputDebugString(_T("\r\n设备名:"));
						OutputDebugString(strTemp);
						OutputDebugString(_T(">>>>"));
					}
					pUserListIt++;
				}
	loop:			pUserListIt = pUserList.begin(); //int pUserListCount=0;
				while(pUserListIt!=pUserList.end())
				{
					if((*pUserListIt)==NULL)
					{
						pUserList.erase(pUserListIt);
						goto loop;
					}
					pUserListIt++;
				}
				nTimeCount = 0;
				SetEvent(LoginEvent);
				OutputDebugString(_T("\r\n开始检查设备状态out"));
			}
			catch(...)
			{
				SetEvent(LoginEvent);
				OutputDebugString(_T("\r\n 为什么断线会掉会发生异常了！！！"));
			}
	}
 };