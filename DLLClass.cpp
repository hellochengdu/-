// ---------------------------------------------------------------------------
#include "stdafx.h"
#pragma hdrstop
#include "DLLClass.h"
#include "time.h"
#include <MMSystem.h>

//#include "DHIPCDLL.h"
//#include "StreamConvertor.h"
#include <list>
int PTZControlChange[64][1];

//TList *RealPlayList = new TList; // 播放列表
std::list<void*> RealPlayList;
//TList *DVRList = new TList; // 主机列表
std::list<void*> DVRList;
//TList *ControlList = new TList; // 控制列表
std::list<void*> ControlList;
IRealPlay* _RealPlay = NULL;
IHikRealPlay HikRealPlay[MAX_NUM_DH]; // 
IHikPlayBack _PlayBack;
//2011-10-11
CMapWordToOb m_map_playback;


extern HANDLE AlarmEvent;  //2013/1/18
extern HWND AgentWnd;    //2013/1/18
extern VOID CALLBACK OnTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);      //2013/1/18
extern CMapStringToOb m_chanNameMap;
extern CString str_SeverType;
extern CMapStringToOb m_map_NVR_IPC;
HANDLE m_hRealPlay = CreateEvent(NULL,TRUE,TRUE,NULL);   //20130509 增加拉流互斥
//
extern void GetOneChannelName(AV_CFG_ChannelName *pstChannelName, int nCurChannel,long m_LoginID);
extern AV_CFG_ChannelName *m_pstChannelName;

//监听回调，2011-12-14  glfu
BOOL CALLBACK MessageCallBack(LONG  lCommand,LONG  lLoginID,char  *pBuf,DWORD dwBufLen,char  *pchDVRIP,LONG  nDVRPort,DWORD dwUser)
{
	CString tempIP;
	tempIP.Format(_T("\r\n alarm LMT ip: %S"),pchDVRIP);
	//	OutputDebugString(tempIP);
	//NET_CLIENT_STATE clientState;		// 普通报警信息
	//memset(&clientState,0,sizeof(NET_CLIENT_STATE));
	int myFlag = 0 ;
	int i = 0;
	DWORD diskAlarm = 0;
	DWORD *p = NULL ;
	char buf[256] = {0};
	DWORD alarmType = 101;
	DWORD byChannel = 0;
	DWORD backChannel = 0;
	DWORD dwTimePre = 0;  //获取当前时间，和上一次相减，结果<100的不发送

	ALARM_FRONTDISCONNET_INFO lp_ALARM_FRONTDISCONNET_INFO;
	ZeroMemory(&lp_ALARM_FRONTDISCONNET_INFO,sizeof(lp_ALARM_FRONTDISCONNET_INFO));
	ALARM_REMOTE_ALARM_INFO lp_ALARM_REMOTE_ALARM_INFO;
	ZeroMemory(&lp_ALARM_REMOTE_ALARM_INFO,sizeof(lp_ALARM_REMOTE_ALARM_INFO));

	WaitForSingleObject(AlarmEvent,5000);
 	ResetEvent(AlarmEvent);
	DVRListNode *DVRNode;
	std::list<void*>::iterator DVRListIt = DVRList.begin();
	while(DVRListIt!=DVRList.end())
	{
		DVRNode =(DVRListNode*)(*DVRListIt);
		if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
		{
			if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 
			{
				switch(lCommand)
				{	
				case DH_COMM_ALARM:					//  数据为一个NET_CLIENT_STATE结 构体:4352

					break;
				case DH_SHELTER_ALARM:       // 数据为16个字节，每个字节表示一个视频通道的遮挡报警状态，1为有报警，0为无报警:4353

					break;
				case DH_ALARM_ALARM_EX://外部报警
					alarmType = 104;
					memcpy_s(buf,sizeof(BYTE)*256,pBuf,dwBufLen);
					for (int i=0;i<dwBufLen;i++)
					{
						if(buf[i] == 1 && DVRNode->redalarm[i]<1)
						{							
							DVRNode->redalarm[i] =1;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", i);
							tempIP.Format(_T("\r\nDH外部报警开始ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}
						else if(buf[i] == 1 && DVRNode->redalarm[i] == 1)
						{
								DVRNode->redalarm[i] =1;
						}
						else if(buf[i] == 0 && DVRNode->redalarm[i] == 1)	
						{
							DVRNode->redalarm[i] =0;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", i);
							tempIP.Format(_T("\r\nDH外部报警结束ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}
						
					}

					//		memcpy_s(buf,sizeof(BYTE)*16,pBuf,sizeof(BYTE)*16);
					//		while(DVRListIt!=DVRList.end())
					//		{
					//			DVRNode =(DVRListNode*)(*DVRListIt);
					//			if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
					//			{
					//				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 
					//				{
					//					alarmType = 104;  //外部报警
					//					for (i=0;i<16;i++)
					//					{
					//						if (buf[i]==1)
					//						{
					//							byChannel |= (0x01<<i);
					//						}
					//					}			
					//// 					WaitForSingleObject(AlarmEvent,INFINITE);
					//// 					ResetEvent(AlarmEvent);
					//					backChannel |= (byChannel<<16);
					//					backChannel |= (((DVRNode->pDVRUser)->preAlarm^byChannel)& ((DVRNode->pDVRUser)->preAlarm));
					//					backChannel = ( backChannel & ((((DVRNode->pDVRUser)->preAlarm<<16) | 0x0000FFFF)) ) ^  (byChannel<<16);  //2012/12/4 去掉上次已经发过的报警通道，发送新的通道报警或是其他通道结束报警
					//					if ((DVRNode->pDVRUser)->preAlarm!=byChannel)
					//					{
					//						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, pBuf, backChannel);
					//						tempIP.Format(_T("\r\nDH外部报警ip=%S,通道号=%lu"),pchDVRIP,backChannel);
					//						OutputDebugString(tempIP);
					//					}
					//					(DVRNode->pDVRUser)->preAlarm = byChannel;
					////					SetEvent(AlarmEvent);
					//					break;
					//				}
					//			}
					//			DVRListIt++;
					//		}
					break;
				case DH_MOTION_ALARM_EX:      //数据为16个字节，每个字节表示一个视频通道的动态检测报警状态，1为有报警，0为无报警。	
					alarmType = 101;
					memcpy_s(buf,sizeof(BYTE)*256,pBuf,dwBufLen);
					for (int i=0;i<dwBufLen;i++)
					{
						if(buf[i] == 1 && DVRNode->movealarm[i]<1)
						{							
							DVRNode->movealarm[i] =1;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", i);
							tempIP.Format(_T("\r\nDH移动侦测报警开始ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}
						else if(buf[i] == 1 && DVRNode->movealarm[i] == 1)
						{
							DVRNode->movealarm[i] =1;
						}
						else if(buf[i] == 0 && DVRNode->movealarm[i] == 1)	
						{
							DVRNode->movealarm[i] =0;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", i);
							tempIP.Format(_T("\r\nDH移动侦测报警结束ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}

					}
					//		memcpy_s(buf,sizeof(BYTE)*16,pBuf,sizeof(BYTE)*16);
					//		while(DVRListIt!=DVRList.end())
					//		{
					//			DVRNode =(DVRListNode*)(*DVRListIt);
					//			if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
					//			{
					//				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 
					//				{
					//					alarmType = 101;		//移动侦测
					//					for (i=0;i<16;i++)
					//					{
					//						if (buf[i]==1)
					//						{
					//							byChannel |= (0x01<<i);
					//						}
					//					}			
					//// 					WaitForSingleObject(AlarmEvent,INFINITE);
					//// 					ResetEvent(AlarmEvent);
					//					backChannel |= (byChannel<<16);
					//					backChannel |= (((DVRNode->pDVRUser)->preMotion^byChannel)&(DVRNode->pDVRUser)->preMotion);
					//					dwTimePre = ::timeGetTime();
					//					if ((DVRNode->pDVRUser)->preMotion!=byChannel && (dwTimePre - (DVRNode->pDVRUser)->preMoTimer)>500)
					//					{
					//						backChannel = ( backChannel & ((((DVRNode->pDVRUser)->preMotion<<16) | 0x0000FFFF)) ) ^  (byChannel<<16);  //2012/12/4 去掉上次已经发过的报警通道，发送新的通道报警或是其他通道结束报警
					//						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, pBuf, backChannel);
					//						tempIP.Format(_T("\r\nDH移动侦测ip=%S,通道号=%lu"),pchDVRIP,backChannel);
					//						OutputDebugString(tempIP);
					//					}
					//					(DVRNode->pDVRUser)->preMoTimer = dwTimePre;
					//					(DVRNode->pDVRUser)->preMotion = byChannel;
					////					SetEvent(AlarmEvent);
					//					break;
					//				}
					//			}
					//			DVRListIt++;
					//		}
					break;
				case DH_VIDEOLOST_ALARM_EX:				//数据为16个字节，每个字节表示一个视频通道的视频丢失报警状态，1为有报警，0为无报警
					alarmType = 102;
					memcpy_s(buf,sizeof(BYTE)*256,pBuf,dwBufLen);
					for (int i=0;i<dwBufLen;i++)
					{
						if(buf[i] == 1 && DVRNode->lostsigal[i]<1)
						{							
							DVRNode->lostsigal[i] =1;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", i);
							tempIP.Format(_T("\r\nDH视频丢失报警开始ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}
						else if(buf[i] == 1 && DVRNode->lostsigal[i] == 1)
						{
							DVRNode->lostsigal[i] =1;
						}
						else if(buf[i] == 0 && DVRNode->lostsigal[i] == 1)	
						{
							DVRNode->lostsigal[i] =0;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", i);
							tempIP.Format(_T("\r\nDH视频丢失报警结束ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}

					}
					//		OutputDebugString(_T("\n\tListening!"));
					//		memcpy_s(buf,sizeof(BYTE)*16,pBuf,sizeof(BYTE)*16);
					//		while(DVRListIt!=DVRList.end())
					//		{
					//			DVRNode =(DVRListNode*)(*DVRListIt);
					//			if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
					//			{
					//				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 				
					//				{
					//					alarmType = 102;				//视频丢失
					//					for (i=0;i<16;i++)
					//					{
					//						if (buf[i]==1)
					//						{
					//							byChannel |= (0x01<<(i));
					//						}
					//					}
					//// 					WaitForSingleObject(AlarmEvent,INFINITE);
					//// 					ResetEvent(AlarmEvent);
					//					backChannel |= (byChannel<<16);
					//					backChannel |= (((DVRNode->pDVRUser)->preViLost^byChannel)&(DVRNode->pDVRUser)->preViLost);
					//					backChannel = ( backChannel & ((((DVRNode->pDVRUser)->preViLost<<16) | 0x0000FFFF)) ) ^  (byChannel<<16);  //2012/12/4 去掉上次已经发过的报警通道，发送新的通道报警或是其他通道结束报警
					//					if ((DVRNode->pDVRUser)->preViLost != byChannel)
					//					{
					//						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, pBuf, backChannel);
					//						tempIP.Format(_T("\r\nDH视频丢失ip=%S,通道号=%lu"),pchDVRIP,backChannel);
					//						OutputDebugString(tempIP);
					//					}
					//					(DVRNode->pDVRUser)->preViLost = byChannel;		
					////					SetEvent(AlarmEvent);
					//					break;
					//				}
					//			}
					//			DVRListIt++;
					//		}
					break;
				case DH_SHELTER_ALARM_EX:						//数据为16个字节，每个字节表示一个视频通道的遮挡（黑屏）报警状态，1为有报警，0为无报警
					alarmType = 103;
					memcpy_s(buf,sizeof(BYTE)*256,pBuf,dwBufLen);
					for (int i=0;i<dwBufLen;i++)
					{
						if(buf[i] == 1 && DVRNode->maskalarm[i]<1)
						{							
							DVRNode->maskalarm[i] =1;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", i);
							tempIP.Format(_T("\r\nDH视频遮挡报警开始ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}
						else if(buf[i] == 1 && DVRNode->maskalarm[i] == 1)
						{
							DVRNode->maskalarm[i] =1;
						}
						else if(buf[i] == 0 && DVRNode->maskalarm[i] == 1)	
						{
							DVRNode->maskalarm[i] =0;
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", i);
							tempIP.Format(_T("\r\nDH视频遮挡报警结束ip=%S,通道号=%lu"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}

					}
					//		OutputDebugString(_T("\n\tListening!"));
					//		memcpy(buf,pBuf,sizeof(BYTE)*16);
					//		while(DVRListIt!=DVRList.end())
					//		{
					//			DVRNode =(DVRListNode*)(*DVRListIt);
					//			if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
					//			{
					//				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 
					//				{
					//					alarmType = 103;		//视频遮挡
					//					for (i=0;i<16;i++)
					//					{
					//						if (buf[i]==1)
					//						{
					//							byChannel |= (0x01<<(i));
					//						}
					//					}
					//// 					WaitForSingleObject(AlarmEvent,INFINITE);
					//// 					ResetEvent(AlarmEvent);
					//					backChannel |= (byChannel<<16);
					////					backChannel = backChannel^((DVRNode->pDVRUser)->preShelter)<<16; //  2012/12/4 去掉上次已经发过的报警通道
					//					backChannel |= (((DVRNode->pDVRUser)->preShelter^byChannel)&(DVRNode->pDVRUser)->preShelter);
					//					backChannel = ( backChannel & ((((DVRNode->pDVRUser)->preShelter<<16) | 0x0000FFFF)) ) ^  (byChannel<<16);  //2012/12/4 去掉上次已经发过的报警通道，发送新的通道报警或是其他通道结束报警
					//					if ((DVRNode->pDVRUser)->preShelter != byChannel)
					//					{
					//						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, pBuf, backChannel);
					//						tempIP.Format(_T("\r\nDH视频遮挡ip=%S,通道号=%lu"),pchDVRIP,backChannel);
					//						OutputDebugString(tempIP);
					//					}
					//					(DVRNode->pDVRUser)->preShelter = byChannel;
					////					SetEvent(AlarmEvent);
					//					break;
					//				}
					//			}
					//			DVRListIt++;
					//		}
					break;
				case DH_SOUND_DETECT_ALARM_EX:			//数据为16个字节，每个字节表示一个视频通道的音频检测报警状态，1为有报警，0为无报警
					break;
				case DH_DISKFULL_ALARM_EX:			//数据为1个字节，1为有硬盘满报警，0为无报警。
					alarmType = 105;
					memcpy_s(buf,sizeof(BYTE)*256,pBuf,dwBufLen);
					//for (int i=0;i<dwBufLen;i++)
					//{
						//if(buf[i] == 1 && DVRNode->maskalarm[i]<1)
					   if(buf[0] ==1)
						{					
							
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", -1);
							tempIP.Format(_T("\r\nDH硬盘满报警ip=%S"),pchDVRIP);
							//OutputDebugString(tempIP);
						}
						

					//}
					//		memcpy(buf,pBuf,sizeof(BYTE)*16);
					//		while(DVRListIt!=DVRList.end())
					//		{
					//			DVRNode =(DVRListNode*)(*DVRListIt);
					//			if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
					//			{
					//				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 
					//				{
					//// 					WaitForSingleObject(AlarmEvent,INFINITE);
					//// 					ResetEvent(AlarmEvent);
					//					alarmType = 105;		//硬盘满报警
					//					byChannel = 0;
					//					byChannel = /**(DWORD*)*/*pBuf;
					//					backChannel |= (byChannel<<16);
					//					backChannel |= (((DVRNode->pDVRUser)->preDiskFull^byChannel)&(DVRNode->pDVRUser)->preDiskFull);
					//					if ((DVRNode->pDVRUser)->preDiskFull != byChannel)
					//					{
					//						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, pBuf, backChannel);
					//						tempIP.Format(_T("\r\nDH硬盘满报警ip=%S,是否开始=%lu"),pchDVRIP,backChannel);
					//						OutputDebugString(tempIP);
					//					}
					//					(DVRNode->pDVRUser)->preDiskFull = byChannel;
					////					SetEvent(AlarmEvent);
					//					break;
					//				}
					//			}
					//			DVRListIt++;
					//		}
					break;
				case DH_DISKERROR_ALARM_EX:				//数据为32个字节，每个字节表示一个硬盘的故障报警状态，1为有报警，0为无报警
					alarmType = 106;
					memcpy_s(buf,sizeof(BYTE)*256,pBuf,dwBufLen);
					for (int i=0;i<dwBufLen;i++)
					{
						if(buf[i] == 1)					
						{					
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", i+1);
							tempIP.Format(_T("\r\nDH硬盘故障ip=%S,硬盘位置为:%d"),pchDVRIP,i+1);
							//OutputDebugString(tempIP);
						}
					}
					//		memcpy(buf,pBuf,sizeof(BYTE)*16);
					//		while(DVRListIt!=DVRList.end())
					//		{
					//			DVRNode =(DVRListNode*)(*DVRListIt);
					//			if (strcmp(pchDVRIP, DVRNode->sDVRIP) == 0) 
					//			{
					//				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) 
					//				{
					//					alarmType = 106;		//硬盘故障
					//					for (i=0;i<16;i++)
					//					{
					//						if (buf[i]==1)
					//						{
					//							byChannel |= (0x01<<(i));
					//						}
					//					}
					//// 					WaitForSingleObject(AlarmEvent,INFINITE);
					//// 					ResetEvent(AlarmEvent);
					//					backChannel |= (byChannel<<16);
					//					backChannel |= (((DVRNode->pDVRUser)->preHardDisk^byChannel)&(DVRNode->pDVRUser)->preHardDisk);
					//					if ((DVRNode->pDVRUser)->preHardDisk != byChannel)
					//					{
					//						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, pBuf, backChannel);
					//						tempIP.Format(_T("\r\nDH硬盘故障ip=%S,故障个数=%lu"),pchDVRIP,backChannel);
					//						OutputDebugString(tempIP);
					//					}
					//					(DVRNode->pDVRUser)->preHardDisk = byChannel;
					////					SetEvent(AlarmEvent);
					//					break;
					//				}
					//			}
					//			DVRListIt++;
					//		}
					break;
				case DH_ENCODER_ALARM_EX:			//数据为16个字节，每个字节表示一个通道编码器状态，1为有报警，0为无报警。
					break;
				case DH_ALARM_STORAGE_FAILURE:

					break;
				case DH_ALARM_FRONTDISCONNECT://ipc 断网  // 通道号，从1开始
					alarmType = 107;
					memcpy_s(&lp_ALARM_FRONTDISCONNET_INFO,sizeof(lp_ALARM_FRONTDISCONNET_INFO),pBuf,sizeof(lp_ALARM_FRONTDISCONNET_INFO));
					if (lp_ALARM_FRONTDISCONNET_INFO.nAction == 0)//开始
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_FRONTDISCONNET_INFO.nChannelID-1);
						tempIP.Format(_T("\r\nDH IPC断网报警开始ip=%S,IPC通道=%lu"),pchDVRIP,lp_ALARM_FRONTDISCONNET_INFO.nChannelID);
						OutputDebugString(tempIP);
					}
					else
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_FRONTDISCONNET_INFO.nChannelID-1);
						tempIP.Format(_T("\r\nDH IPC断网报警结束ip=%S,IPC通道=%lu"),pchDVRIP,lp_ALARM_FRONTDISCONNET_INFO.nChannelID);
						OutputDebugString(tempIP);

					}
					break;
				case DH_ALARM_ALARM_EX_REMOTE: // ipc 报警 // 通道号，从1开始
					alarmType = 108;
					memcpy_s(&lp_ALARM_REMOTE_ALARM_INFO,sizeof(lp_ALARM_REMOTE_ALARM_INFO),pBuf,sizeof(lp_ALARM_REMOTE_ALARM_INFO));
					if (lp_ALARM_REMOTE_ALARM_INFO.nChannelID>0)
					{
						if (DVRNode->pDVRUser->dwTypeID != 228 && DVRNode->pDVRUser->dwTypeID != 227 && DVRNode->pDVRUser->dwTypeID != 503 && DVRNode->pDVRUser->dwTypeID != 504)//不是IPC,反过来
						{
							if (lp_ALARM_REMOTE_ALARM_INFO.nState ==0)
							{
								lp_ALARM_REMOTE_ALARM_INFO.nState =1;						
							}
							else
							{
								lp_ALARM_REMOTE_ALARM_INFO.nState =0;
							}
						}
						if (lp_ALARM_REMOTE_ALARM_INFO.nState == 0)//开始
						{
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_REMOTE_ALARM_INFO.nChannelID-1);
							tempIP.Format(_T("\r\nDH IPC外部报警开始ip=%S,IPC通道=%lu"),pchDVRIP,lp_ALARM_REMOTE_ALARM_INFO.nChannelID);
							OutputDebugString(tempIP);
						}
						else
						{
							(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_REMOTE_ALARM_INFO.nChannelID-1);
							tempIP.Format(_T("\r\nDH IPC外部报警结束ip=%S,IPC通道=%lu"),pchDVRIP,lp_ALARM_REMOTE_ALARM_INFO.nChannelID);
							OutputDebugString(tempIP);

						}
					}
					
					break;

				//2016-11-14 新增报警类型
				case DH_ALARM_STORAGE_FAILURE_EX://N7,IPC存储卡异常报警事件
					alarmType = 111;
					ALARM_STORAGE_FAILURE_EX lp_ALARM_STORAGE_FAILURE_EX;
					memcpy_s(&lp_ALARM_STORAGE_FAILURE_EX,sizeof(ALARM_STORAGE_FAILURE_EX),pBuf,sizeof(ALARM_STORAGE_FAILURE_EX));
					if (lp_ALARM_STORAGE_FAILURE_EX.nAction==0)
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_STORAGE_FAILURE_EX.nPhysicNo);
						tempIP.Format(_T("\r\nDH  SD卡报警开始ip=%S,通道 %lu"),pchDVRIP,lp_ALARM_STORAGE_FAILURE_EX.nPhysicNo);
					}
					else
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_STORAGE_FAILURE_EX.nPhysicNo);
						tempIP.Format(_T("\r\nDH  SD卡报警结束ip=%S,通道 %lu"),pchDVRIP,lp_ALARM_STORAGE_FAILURE_EX.nPhysicNo);
					}					
					OutputDebugString(tempIP);
					break;				

				case DH_ALARM_STORAGE_LOW_SPACE://// 存储容量不足事件
					alarmType = 112;
					ALARM_STORAGE_LOW_SPACE_INFO lp_ALARM_STORAGE_LOW_SPACE_INFO;
					memcpy_s(&lp_ALARM_STORAGE_LOW_SPACE_INFO,sizeof(ALARM_STORAGE_LOW_SPACE_INFO),pBuf,sizeof(ALARM_STORAGE_LOW_SPACE_INFO));
					if(lp_ALARM_STORAGE_LOW_SPACE_INFO.nAction ==0)
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1",1);
						tempIP.Format(_T("\r\nDH  存储容量不足报警开始ip=%S,通道 "),pchDVRIP);
					}
					else
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0",0);
						tempIP.Format(_T("\r\nDH  存储容量不足报警结束ip=%S,通道 "),pchDVRIP);
					}
					OutputDebugString(tempIP);
					break;
					
				case DH_ALARM_STORAGE_IPC_FAILURE: //nvr 存储卡异常报警事件从0开始
					alarmType = 111;
					ALARM_STORAGE_IPC_FAILURE_INFO lp_ALARM_STORAGE_IPC_FAILURE_INFO;
					memcpy_s(&lp_ALARM_STORAGE_IPC_FAILURE_INFO,sizeof(ALARM_STORAGE_IPC_FAILURE_INFO),pBuf,sizeof(ALARM_STORAGE_IPC_FAILURE_INFO));
					if (lp_ALARM_STORAGE_IPC_FAILURE_INFO.nAction==0)//开始
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_STORAGE_IPC_FAILURE_INFO.nChannelID);
						tempIP.Format(_T("\r\nDH NVR SD卡报警开始ip=%S,通道 %lu"),pchDVRIP,lp_ALARM_STORAGE_IPC_FAILURE_INFO.nChannelID+1);
					}
					else
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_STORAGE_IPC_FAILURE_INFO.nChannelID);
						tempIP.Format(_T("\r\nDH NVR SD卡报警结束ip=%S,通道 %lu"),pchDVRIP,lp_ALARM_STORAGE_IPC_FAILURE_INFO.nChannelID+1);
					}					
					OutputDebugString(tempIP);
					break;
				
				//case DH_EVENT_CROSSREGION_DETECTION:
				//	ALARM_EVENT_CROSSREGION_INFO lp_ALARM_EVENT_CROSSREGION_INFO;
				//	memcpy_s(&lp_ALARM_EVENT_CROSSREGION_INFO,sizeof(lp_ALARM_EVENT_CROSSREGION_INFO),pBuf,dwBufLen);
				//	(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,114, "1", lp_ALARM_EVENT_CROSSREGION_INFO.nChannelID);
				//	//tempIP.Format(_T("\r\nDH NVR SD卡报警开始ip=%S,通道 %lu"),pchDVRIP,lp_ALARM_STORAGE_IPC_FAILURE_INFO.nChannelID+1);
				//	tempIP.Format(_T("\r\nDH 警戒区报警 %S %d"),pchDVRIP,lp_ALARM_EVENT_CROSSREGION_INFO.nChannelID);
				//	OutputDebugString(tempIP);
				//	break;
				case DH_EVENT_CROSSLINE_DETECTION://警戒线   （绊线入侵）
					alarmType =113;
					ALARM_EVENT_CROSSLINE_INFO lp_ALARM_EVENT_CROSSLINE_INFO;
					memcpy_s(&lp_ALARM_EVENT_CROSSLINE_INFO,sizeof(lp_ALARM_EVENT_CROSSLINE_INFO),pBuf,sizeof(lp_ALARM_EVENT_CROSSLINE_INFO));
					if (lp_ALARM_EVENT_CROSSLINE_INFO.nEventAction!=2)//开始
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_EVENT_CROSSLINE_INFO.nChannelID);
						tempIP.Format(_T("\r\nDH 警戒线报警开始ip=%S %d"),pchDVRIP,lp_ALARM_EVENT_CROSSLINE_INFO.nChannelID+1);
					}
					else//end
					{
						(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_EVENT_CROSSLINE_INFO.nChannelID);
						tempIP.Format(_T("\r\nDH 警戒线报警结束ip=%S %d"),pchDVRIP,lp_ALARM_EVENT_CROSSLINE_INFO.nChannelID+1);
					}					
					
					OutputDebugString(tempIP);
					break;
               case DH_EVENT_CROSSREGION_DETECTION: //警戒区事件         区域入侵           
				   alarmType =114;
				   ALARM_EVENT_CROSSREGION_INFO lp_ALARM_EVENT_CROSSREGION_INFO;
				   memcpy_s(&lp_ALARM_EVENT_CROSSREGION_INFO,sizeof(lp_ALARM_EVENT_CROSSREGION_INFO),pBuf,sizeof(lp_ALARM_EVENT_CROSSREGION_INFO));
				   if (lp_ALARM_EVENT_CROSSREGION_INFO.nEventAction!=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_EVENT_CROSSREGION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 警戒区事件开始ip=%S %d"),pchDVRIP,lp_ALARM_EVENT_CROSSREGION_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_EVENT_CROSSREGION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 警戒区事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_EVENT_CROSSREGION_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
			   case DH_EVENT_LEFT_DETECTION://物品遗留事件
				   alarmType =115;
				   ALARM_EVENT_LEFT_INFO lp_ALARM_EVENT_LEFT_INFO;
				   memcpy_s(&lp_ALARM_EVENT_LEFT_INFO,sizeof(lp_ALARM_EVENT_LEFT_INFO),pBuf,sizeof(lp_ALARM_EVENT_LEFT_INFO));
				   if (lp_ALARM_EVENT_LEFT_INFO.nEventAction!=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_EVENT_LEFT_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 物品遗留事件开始ip=%S %d"),pchDVRIP,lp_ALARM_EVENT_LEFT_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_EVENT_LEFT_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 物品遗留事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_EVENT_LEFT_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
			   case  DH_EVENT_TAKENAWAYDETECTION://物品搬移事件
				   alarmType =116;
				   ALARM_TAKENAWAY_DETECTION_INFO lp_ALARM_TAKENAWAY_DETECTION_INFO;
				   memcpy_s(&lp_ALARM_TAKENAWAY_DETECTION_INFO,sizeof(lp_ALARM_TAKENAWAY_DETECTION_INFO),pBuf,sizeof(lp_ALARM_TAKENAWAY_DETECTION_INFO));
				   if (lp_ALARM_TAKENAWAY_DETECTION_INFO.nEventAction!=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_TAKENAWAY_DETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 物品搬移事件开始ip=%S %d"),pchDVRIP,lp_ALARM_TAKENAWAY_DETECTION_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_TAKENAWAY_DETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 物品搬移事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_TAKENAWAY_DETECTION_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
			   case DH_ALARM_WANDERDETECTION://徘徊事件
				   alarmType =117;
				   ALARM_WANDERDETECTION_INFO lp_ALARM_WANDERDETECTION_INFO;
				   memcpy_s(&lp_ALARM_WANDERDETECTION_INFO,sizeof(lp_ALARM_WANDERDETECTION_INFO),pBuf,sizeof(lp_ALARM_WANDERDETECTION_INFO));
				   if (lp_ALARM_WANDERDETECTION_INFO.nAction !=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_WANDERDETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 徘徊事件开始ip=%S %d"),pchDVRIP,lp_ALARM_WANDERDETECTION_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_WANDERDETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 徘徊事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_WANDERDETECTION_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
			   case DH_ALARM_CROSSFENCEDETECTION://翻越围栏事件
				   alarmType =118;
				   ALARM_CROSSFENCEDETECTION_INFO lp_ALARM_CROSSFENCEDETECTION_INFO;
				   memcpy_s(&lp_ALARM_CROSSFENCEDETECTION_INFO,sizeof(lp_ALARM_CROSSFENCEDETECTION_INFO),pBuf,sizeof(lp_ALARM_CROSSFENCEDETECTION_INFO));
				   if (lp_ALARM_CROSSFENCEDETECTION_INFO.nAction !=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_CROSSFENCEDETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 翻越围栏事件开始ip=%S %d"),pchDVRIP,lp_ALARM_CROSSFENCEDETECTION_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_CROSSFENCEDETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 翻越围栏事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_CROSSFENCEDETECTION_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
			   case  DH_ALARM_MOVEDETECTION://移动事件
				   alarmType =119;
				   ALARM_MOVE_DETECTION_INFO lp_ALARM_MOVE_DETECTION_INFO;
				   memcpy_s(&lp_ALARM_MOVE_DETECTION_INFO,sizeof(lp_ALARM_MOVE_DETECTION_INFO),pBuf,sizeof(lp_ALARM_MOVE_DETECTION_INFO));
				   if (lp_ALARM_MOVE_DETECTION_INFO.nAction !=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_MOVE_DETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 移动事件开始ip=%S %d"),pchDVRIP,lp_ALARM_MOVE_DETECTION_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_MOVE_DETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 移动事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_MOVE_DETECTION_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
			   case DH_ALARM_RIOTERDETECTION://人员聚集事件
				   alarmType =120;
				   ALARM_RIOTERDETECTION_INFO lp_ALARM_RIOTERDETECTION_INFO;
				   memcpy_s(&lp_ALARM_RIOTERDETECTION_INFO,sizeof(lp_ALARM_RIOTERDETECTION_INFO),pBuf,sizeof(lp_ALARM_RIOTERDETECTION_INFO));
				   if (lp_ALARM_RIOTERDETECTION_INFO.nAction !=2)
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "1", lp_ALARM_RIOTERDETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 人员聚集事件开始ip=%S %d"),pchDVRIP,lp_ALARM_RIOTERDETECTION_INFO.nChannelID+1);
				   }
				   else//end
				   {
					   (DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,alarmType, "0", lp_ALARM_RIOTERDETECTION_INFO.nChannelID);
					   tempIP.Format(_T("\r\nDH 人员聚集事件 结束ip=%S %d"),pchDVRIP,lp_ALARM_RIOTERDETECTION_INFO.nChannelID+1);
				   }
				   OutputDebugString(tempIP);
				   break;
				}



				
			}
		}
		DVRListIt++;
	}
	
	
	
	DVRNode = NULL;
	//delete DVRNode;
	SetEvent(AlarmEvent);
	return 0 ;
}

//下载的的进度
//Process status callback
void CALLBACK PlayCallBack(LONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, DWORD dwUser)
{
	if(0 != dwUser)
	{
		IHikDownloadFile *lppDownloadFile = (IHikDownloadFile *)dwUser;
		if(lppDownloadFile->lpReceivePlayPos != NULL)
			lppDownloadFile->lpReceivePlayPos(lppDownloadFile,dwTotalSize,dwDownLoadSize,lppDownloadFile->dwUserData);
	}
}
//按时间下载方式得到下载进度
void CALLBACK lpTimeDownLoadPosCallBack(LONG lPlayHandle, DWORD dwTotalSize, DWORD dwDownLoadSize, int index, NET_RECORDFILE_INFO recordfileinfo, DWORD dwUser)
{
	if(0 != dwUser)
	{
		IHikDownloadFile *lppDownloadFile = (IHikDownloadFile *)dwUser;
		if(lppDownloadFile->lpReceivePlayPos != NULL)
			lppDownloadFile->lpReceivePlayPos(lppDownloadFile,dwTotalSize,dwDownLoadSize,lppDownloadFile->dwUserData);
	}
}
// 历史数据回放
// 海数回调函数
void CALLBACK RealDataCallBack(LONG lRealHandle, DWORD dwDataType,
	BYTE *pBuffer, DWORD dwBufSize, DWORD dwUser) {
		try{
			RealPlayNode* Node = NULL;
			std::list<void*>::iterator RealPlayListIt = RealPlayList.begin();
			while(RealPlayListIt!=RealPlayList.end())
			{
				Node = (RealPlayNode*)(*RealPlayListIt);
				if (Node->lPlayID == lRealHandle) {
					if (Node->pRealPlay->lpREALDATACALLBACK != NULL) {
						Node->pRealPlay->lpREALDATACALLBACK(Node->pRealPlay,
							(char)dwDataType, (char)dwUser, (void*)pBuffer, dwBufSize);
						break;
					}
				}
				RealPlayListIt++;
			}
		}
		catch(...)
		{
			OutputDebugString(_T("\r\n大华数据流回调异常！！！"));
		}

/*	for (int i = 0; i < RealPlayList.size;i++) {
		Node = (RealPlayNode*)RealPlayList.
		if (Node->lPlayID == lRealHandle) {
			if (Node->pRealPlay->lpREALDATACALLBACK != NULL) {
				Node->pRealPlay->lpREALDATACALLBACK(Node->pRealPlay,
					(char)dwDataType, (char)1, (void*)pBuffer, dwBufSize);
				break;
			}
		}
	}*/

}

// 语音数据回调
/*void CALLBACK VoiceDataCallBack(LONG lVoiceComHandle, char *pRecvDataBuffer,
	DWORD dwBufSize, BYTE byAudioFlag, DWORD dwUser)
// 语音对讲回调
{

} */

// DVR信息回调
extern "C" {
	/*int CALLBACK fMessCallBack(LONG lCommand, char *sDVRIP, char *pBuf,
		DWORD dwBufLen) {
		DVRListNode *DVRNode;
		std::list<void*>::iterator DVRListIt = DVRList.begin();
		while(DVRListIt!=DVRList.end())
		{
			DVRNode =(DVRListNode*)(*DVRListIt);
            if (strcmp(sDVRIP, DVRNode->sDVRIP) == 0) // 如果两个IP相同
			{
				if ((DVRNode->pDVRUser)->lpFMessCallBack != NULL) {
					(DVRNode->pDVRUser)->lpFMessCallBack(DVRNode->pDVRUser,
						lCommand, pBuf, dwBufLen);
				}
			}
          DVRListIt++;
		}
	   
		return 0;
	}*/
}
void CALLBACK fDHRealDataCallBack(
								  LONG  lRealHandle,
								  DWORD dwDataType,
								  BYTE  *pBuffer,
								  DWORD dwBufsize,
								  DWORD dwUser
								  )
{
//	try{
		RealPlayNode* Node = NULL;
		std::list<void*>::iterator RealPlayListIt = RealPlayList.begin();
		while(RealPlayListIt!=RealPlayList.end())
		{
			Node = (RealPlayNode*)(*RealPlayListIt);
			if (Node!=NULL &&Node->lPlayID == lRealHandle) 
			{
				if (Node->pRealPlay->lpREALDATACALLBACK != NULL) 
				{
					Node->pRealPlay->lpREALDATACALLBACK(Node->pRealPlay,(char)dwDataType, (char)1, (void*)pBuffer, dwBufsize);
					break;
				}
			}
			RealPlayListIt++;
		}
//	}
// 	catch(...)
// 	{
// 		OutputDebugString(_T("\r\n在拉流查找hash表的时候发生了异常！！！"));
// 	}

/*	RealPlayNode* Node;
	for (int i = 0; i < RealPlayList->Count; i++) {
		Node = (RealPlayNode*)RealPlayList->Items[i];
		if (Node->lPlayID == lRealHandle) {
			if (Node->pRealPlay->lpREALDATACALLBACK != NULL) {
				Node->pRealPlay->lpREALDATACALLBACK(Node->pRealPlay,
					(char)dwDataType, (char)1, (void*)pBuffer, dwBufsize);
				break;
			}
		}
	}*/
}

// -----------------|-----------------------------|------------------------------
// -----------------|       PTZControlList 类     |------------------------------
// -----------------|-----------------------------|------------------------------
// 查找云台控制
DWORD __fastcall FindControl(DWORD dwChannel) {
	Control *control;
	DWORD _dwPriority;
	std::list<void*>::iterator ControlListIt = ControlList.begin();
	while(ControlListIt!=ControlList.end())
	{
		control = (Control*)(*ControlListIt);
		if (control->dwChannel == dwChannel) {
			_dwPriority = control->dwPriority;
			delete control;
			return _dwPriority;
		}
       ControlListIt++;
	}
/*	for (i = 0; i < ControlList->Count; i++) {
		control = (Control*)ControlList->Items[i];
		if (control->dwChannel == dwChannel) {
			_dwPriority = control->dwPriority;
			delete control;
			return _dwPriority;
		}
	}*/
	delete control;
	return -1;

}

// 增加云台控制
bool __fastcall AddPTZControl(DWORD dwChannel, DWORD dwPriority) {
	try {
		Control *control = new Control(dwChannel, dwPriority);
		ControlList.push_front((void*)control);
	//	ControlList->Add(control);
	//	delete control;
	}
	catch(...) {
		return false;
	}
	return true;
}

// 删除云台控制
bool __fastcall DelPTZControl(DWORD dwChannel, DWORD dwPriority) {
	Control *control;
	std::list<void*>::iterator ControlListIt = ControlList.begin();
	while(ControlListIt!=ControlList.end())
	{
		control = (Control*)(*ControlListIt);
		if ((control->dwChannel == dwChannel) &&
			(control->dwChannel == dwPriority)) {
				ControlList.erase(ControlListIt);
			break;
		}
		ControlListIt++;
	}
	/*for (int i = 0; i < ControlList->Count; i++) {
		control = (Control*)ControlList->Items[i];
		if ((control->dwChannel == dwChannel) &&
			(control->dwChannel == dwPriority)) {
			ControlList->Delete(i);
			break;
		}
	}*/
	delete control;
	return true;
}

// ----------|----------------------------|---------------
// ----------|      IPlayBack类           |---------------
// ----------|----------------------------|---------------
// 回放控制
//DWORD __stdcall IHikPlayBack::PlayBackControl(DWORD dwControlCode,
//	DWORD dwInValue, DWORD *lpOutValue) {
//	bool ret;
//	ret = NET_DVR_PlayBackControl(lPlayBackID, dwControlCode, dwInValue,
//		lpOutValue);
//	return ret;
//}


// ----------|----------------------------|---------------
// ----------|      IIVoiceComPlay类      |---------------
// ----------|----------------------------|---------------
// 设置音量
DWORD IHikIVoiceCom::SetVolume(WORD wVol) {
	BOOL ret;
	//ret = NET_DVR_SetVoiceComClientVolume(lVoiceID, wVol);
	return ret;
}

// 退出
void IHikIVoiceCom::Release() {
	//NET_DVR_StopVoiceCom(lVoiceID);
}
//向设备发送数据  fgl 2012、12、25
LONG IHikIVoiceCom::TalkSendData(char* lpInBuf,DWORD dwBufSize)
{
	LONG lSendSize = 0;
	lSendSize = CLIENT_TalkSendData(lVoiceID,lpInBuf,dwBufSize);
	if(lSendSize==-1) return 0;
	else return lSendSize;
}
BOOL IHikIVoiceCom::RecordStart()
{
	CString csTemp;
	BOOL bret = FALSE;
	bret = CLIENT_RecordStart();
	if(bret == FALSE)
	{
		csTemp.Format(_T("\r\n 启动本地采集错误代码%lu"),CLIENT_GetLastError());
		OutputDebugString(csTemp);
	}
	return bret;
}

// ----------|----------------------------|---------------
// ----------|         IRealPlay类        |---------------
// ----------|----------------------------|---------------
// 设置缓冲大小
DWORD __stdcall IHikRealPlay::SetPlayerBufNumber(DWORD dwBufNum) {
	BOOL ret;
	//ret = NET_DVR_SetPlayerBufNumber(lPlayID, dwBufNum);
	return ret;
}

// 设置抛帧数目
DWORD __stdcall IHikRealPlay::ThrowBFrame(DWORD dwNum) {
	BOOL ret;
	//ret = NET_DVR_ThrowBFrame(lPlayID, dwNum);
	return ret;
}

// 设置图像色彩
DWORD __stdcall IHikRealPlay::SetVideoEffect(DWORD dwBrightValue,
	DWORD dwContrastValue, DWORD dwSaturationValue, DWORD dwHueValue) {
	BOOL ret;
	//ret = NET_DVR_ClientSetVideoEffect(lPlayID, dwBrightValue, dwContrastValue,
	//	dwSaturationValue, dwHueValue);
	return ret;
}

// 取得图像色彩
DWORD __stdcall IHikRealPlay::GetVideoEffect(DWORD *pBrightValue,
	DWORD *pContrastValue, DWORD *pSaturationValue, DWORD *pHueValue) {
	BOOL ret;
	//ret = NET_DVR_ClientGetVideoEffect(lPlayID, pBrightValue, pContrastValue,
	//	pSaturationValue, pHueValue);
	return ret;
}

// 打开声音
DWORD __stdcall IHikRealPlay::OpenSound() {
	BOOL ret;
	//ret = NET_DVR_OpenSound(lPlayID);
	ret = CLIENT_OpenSound(lPlayID);
	return ret;
}

// 关闭声音
DWORD __stdcall IHikRealPlay::CloseSound() {
	BOOL ret;
	//ret = NET_DVR_CloseSound();
	  ret = CLIENT_CloseSound();
	return ret;
}

// 设置音量大小
DWORD __stdcall IHikRealPlay::SetVolume(WORD wVol) {
	BOOL ret;
	//ret = NET_DVR_Volume(lPlayID, wVol);
	ret = CLIENT_SetVolume(lPlayID,wVol);
	return ret;
}

// 存储文件
DWORD __stdcall IHikRealPlay::SaveRealData(char *sFileName) {
	BOOL ret;
//	ret = NET_DVR_SaveRealData(lPlayID, sFileName);
	ret = CLIENT_SaveRealData(lPlayID,sFileName);

	return ret;
}

// 停止存储文件
DWORD __stdcall IHikRealPlay::StopSaveRealData() {
	BOOL ret;
//	ret = NET_DVR_StopSaveRealData(lPlayID);
	ret = CLIENT_StopSaveRealData(lPlayID);
	return ret;
}

// 停止播放
void __stdcall IHikRealPlay::Release() {
	RealPlayNode *Node = NULL;
	//OutputDebugString(_T("\r\nDH delete-------------------1"));
	CLIENT_StopRealPlay(lPlayID);
	//OutputDebugString(_T("\r\nDH delete-------------------2"));
	CString strid;
	try{
		std::list<void*>::iterator  RealPlayListIt = RealPlayList.begin();
		while(RealPlayListIt!=RealPlayList.end())
		{
			Node = (RealPlayNode*)(*RealPlayListIt);
			if (Node!=NULL && lPlayID == Node->lPlayID) 
			{
				for (int i=0;i<MAX_NUM_DH;i++)
				{
					if (HikRealPlay[i].lPlayID == lPlayID)
					{
						HikRealPlay[i].bUsed = false;
						//test
						strid.Format(_T("\r\nRelease[%d]:%d"),i,lPlayID);
						OutputDebugString(strid);
 						delete Node;  //glfu  修改先delete后赋值为空 20130509
 						Node = NULL;
						RealPlayList.erase(RealPlayListIt);
						break;
					}
				}		
				break;
			}
			RealPlayListIt++;
		}
//		Node = NULL;
//		delete Node;  //glfu  修改先delete后赋值为空 20130509
		
		/*for (int i = 0; i < RealPlayList->Count; i++) {
		Node = (RealPlayNode*)RealPlayList->Items[i];
		if (lPlayID == Node->lPlayID) {
		RealPlayList->Delete(i);
		break;
		}
		}*/
		
// 		delete Node;  //glfu  修改先delete后赋值为空 20130509
// 		Node = NULL;
	}
	catch(...)
	{
		OutputDebugString(_T("\r\n删除某一路的流发生了异常！！"));
	}
	//OutputDebugString(_T("\r\nDH delete-------------------3"));
}

// 抓图 20121119
DWORD __stdcall IHikRealPlay::CapturePicture(char *sFileName) {
	DWORD ret;
//	ret = NET_DVR_CapturePicture(lPlayID, sFileName);
// 	CString csTemp;
// 	csTemp.Format(_T("c:\\%u"),lPlayID);
	ret = CLIENT_CapturePicture(lPlayID,sFileName);
	if (ret == FALSE)
	{
		CString csFile;
		ret = CLIENT_GetLastError();
		csFile.Format(_T("\r\n抓图路径：%S,error=%u"),sFileName,ret);
		OutputDebugString(csFile);
	}
	return ret;
}
// ----------|--------------------------------|-------------
// ----------|           IDVRUser类           |-------------
// ----------|--------------------------------|-------------

DWORD __stdcall IHikDVRUser::RealPlay(DWORD dwChannel, char cLinkType,
	char* sMulticastIP, char cFPS, HWND hWnd, IRealPlay* &pRealPlay) {
   	OutputDebugString(L"DH RealPlay in");
	LONG lPlayID;
	
	CString strid;
	
	// _RealPlay = &HikRealPlay;

	//////////////////////////////////////////////////////////////////////////fugl 2012-4-13  Begin
 	DH_RealPlayType subtype;
	 int subidx = 0;
	 if (str_SeverType == _T("4"))
	 {
		 CString streamNo;
		 GetPrivateProfileString(L"StreamNO",L"dhStreamType",L"",streamNo.GetBuffer(20),20,_T(".\\StreamNo.ini"));
		 if (streamNo == _T("0"))
			 cLinkType = 0;
		 else if (streamNo == _T("1"))
			 cLinkType = 1;
	 }

	 if(cLinkType == 0 || cLinkType == 1)
		 subidx = cLinkType;
	 else
		 subidx = 0;

	switch(subidx)
	{
	case 0:
		subtype = DH_RType_Realplay_0;
		break;
	case 1:
		subtype = DH_RType_Realplay_1;
		break;
	case 2:
		subtype = DH_RType_Realplay_2;
		break;
	case 3:
		subtype = DH_RType_Realplay_3;
		break;
	default:
		subtype = DH_RType_Realplay_0;
	}

	lPlayID = CLIENT_RealPlayEx(lUserID,dwChannel,hWnd,subtype);  //fugl 2012-4-13	
	int  num=CLIENT_GetLastError()&(0x7fffffff);
	printf("num:%d/n",num);
	//////////////////////////////////////////////////////////////////////////fugl 2012-4-13 End

	//	lPlayID = CLIENT_RealPlay(lUserID,dwChannel,hWnd); 
	if (lPlayID != 0) 
	{
		int nCount;
		for(nCount=0;nCount<MAX_NUM_DH;nCount++)
		{
			if(HikRealPlay[nCount].bUsed==false)
			{   
				HikRealPlay[nCount].bUsed = true;
				_RealPlay = &HikRealPlay[nCount];
				HikRealPlay[nCount].lPlayID = lPlayID;
				//test
				strid.Format(_T("\r\nDHRealPlay[%d]:%d"),nCount,lPlayID);
				OutputDebugString(strid);
				break;
			}
		}
		if(_RealPlay!=NULL)
		{
			RealPlayNode *Node = new RealPlayNode(lPlayID, _RealPlay);
			RealPlayList.push_front((void*)Node); // 加入列表
			//RealPlayList.push_back((void*)Node); // 加入列表  glfu 20130504 加入列表从末尾
		}

		// 		if(nCount<MAX_NUM_DH)      //glfu 注释掉该段20130504
		// 		{
		// 		  Node->pRealPlay =   &HikRealPlay[nCount];
		//         }

		CLIENT_SetRealDataCallBack(lPlayID,&fDHRealDataCallBack,0);
		pRealPlay = _RealPlay;
		_RealPlay = NULL;

	}
	else {
		pRealPlay = NULL;
		_RealPlay = NULL;
		OutputDebugString(L"DH RealPlay Fail out!!");
		return false;
		
	}

	return true;
}

// 使用透明通道传输云台控制
DWORD __stdcall IHikDVRUser::TransPTZControl(LONG lChannel, char *pPTZCodeBuf,
	DWORD dwBufSize) {
	BOOL ret;
	// ret = NET_DVR_SerialSend(lUserID,lChannel,pPTZCodeBuf,dwBufSize);
	//ret = NET_DVR_TransPTZ_Other(lUserID, lChannel, pPTZCodeBuf, dwBufSize);
	// 传输云台控制命令
	return ret;
}

// 云台控制
DWORD __stdcall IHikDVRUser::PTZControl(DWORD dwChannel, DWORD dwPTZCommand,
	DWORD dwParam, DWORD dwAction, BOOL bLock, DWORD dwPriority,DWORD step) {
	// 云台控制
	// 云台控制
	BOOL ret;	
	BOOL b_stop = TRUE;

	if (dwAction ==1 )
	{
		b_stop = FALSE;
	}
	int sendX = dwParam;
	int sendY =dwAction;
	int multple = dwPriority;
	CString tempPTZ;
	switch(dwPTZCommand) {
	case 60: // 设置预置位
		ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_PTZ_POINT_SET_CONTROL,0,dwParam,0,false);
		break;
	case 61: // 调取预置位
		ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_PTZ_POINT_MOVE_CONTROL,0,dwParam,0,false);
		break;
	case 5://水平旋转
		 if(!b_stop)
		 {
		  ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_EXTPTZ_STARTPANCRUISE,0,6,0,false);
		 }
		 else
		 {
			 ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_EXTPTZ_STOPPANCRUISE,0,6,0,false);
			 //ret = CLIENT_DHPTZControl(lUserID, dwChannel,DH_PTZ_LEFT_CONTROL,0,6,0,true);
		 }
		 break;
	case 51://线扫
		if(!b_stop)
		{
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_EXTPTZ_STARTLINESCAN,0,6,0,false);
		}
		else
		{
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_EXTPTZ_CLOSELINESCAN,0,6,0,false);
			//ret = CLIENT_DHPTZControl(lUserID, dwChannel,DH_PTZ_LEFT_CONTROL,0,6,0,true);
		}
		break;
	case 52://模式线路
		if(!b_stop)
		{
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_EXTPTZ_RUNMODE,dwParam,6,0,false);
		}
		else
		{
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_EXTPTZ_STOPMODE,dwParam,6,0,false);
			//ret = CLIENT_DHPTZControl(lUserID, dwChannel,DH_PTZ_LEFT_CONTROL,0,6,0,true);
		}
		break;
	case 53://点间轮循
		if(!b_stop)
		{
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_PTZ_POINT_LOOP_CONTROL,dwParam,6,76,false);
		}
		else
		{
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,DH_PTZ_POINT_LOOP_CONTROL,dwParam,6,96,false);
			//ret = CLIENT_DHPTZControl(lUserID, dwChannel,DH_PTZ_LEFT_CONTROL,0,6,0,true);
		}		
		break;
		//2011-08-30 左上，左下，右上，右下
		case 21:
		case 22:
		case 23:
		case 24:
			ret = CLIENT_DHPTZControlEx(lUserID, dwChannel,PTZControlChange[dwPTZCommand][0],step,step,0,b_stop);
		break;
		//2012-4-28 左边界   fugl
		case 65:
			ret = CLIENT_DHPTZControlEx(lUserID,dwChannel,DH_EXTPTZ_SETLEFTBORDER,0,0,0,FALSE);
			// 			tempPTZ.Format(_T("\r\n左边界 channel %d 设置是否成功过:%d"),dwChannel,ret);
			// 			OutputDebugString(tempPTZ);
			break;
		case 66:   //2012-4-28 右边界    fugl
			ret = CLIENT_DHPTZControlEx(lUserID,dwChannel,DH_EXTPTZ_SETRIGHTBORDER,0,0,0,FALSE);
			// 			tempPTZ.Format(_T("\r\n右边界 channel %d 设置是否成功过:%d"),dwChannel,ret);
			// 			OutputDebugString(tempPTZ);
			break;
		case 67:   //快速定位功能   //fgl 2012 12 6
			if((dwParam&0x80000000)==0x80000000)
			{
				sendY = dwParam & 0x7FFFFFFF;
				sendY = sendY | 0x80000000;
			}
			if((dwAction&0x80000000)==0x80000000)
			{
				sendX = dwAction & 0x7FFFFFFF;
				sendX = sendX | 0x80000000;
			}
			if ((dwPriority&0x80000000)==0x80000000)
			{
				multple = dwPriority & 0x7FFFFFFF;
				multple = multple | 0x80000000;
			}
			ret = CLIENT_DHPTZControlEx(lUserID,dwChannel,DH_EXTPTZ_FASTGOTO,sendX,sendY,multple,FALSE);
			tempPTZ.Format(_T("\r\n快速定位中心x坐标:%d，y坐标:%d，倍数:%d, 返回值%d"),sendX,sendY,multple,ret);
			OutputDebugString(tempPTZ);
			break;
	default:
		//ret = CLIENT_PTZControl(lUserID,dwChannel, PTZControlChange[dwPTZCommand][0], step, b_stop);
		ret = CLIENT_DHPTZControl(lUserID,(int)dwChannel, PTZControlChange[dwPTZCommand][0], 0, step,0, b_stop);
		break;
	}
	return ret;
}
void __stdcall IHikDVRUser::SetDVRDateTime(DWORD dwYear,DWORD dwMonth,DWORD dwDay,DWORD dwHour,DWORD dwMin,DWORD dwSec)
{
	NET_TIME nDVRTime;
	//Net
	nDVRTime.dwYear = dwYear;
	nDVRTime.dwMonth = dwMonth;
	nDVRTime.dwDay = dwDay;
	nDVRTime.dwHour = dwHour;
	nDVRTime.dwMinute = dwMin;
	nDVRTime.dwSecond = dwSec;
//	CLIENT_SetupDeviceTime(lUserID,&nDVRTime);  //glfu 2013/3/26修改时间同步方式
	if(!CLIENT_SetDevConfig(lUserID,DH_DEV_TIMECFG,-1,&nDVRTime,sizeof(NET_TIME)))
	{
		OutputDebugString(_T("\r\nupdate time fiale:"));			
	}
	

}
//2017-08-09 设置录像状态
DWORD __stdcall IHikDVRUser::SetRecordFile2017(int _chan,int  _type)
{
	char m_State[128];
	int  rnum;
	int  rnum1=0;
	CString strid;
	strid.Format(_T("%u"),lUserID);
	NVR_IPC *_pa=NULL;
	if (m_map_NVR_IPC.Lookup(strid,(CObject*&)_pa))
	{
		for (int i=0;i<128;i++)
		{
			if (strcmp(_pa->ip[i],"192.168.0.0") ==0)
			{
				rnum1 = i;
				break;
			}
		}
	}
	BOOL nRet = CLIENT_QueryRecordState(lUserID,m_State,128,&rnum);
	if (nRet ==1)
	{
		m_State[_chan] = _type;		
		nRet = CLIENT_SetupRecordState(lUserID,m_State,rnum1);		
	}
	//BOOL nRet = CLIENT_SetupRecordState(lUserID,_inbuf,_stateRecordnum);
	if(!nRet)
	{
		return CLIENT_GetLastError();	    
	}
	return nRet;
}
DWORD __stdcall IHikDVRUser::SetFileCover(int _chan,NET_TIME_EX *_starttime,NET_TIME_EX *_endtime,int _flag)
{
	NET_IN_SET_MARK_FILE_BY_TIME stInParam = {sizeof(stInParam)};
	NET_OUT_SET_MARK_FILE_BY_TIME stOutParam = {sizeof(stOutParam)};
	stInParam.bFlag = _flag;
	stInParam.nChannel = _chan;
	memcpy(&stInParam.stuStartTime,_starttime,sizeof(NET_TIME_EX));
	memcpy(&stInParam.stuEndTime,_endtime,sizeof(NET_TIME_EX));
	BOOL bRet = CLIENT_SetMarkFileByTime(lUserID, &stInParam, &stOutParam,5000);
	if(!bRet)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

//获取系统时间
DWORD __stdcall IHikDVRUser::QueryDeviceTime()
{
	NET_TIME nDVRTime;
	if (CLIENT_QueryDeviceTime(lUserID, &nDVRTime, 1000))
	{
		if (nDVRTime.dwYear > 1970)
		{
			CTime ts(nDVRTime.dwYear, nDVRTime.dwMonth, nDVRTime.dwDay, nDVRTime.dwHour, nDVRTime.dwMinute, nDVRTime.dwSecond);
			return ts.GetTime();
		}
		else
		{
			return 0;  
		}
	}
	else
	{
		DWORD er = CLIENT_GetLastError();
		CString strer;
		strer.Format(_T("\r\nGet LMTTime Fail er=%0x"), er);
		OutputDebugString(strer);
		return 0;
	}
}
// 取得设备配置
DWORD __stdcall IHikDVRUser::GetDVRConfig(DWORD dwCommand, LONG lChannel,
	LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned) {
		//2013-10-23 暂时不处理NVR
		if (dwTypeID == 201)
		{
			OutputDebugString(_T("\r\nNVR not handle for this time"));
			return 0; 
		}
		//---------------------------------

	// 我们的通道号是从0开始，所以+1
		DWORD ret =1;//ok
		int   ruslt =-1;
		unsigned long lpRet =0;
		char pBuf[1024];
	//	char pBuf[1326];  //glfu 2013/3/23
		stSetInfoPACKET SetInfoPACKET;
		memset(&SetInfoPACKET,0,sizeof(stSetInfoPACKET));
		memcpy(&SetInfoPACKET,lpOutBuffer,sizeof(stSetInfoPACKET));
		DWORD* p = (DWORD*)(SetInfoPACKET.Commandbuf);
		DWORD BytesReturned;
		DHDEV_CHANNEL_CFG piccfg;
		ZeroMemory(&piccfg,sizeof(DHDEV_CHANNEL_CFG));
		DHDEV_VIDEOCOVER_CFG lp_DHDEV_VIDEOCOVER_CFG;
		ZeroMemory(&lp_DHDEV_VIDEOCOVER_CFG,sizeof(DHDEV_VIDEOCOVER_CFG));
		DHDEV_RECORD_CFG    lp_DHDEV_RECORD_CFG;
		ZeroMemory(&lp_DHDEV_RECORD_CFG,sizeof(DHDEV_RECORD_CFG));
		DH_MOTION_DETECT_CFG_EX lp_DH_MOTION_DETECT_CFG_EX;
		ZeroMemory(&lp_DH_MOTION_DETECT_CFG_EX,sizeof(DH_MOTION_DETECT_CFG_EX));
		DH_ALARMIN_CFG_EX      lp_DH_ALARMIN_CFG_EX;
		ZeroMemory(&lp_DH_ALARMIN_CFG_EX,sizeof(DH_ALARMIN_CFG_EX));
		DH_VIDEO_LOST_CFG_EX   lp_DH_VIDEO_LOST_CFG_EX;
		ZeroMemory(&lp_DH_VIDEO_LOST_CFG_EX,sizeof(DH_VIDEO_LOST_CFG_EX));
		DH_BLIND_CFG_EX    lp_DH_BLIND_CFG_EX;
		ZeroMemory(&lp_DH_BLIND_CFG_EX,sizeof(DH_BLIND_CFG_EX));
	
		JF_ParamVideo m_JF_ParamVideo;
		JF_ParamCompress m_JF_ParamCompress;
		JF_ParamMotionDetect m_JF_ParamMotionDetect;
		JF_ParamALARMIN      m_JF_ParamALARMIN;
		JF_Param_LOST  m_JF_Param_LOST;
		JF_Param_BLIND m_JF_Param_BLIND;
		JF_DHDEV_RECORD_CFG    m_JF_DHDEV_RECORD_CFG;

		DHDEV_DSP_ENCODECAP stDspInfo = {0};  //glfu  20120808  增加了分辨率种类


		try
		{
			switch (p[0])
			{
			case 0://视频参数
				lpRet = sizeof(JF_ParamVideo);
				ZeroMemory(&m_JF_ParamVideo,sizeof(JF_ParamVideo));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned);
				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_VIDEO_COVER,lChannel,&lp_DHDEV_VIDEOCOVER_CFG,sizeof(DHDEV_VIDEOCOVER_CFG),&BytesReturned); 

				m_JF_ParamVideo.byBrightness = piccfg.stColorCfg[0].byBrightness;
				m_JF_ParamVideo.byContrast = piccfg.stColorCfg[0].byContrast;
				m_JF_ParamVideo.byHue = piccfg.stColorCfg[0].byHue;
				m_JF_ParamVideo.bySaturation = piccfg.stColorCfg[0].bySaturation;
				strcpy_s(m_JF_ParamVideo.ChanName,piccfg.szChannelName);
				strcpy_s(m_JF_ParamVideo.SzOSD_Name,lp_DHDEV_VIDEOCOVER_CFG.szChannelName);
				memcpy_s(&m_JF_ParamVideo.OSD_rcRect,sizeof(DH_RECT),&piccfg.stChannelOSD.rcRect,sizeof(DH_RECT));
				m_JF_ParamVideo.rgbaBackground = piccfg.stChannelOSD.rgbaBackground;
				m_JF_ParamVideo.rgbaFrontground = piccfg.stChannelOSD.rgbaFrontground;
				memcpy_s(&m_JF_ParamVideo.Time_rcRect,sizeof(DH_RECT),&piccfg.stTimeOSD.rcRect,sizeof(DH_RECT));
				m_JF_ParamVideo.maskcount = lp_DHDEV_VIDEOCOVER_CFG.bCoverCount;
				if (m_JF_ParamVideo.maskcount>4)
				{
					m_JF_ParamVideo.maskcount =4;
				}
				for (int i =0; i<m_JF_ParamVideo.maskcount;i++)
				{
					memcpy_s(&m_JF_ParamVideo.mask_rcRect[i],sizeof(DH_RECT),&lp_DHDEV_VIDEOCOVER_CFG.CoverBlock[i].rcBlock,sizeof(DH_RECT));
				}

				memcpy(pBuf,&m_JF_ParamVideo,lpRet);
				break;
			case 1://压缩参数 主流
				lpRet = sizeof(JF_ParamCompress);
				ZeroMemory(&m_JF_ParamCompress,sizeof(JF_ParamCompress));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned);
				ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_DSP,(char*)&stDspInfo,sizeof(DHDEV_DSP_ENCODECAP),(int*)&BytesReturned, 2000);
				m_JF_ParamCompress.dwImageSizeMask = stDspInfo.dwImageSizeMask;
				m_JF_ParamCompress.CIFSize = piccfg.stMainVideoEncOpt[0].byImageSize;
				m_JF_ParamCompress.FrameRate = piccfg.stMainVideoEncOpt[0].byFramesPerSec;
				m_JF_ParamCompress.RecordQualityLevel = piccfg.stMainVideoEncOpt[0].byImageQlty;
				m_JF_ParamCompress.byBitRateControl = piccfg.stMainVideoEncOpt[0].byBitRateControl;
				m_JF_ParamCompress.byAudioEnable = piccfg.stMainVideoEncOpt[0].byAudioEnable;
				m_JF_ParamCompress.byEncodeMode = piccfg.stMainVideoEncOpt[0].byEncodeMode;

				memcpy(pBuf,&m_JF_ParamCompress,lpRet);
				break;

			case 2://录象计划
				lpRet = sizeof(JF_DHDEV_RECORD_CFG);
				ZeroMemory(&m_JF_DHDEV_RECORD_CFG,sizeof(JF_DHDEV_RECORD_CFG));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_RECORDCFG,lChannel,&lp_DHDEV_RECORD_CFG,sizeof(DHDEV_RECORD_CFG),&BytesReturned);

				m_JF_DHDEV_RECORD_CFG.byPreRecordLen = lp_DHDEV_RECORD_CFG.byPreRecordLen;
				m_JF_DHDEV_RECORD_CFG.byRecordType = lp_DHDEV_RECORD_CFG.byRecordType;
				m_JF_DHDEV_RECORD_CFG.byRedundancyEn = lp_DHDEV_RECORD_CFG.byRedundancyEn;
				m_JF_DHDEV_RECORD_CFG.byReserved = lp_DHDEV_RECORD_CFG.byReserved;
				m_JF_DHDEV_RECORD_CFG.dwSize = lp_DHDEV_RECORD_CFG.dwSize;
				//				memcpy_s(m_JF_DHDEV_RECORD_CFG.stSect,sizeof(DH_TSECT)*7*5,lp_DHDEV_RECORD_CFG.stSect,sizeof(DH_TSECT)*7*5);
				for (int i=0;i<7;++i)  //2012-5-11
				{
					for (int j=0;j<6;j++)  //2013/3/23
					{
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].bEnable = lp_DHDEV_RECORD_CFG.stSect[i][j].bEnable;
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].iBeginHour = lp_DHDEV_RECORD_CFG.stSect[i][j].iBeginHour;
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].iBeginMin = lp_DHDEV_RECORD_CFG.stSect[i][j].iBeginMin;
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].iBeginSec = lp_DHDEV_RECORD_CFG.stSect[i][j].iBeginSec;
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].iEndHour = lp_DHDEV_RECORD_CFG.stSect[i][j].iEndHour;
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].iEndMin = lp_DHDEV_RECORD_CFG.stSect[i][j].iEndMin;
						m_JF_DHDEV_RECORD_CFG.stSect[i][j].iEndSec = lp_DHDEV_RECORD_CFG.stSect[i][j].iEndSec;
					}
				//	memcpy_s(m_JF_DHDEV_RECORD_CFG.stSect[i],sizeof(DH_TSECT)*5,lp_DHDEV_RECORD_CFG.stSect[i],sizeof(DH_TSECT)*5);
				}


				memcpy(pBuf,&m_JF_DHDEV_RECORD_CFG,lpRet);

				break;
			case 3://运动检测
				lpRet = sizeof(JF_ParamMotionDetect);
				ZeroMemory(&m_JF_ParamMotionDetect,sizeof(JF_ParamMotionDetect));
				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_MOTIONALARM_CFG,lChannel,&lp_DH_MOTION_DETECT_CFG_EX,sizeof(DH_MOTION_DETECT_CFG_EX),&BytesReturned);

				m_JF_ParamMotionDetect.MotionEn = lp_DH_MOTION_DETECT_CFG_EX.byMotionEn;
				m_JF_ParamMotionDetect.SenseLevel = lp_DH_MOTION_DETECT_CFG_EX.wSenseLevel;
				m_JF_ParamMotionDetect.MotionRow = lp_DH_MOTION_DETECT_CFG_EX.wMotionRow;
				m_JF_ParamMotionDetect.MotionCol = lp_DH_MOTION_DETECT_CFG_EX.wMotionCol;
				//				memcpy_s(m_JF_ParamMotionDetect.stSect,sizeof(DH_TSECT)*7*4,lp_DH_MOTION_DETECT_CFG_EX.stSect,sizeof(DH_TSECT)*7*4);
				for (int i=0;i<7;++i)  //2012-5-11
				{
					for (int j=0;j<6;j++)
					{
						m_JF_ParamMotionDetect.stSect[i][j].bEnable = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].bEnable;
						m_JF_ParamMotionDetect.stSect[i][j].iBeginHour = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iBeginHour;
						m_JF_ParamMotionDetect.stSect[i][j].iBeginMin = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iBeginMin;
						m_JF_ParamMotionDetect.stSect[i][j].iBeginSec = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iBeginSec;
						m_JF_ParamMotionDetect.stSect[i][j].iEndHour = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iEndHour;
						m_JF_ParamMotionDetect.stSect[i][j].iEndMin = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iEndMin;
						m_JF_ParamMotionDetect.stSect[i][j].iEndSec = lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iEndSec;
					}
					//memcpy_s(m_JF_ParamMotionDetect.stSect[i],sizeof(DH_TSECT)*4,lp_DH_MOTION_DETECT_CFG_EX.stSect[i],sizeof(DH_TSECT)*4);
				}
				m_JF_ParamMotionDetect.dwRecLatch = lp_DH_MOTION_DETECT_CFG_EX.struHandle.dwRecLatch;
				//				memcpy_s(m_JF_ParamMotionDetect.bySnap,sizeof(BYTE)*24,lp_DH_MOTION_DETECT_CFG_EX.struHandle.bySnap,sizeof(BYTE)*24);



				for (int i=0;i<32;i++)
				{
					for (int j=0;j<32;j++)  //glfu 2013/3/23
					{
						m_JF_ParamMotionDetect.byDetected[i] |= (lp_DH_MOTION_DETECT_CFG_EX.byDetected[i][j]<<(31-j));
					}
				}
				memcpy(pBuf,&m_JF_ParamMotionDetect,lpRet);
				break;
			case 4://报警设置
				lpRet = sizeof(JF_ParamALARMIN);
				ZeroMemory(&m_JF_ParamALARMIN,sizeof(JF_ParamALARMIN));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_LOCALALARM_CFG,lChannel,&lp_DH_ALARMIN_CFG_EX,sizeof(DH_ALARMIN_CFG_EX),&BytesReturned);
				m_JF_ParamALARMIN.byAlarmType = lp_DH_ALARMIN_CFG_EX.byAlarmType;
				m_JF_ParamALARMIN.AlarmEn = lp_DH_ALARMIN_CFG_EX.byAlarmEn;
				//				memcpy_s(m_JF_ParamALARMIN.stSect,sizeof(DH_TSECT)*7*4,lp_DH_ALARMIN_CFG_EX.stSect,sizeof(DH_TSECT)*7*4);
				for (int i=0;i<7;++i)   //2012-5-11
				{
					for (int j = 0;j<6;j++)
					{
						m_JF_ParamALARMIN.stSect[i][j].bEnable = lp_DH_ALARMIN_CFG_EX.stSect[i][j].bEnable;
						m_JF_ParamALARMIN.stSect[i][j].iBeginHour = lp_DH_ALARMIN_CFG_EX.stSect[i][j].iBeginHour;
						m_JF_ParamALARMIN.stSect[i][j].iBeginMin = lp_DH_ALARMIN_CFG_EX.stSect[i][j].iBeginMin;
						m_JF_ParamALARMIN.stSect[i][j].iBeginSec = lp_DH_ALARMIN_CFG_EX.stSect[i][j].iBeginSec;
						m_JF_ParamALARMIN.stSect[i][j].iEndHour = lp_DH_ALARMIN_CFG_EX.stSect[i][j].iEndHour;
						m_JF_ParamALARMIN.stSect[i][j].iEndMin = lp_DH_ALARMIN_CFG_EX.stSect[i][j].iEndMin;
						m_JF_ParamALARMIN.stSect[i][j].iEndSec = lp_DH_ALARMIN_CFG_EX.stSect[i][j].iEndSec;
					}
					//memcpy_s(m_JF_ParamALARMIN.stSect[i],sizeof(DH_TSECT)*4,lp_DH_ALARMIN_CFG_EX.stSect[i],sizeof(DH_TSECT)*4);
				}
				m_JF_ParamALARMIN.ActionMask = lp_DH_ALARMIN_CFG_EX.struHandle.dwActionMask;
				memcpy_s(m_JF_ParamALARMIN.byRelAlarmOut,sizeof(BYTE)*32,lp_DH_ALARMIN_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*32);
				memcpy_s(m_JF_ParamALARMIN.byRecordChannel,sizeof(BYTE)*32,lp_DH_ALARMIN_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*32);
				m_JF_ParamALARMIN.dwRecLatch = lp_DH_ALARMIN_CFG_EX.struHandle.dwRecLatch;
				memcpy_s(m_JF_ParamALARMIN.bySnap,sizeof(BYTE)*32,lp_DH_ALARMIN_CFG_EX.struHandle.bySnap,sizeof(BYTE)*32);

				memcpy(pBuf,&m_JF_ParamALARMIN,lpRet);
				break;
			case 5://远程压缩参数设置
				lpRet = sizeof(JF_ParamCompress);
				ZeroMemory(&m_JF_ParamCompress,sizeof(JF_ParamCompress));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned);

				m_JF_ParamCompress.CIFSize = piccfg.stAssiVideoEncOpt[0].byImageSize;
				m_JF_ParamCompress.FrameRate = piccfg.stAssiVideoEncOpt[0].byFramesPerSec;
				m_JF_ParamCompress.RecordQualityLevel = piccfg.stAssiVideoEncOpt[0].byImageQlty;
				m_JF_ParamCompress.byBitRateControl = piccfg.stAssiVideoEncOpt[0].byBitRateControl;
				m_JF_ParamCompress.byAudioEnable = piccfg.stAssiVideoEncOpt[0].byAudioEnable;
				m_JF_ParamCompress.byEncodeMode = piccfg.stAssiVideoEncOpt[0].byEncodeMode;

				memcpy(pBuf,&m_JF_ParamCompress,lpRet);
				break;
			case 6://视频丢失-6
				lpRet = sizeof(JF_Param_LOST);
				ZeroMemory(&m_JF_Param_LOST,sizeof(JF_Param_LOST));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,&lp_DH_VIDEO_LOST_CFG_EX,sizeof(DH_VIDEO_LOST_CFG_EX),&BytesReturned);


				m_JF_Param_LOST.Lost_AlarmEn = lp_DH_VIDEO_LOST_CFG_EX.byAlarmEn;
				memcpy_s(m_JF_Param_LOST.Lost_stSect,sizeof(DH_TSECT)*7*4,lp_DH_VIDEO_LOST_CFG_EX.stSect,sizeof(DH_TSECT)*7*4);
				m_JF_Param_LOST.Lost_ActionMask = lp_DH_VIDEO_LOST_CFG_EX.struHandle.dwActionMask;
				memcpy_s(m_JF_Param_LOST.Lost_byRelAlarmOut,sizeof(BYTE)*32,lp_DH_VIDEO_LOST_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*32);
				memcpy_s(m_JF_Param_LOST.Lost_byRecordChannel,sizeof(BYTE)*32,lp_DH_VIDEO_LOST_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*32);
				m_JF_Param_LOST.Lost_dwRecLatch = lp_DH_VIDEO_LOST_CFG_EX.struHandle.dwRecLatch;
				memcpy_s(m_JF_Param_LOST.Lost_bySnap,sizeof(BYTE)*32,lp_DH_VIDEO_LOST_CFG_EX.struHandle.bySnap,sizeof(BYTE)*32);



				memcpy(pBuf,&m_JF_Param_LOST,lpRet);
				break;
			case 7://遮挡报警7 
				lpRet = sizeof(JF_Param_BLIND);
				ZeroMemory(&m_JF_Param_BLIND,sizeof(JF_Param_BLIND));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_BLINDALARM_CFG,lChannel,&lp_DH_BLIND_CFG_EX,sizeof(DH_BLIND_CFG_EX),&BytesReturned);

				m_JF_Param_BLIND.byBlindEnable = lp_DH_BLIND_CFG_EX.byBlindEnable;
				m_JF_Param_BLIND.byBlindLevel = lp_DH_BLIND_CFG_EX.byBlindLevel;
				memcpy_s(m_JF_Param_BLIND.Blind_stSect,sizeof(DH_TSECT)*7*4,lp_DH_BLIND_CFG_EX.stSect,sizeof(DH_TSECT)*7*4);
				m_JF_Param_BLIND.Blind_ActionMask = lp_DH_BLIND_CFG_EX.struHandle.dwActionMask;
				memcpy_s(m_JF_Param_BLIND.Blind_byRelAlarmOut,sizeof(BYTE)*32,lp_DH_BLIND_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*32);
				memcpy_s(m_JF_Param_BLIND.Blind_byRecordChannel,sizeof(BYTE)*32,lp_DH_BLIND_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*32);
				m_JF_Param_BLIND.Blind_dwRecLatch = lp_DH_BLIND_CFG_EX.struHandle.dwRecLatch;
				memcpy_s(m_JF_Param_BLIND.Blind_bySnap,sizeof(BYTE)*32,lp_DH_BLIND_CFG_EX.struHandle.bySnap,sizeof(BYTE)*32);

				memcpy(pBuf,&m_JF_Param_BLIND,lpRet);
				break;
			default:
				return 0;
				break;
			}

			memcpy((void*)(&p[2]),pBuf,lpRet);
			SetInfoPACKET.CommandHead.CmdLen = 5+lpRet/4;
			SetInfoPACKET.CommandID = 0x74000004;
			p[1]= lpRet;
			*lpBytesReturned = sizeof(stSetInfoPACKET);
			memcpy(lpOutBuffer,(void*)&SetInfoPACKET,sizeof(stSetInfoPACKET)) ;
		}
		catch (...)
		{
			OutputDebugString(_T("\r\nGetDVRConfig error"));
			return 0;
		}		

		return ret;
}

// 设置设备配置
DWORD __stdcall IHikDVRUser::SetDVRConfig(DWORD dwCommand, LONG lChannel,
	LPVOID lpInBuffer, DWORD dwInBufferSize) {

		//2013-10-23 暂时不处理NVR
		if (dwTypeID == 201)
		{
			OutputDebugString(_T("\r\nNVR not handle for this time"));
			return 0; 
		}
		//---------------------------------

		int ret =1;
		int   ruslt =-1;
		stSetInfoPACKET SetInfoPACKET;
		memcpy(&SetInfoPACKET,lpInBuffer,sizeof(stSetInfoPACKET));
		DWORD* p = (DWORD*)(SetInfoPACKET.Commandbuf);

		JF_ParamVideo m_JF_ParamVideo;
		JF_ParamCompress m_JF_ParamCompress;
		JF_ParamMotionDetect m_JF_ParamMotionDetect;
		JF_ParamALARMIN      m_JF_ParamALARMIN;
		JF_Param_LOST  m_JF_Param_LOST;
		JF_Param_BLIND m_JF_Param_BLIND;
		JF_DHDEV_RECORD_CFG    m_JF_DHDEV_RECORD_CFG;

		DWORD BytesReturned;
		DHDEV_CHANNEL_CFG piccfg;
		ZeroMemory(&piccfg,sizeof(DHDEV_CHANNEL_CFG));
		DHDEV_RECORD_CFG lp_DHDEV_RECORD_CFG;
		ZeroMemory(&lp_DHDEV_RECORD_CFG,sizeof(DHDEV_RECORD_CFG));
		DHDEV_VIDEOCOVER_CFG lp_DHDEV_VIDEOCOVER_CFG;
		ZeroMemory(&lp_DHDEV_VIDEOCOVER_CFG,sizeof(DHDEV_VIDEOCOVER_CFG));
		DH_MOTION_DETECT_CFG_EX lp_DH_MOTION_DETECT_CFG_EX;
		ZeroMemory(&lp_DH_MOTION_DETECT_CFG_EX,sizeof(DH_MOTION_DETECT_CFG_EX));
		DH_ALARMIN_CFG_EX      lp_DH_ALARMIN_CFG_EX;
		ZeroMemory(&lp_DH_ALARMIN_CFG_EX,sizeof(DH_ALARMIN_CFG_EX));
		DH_VIDEO_LOST_CFG_EX   lp_DH_VIDEO_LOST_CFG_EX;
		ZeroMemory(&lp_DH_VIDEO_LOST_CFG_EX,sizeof(DH_VIDEO_LOST_CFG_EX));
		DH_BLIND_CFG_EX    lp_DH_BLIND_CFG_EX;
		ZeroMemory(&lp_DH_BLIND_CFG_EX,sizeof(DH_BLIND_CFG_EX));

		try
		{
			switch(p[0])
			{
			case 0://视频参数
				ZeroMemory(&m_JF_ParamVideo,sizeof(JF_ParamVideo));
				memcpy(&m_JF_ParamVideo,SetInfoPACKET.Commandbuf+8,sizeof(JF_ParamVideo));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned);
				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_VIDEO_COVER,lChannel,&lp_DHDEV_VIDEOCOVER_CFG,sizeof(DHDEV_VIDEOCOVER_CFG),&BytesReturned); 

				piccfg.stColorCfg[0].byBrightness = m_JF_ParamVideo.byBrightness;
				piccfg.stColorCfg[0].byContrast = m_JF_ParamVideo.byContrast;
				piccfg.stColorCfg[0].byHue = m_JF_ParamVideo.byHue;
				piccfg.stColorCfg[0].bySaturation = m_JF_ParamVideo.bySaturation;
				strcpy_s(piccfg.szChannelName,m_JF_ParamVideo.ChanName);
				strcpy_s(lp_DHDEV_VIDEOCOVER_CFG.szChannelName,m_JF_ParamVideo.SzOSD_Name);
				memcpy_s(&piccfg.stChannelOSD.rcRect,sizeof(DH_RECT),&m_JF_ParamVideo.OSD_rcRect,sizeof(DH_RECT));
				piccfg.stChannelOSD.rgbaBackground = m_JF_ParamVideo.rgbaBackground;
				piccfg.stChannelOSD.rgbaFrontground = m_JF_ParamVideo.rgbaFrontground;
				memcpy_s(&piccfg.stTimeOSD.rcRect,sizeof(DH_RECT),&m_JF_ParamVideo.Time_rcRect,sizeof(DH_RECT));
				piccfg.stChannelOSD.bShow = 1;  //glfu 20120808
				for (int i =0; i<m_JF_ParamVideo.maskcount;i++)
				{
					memcpy_s(&lp_DHDEV_VIDEOCOVER_CFG.CoverBlock[i].rcBlock,sizeof(DH_RECT),&m_JF_ParamVideo.mask_rcRect[i],sizeof(DH_RECT));
				}

				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG));
				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_VIDEO_COVER,lChannel,&lp_DHDEV_VIDEOCOVER_CFG,sizeof(DHDEV_VIDEOCOVER_CFG));

				break;
			case 1://压缩参数 主流
				ZeroMemory(&m_JF_ParamCompress,sizeof(JF_ParamCompress));
				memcpy(&m_JF_ParamCompress,SetInfoPACKET.Commandbuf+8,sizeof(JF_ParamCompress));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned);

				piccfg.stMainVideoEncOpt[0].byImageSize = m_JF_ParamCompress.CIFSize;
				piccfg.stMainVideoEncOpt[0].byFramesPerSec = m_JF_ParamCompress.FrameRate;
				piccfg.stMainVideoEncOpt[0].byImageQlty = m_JF_ParamCompress.RecordQualityLevel;
				piccfg.stMainVideoEncOpt[0].byBitRateControl = m_JF_ParamCompress.byBitRateControl;
				piccfg.stMainVideoEncOpt[0].byAudioEnable = m_JF_ParamCompress.byAudioEnable;
				piccfg.stMainVideoEncOpt[0].byEncodeMode = m_JF_ParamCompress.byEncodeMode;

				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG));
				break;
			case 2://录象计划
				ZeroMemory(&m_JF_DHDEV_RECORD_CFG,sizeof(JF_DHDEV_RECORD_CFG));
				memcpy(&m_JF_DHDEV_RECORD_CFG,SetInfoPACKET.Commandbuf+8,sizeof(JF_DHDEV_RECORD_CFG));

				lp_DHDEV_RECORD_CFG.byPreRecordLen = m_JF_DHDEV_RECORD_CFG.byPreRecordLen;
				lp_DHDEV_RECORD_CFG.byRecordType = m_JF_DHDEV_RECORD_CFG.byRecordType;
				lp_DHDEV_RECORD_CFG.byRedundancyEn = m_JF_DHDEV_RECORD_CFG.byRedundancyEn;
				lp_DHDEV_RECORD_CFG.byReserved = m_JF_DHDEV_RECORD_CFG.byReserved;
				lp_DHDEV_RECORD_CFG.dwSize = sizeof(DHDEV_RECORD_CFG);//sizeof(JF_DHDEV_RECORD_CFG);
// 				for (int i=0;i<7;i++)
// 				{
// 					memset(&lp_DHDEV_RECORD_CFG.stSect[i][6],0,sizeof(DH_TSECT));
// 				}
				
//				memcpy_s(lp_DHDEV_RECORD_CFG.stSect,sizeof(DH_TSECT)*7*5,m_JF_DHDEV_RECORD_CFG.stSect,sizeof(DH_TSECT)*7*5);
				for (int i=0;i<7;++i)   //2012-5-11
				{
					for (int j=0;j<6;j++)
					{
						lp_DHDEV_RECORD_CFG.stSect[i][j].bEnable = m_JF_DHDEV_RECORD_CFG.stSect[i][j].bEnable;
						lp_DHDEV_RECORD_CFG.stSect[i][j].iBeginHour = m_JF_DHDEV_RECORD_CFG.stSect[i][j].iBeginHour;
						lp_DHDEV_RECORD_CFG.stSect[i][j].iBeginMin = m_JF_DHDEV_RECORD_CFG.stSect[i][j].iBeginMin;
						lp_DHDEV_RECORD_CFG.stSect[i][j].iBeginSec = m_JF_DHDEV_RECORD_CFG.stSect[i][j].iBeginSec;
						lp_DHDEV_RECORD_CFG.stSect[i][j].iEndHour = m_JF_DHDEV_RECORD_CFG.stSect[i][j].iEndHour;
						lp_DHDEV_RECORD_CFG.stSect[i][j].iEndMin = m_JF_DHDEV_RECORD_CFG.stSect[i][j].iEndMin;
						lp_DHDEV_RECORD_CFG.stSect[i][j].iEndSec = m_JF_DHDEV_RECORD_CFG.stSect[i][j].iEndSec;
					}
					//memcpy_s(lp_DHDEV_RECORD_CFG.stSect[i],sizeof(DH_TSECT)*5,m_JF_DHDEV_RECORD_CFG.stSect[i],sizeof(DH_TSECT)*5);
				}
				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_RECORDCFG,lChannel,&lp_DHDEV_RECORD_CFG,sizeof(DHDEV_RECORD_CFG));
				break;
			case 3://运动检测
				ZeroMemory(&m_JF_ParamMotionDetect,sizeof(JF_ParamMotionDetect));
				memcpy(&m_JF_ParamMotionDetect,SetInfoPACKET.Commandbuf+8,sizeof(JF_ParamMotionDetect));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_MOTIONALARM_CFG,lChannel,&lp_DH_MOTION_DETECT_CFG_EX,sizeof(DH_MOTION_DETECT_CFG_EX),&BytesReturned);

				lp_DH_MOTION_DETECT_CFG_EX.byMotionEn = m_JF_ParamMotionDetect.MotionEn;
				lp_DH_MOTION_DETECT_CFG_EX.wSenseLevel = m_JF_ParamMotionDetect.SenseLevel;
				lp_DH_MOTION_DETECT_CFG_EX.wMotionRow = m_JF_ParamMotionDetect.MotionRow;
				lp_DH_MOTION_DETECT_CFG_EX.wMotionCol = m_JF_ParamMotionDetect.MotionCol;
//				memcpy_s(lp_DH_MOTION_DETECT_CFG_EX.stSect,sizeof(DH_TSECT)*7*4,m_JF_ParamMotionDetect.stSect,sizeof(DH_TSECT)*7*4);
				for (int i=0;i<7;++i)   //2012-5-11
				{
					for (int j=0;j<6;j++)
					{
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].bEnable = m_JF_ParamMotionDetect.stSect[i][j].bEnable;
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iBeginHour = m_JF_ParamMotionDetect.stSect[i][j].iBeginHour;
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iBeginMin = m_JF_ParamMotionDetect.stSect[i][j].iBeginMin;
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iBeginSec = m_JF_ParamMotionDetect.stSect[i][j].iBeginSec;
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iEndHour = m_JF_ParamMotionDetect.stSect[i][j].iEndHour;
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iEndMin = m_JF_ParamMotionDetect.stSect[i][j].iEndMin;
						lp_DH_MOTION_DETECT_CFG_EX.stSect[i][j].iEndSec = m_JF_ParamMotionDetect.stSect[i][j].iEndSec;
					}
					//memcpy_s(lp_DH_MOTION_DETECT_CFG_EX.stSect[i],sizeof(DH_TSECT)*4,m_JF_ParamMotionDetect.stSect[i],sizeof(DH_TSECT)*4);
				}
//				lp_DH_MOTION_DETECT_CFG_EX.struHandle.dwActionMask = m_JF_ParamMotionDetect.ActionMask;
//				memcpy_s(lp_DH_MOTION_DETECT_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*24,m_JF_ParamMotionDetect.byRelAlarmOut,sizeof(BYTE)*24);
//				memcpy_s(lp_DH_MOTION_DETECT_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*24,m_JF_ParamMotionDetect.byRecordChannel,sizeof(BYTE)*24);
				lp_DH_MOTION_DETECT_CFG_EX.struHandle.dwRecLatch = m_JF_ParamMotionDetect.dwRecLatch;
//				memcpy_s(lp_DH_MOTION_DETECT_CFG_EX.struHandle.bySnap,sizeof(BYTE)*24,m_JF_ParamMotionDetect.bySnap,sizeof(BYTE)*24);
				/*
				for (int i=0; i<32; i++)
				{
					ZeroMemory(c_byte,32);
					_ultoa(m_JF_ParamMotionDetect.byDetected[i],c_byte,2);
					for(int j=0; j<32; j++)
					{
						lp_DH_MOTION_DETECT_CFG_EX.byDetected[i][j] = c_byte[j];
					}				
				}
				*/
				
				for (int i=0;i<32;i++)
				{
					for (int j=0;j<32;j++)
					{
						lp_DH_MOTION_DETECT_CFG_EX.byDetected[i][j] = ((m_JF_ParamMotionDetect.byDetected[i]>>(31-j))&0x01);
						
					}
				}
				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_MOTIONALARM_CFG,lChannel,&lp_DH_MOTION_DETECT_CFG_EX,sizeof(DH_MOTION_DETECT_CFG_EX));
				break;
			case 4://报警设置
				ZeroMemory(&m_JF_ParamALARMIN,sizeof(JF_ParamALARMIN));
				memcpy(&m_JF_ParamALARMIN,SetInfoPACKET.Commandbuf+8,sizeof(JF_ParamALARMIN));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_LOCALALARM_CFG,lChannel,&lp_DH_ALARMIN_CFG_EX,sizeof(DH_ALARMIN_CFG_EX),&BytesReturned);

				lp_DH_ALARMIN_CFG_EX.byAlarmType = m_JF_ParamALARMIN.byAlarmType;
				lp_DH_ALARMIN_CFG_EX.byAlarmEn = m_JF_ParamALARMIN.AlarmEn;
//				memcpy_s(lp_DH_ALARMIN_CFG_EX.stSect,sizeof(DH_TSECT)*7*4,m_JF_ParamALARMIN.stSect,sizeof(DH_TSECT)*7*4);
				for (int i=0;i<7;++i) //2012-5-11
				{
					for (int j=0;j<6;j++)
					{
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].bEnable = m_JF_ParamALARMIN.stSect[i][j].bEnable;
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].iBeginHour = m_JF_ParamALARMIN.stSect[i][j].iBeginHour;
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].iBeginMin = m_JF_ParamALARMIN.stSect[i][j].iBeginMin;
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].iBeginSec = m_JF_ParamALARMIN.stSect[i][j].iBeginSec;
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].iEndHour = m_JF_ParamALARMIN.stSect[i][j].iEndHour;
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].iEndMin = m_JF_ParamALARMIN.stSect[i][j].iEndMin;
						lp_DH_ALARMIN_CFG_EX.stSect[i][j].iEndSec = m_JF_ParamALARMIN.stSect[i][j].iEndSec;
					}
					//memcpy_s(lp_DH_ALARMIN_CFG_EX.stSect[i],sizeof(DH_TSECT)*4,m_JF_ParamALARMIN.stSect[i],sizeof(DH_TSECT)*4);
				}
				lp_DH_ALARMIN_CFG_EX.struHandle.dwActionMask = m_JF_ParamALARMIN.ActionMask;
				memcpy_s(lp_DH_ALARMIN_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*32,m_JF_ParamALARMIN.byRelAlarmOut,sizeof(BYTE)*32);
				memcpy_s(lp_DH_ALARMIN_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*32,m_JF_ParamALARMIN.byRecordChannel,sizeof(BYTE)*32);
				lp_DH_ALARMIN_CFG_EX.struHandle.dwRecLatch = m_JF_ParamALARMIN.dwRecLatch;
				memcpy_s(lp_DH_ALARMIN_CFG_EX.struHandle.bySnap ,sizeof(BYTE)*32,m_JF_ParamALARMIN.bySnap,sizeof(BYTE)*32);

				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_LOCALALARM_CFG,lChannel,&lp_DH_ALARMIN_CFG_EX,sizeof(DH_ALARMIN_CFG_EX));
				break;
			case 5://远程压缩参数设置
				ZeroMemory(&m_JF_ParamCompress,sizeof(JF_ParamCompress));
				memcpy(&m_JF_ParamCompress,SetInfoPACKET.Commandbuf+8,sizeof(JF_ParamCompress));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG),&BytesReturned);

				piccfg.stAssiVideoEncOpt[0].byImageSize = m_JF_ParamCompress.CIFSize;
				piccfg.stAssiVideoEncOpt[0].byFramesPerSec = m_JF_ParamCompress.FrameRate;
				piccfg.stAssiVideoEncOpt[0].byImageQlty = m_JF_ParamCompress.RecordQualityLevel;
				piccfg.stAssiVideoEncOpt[0].byBitRateControl = m_JF_ParamCompress.byBitRateControl;
				piccfg.stAssiVideoEncOpt[0].byAudioEnable = m_JF_ParamCompress.byAudioEnable;
				piccfg.stAssiVideoEncOpt[0].byEncodeMode = m_JF_ParamCompress.byEncodeMode;

				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_CHANNELCFG,lChannel,&piccfg,sizeof(DHDEV_CHANNEL_CFG));
				break;
			case 6://视频丢失-6
				ZeroMemory(&m_JF_Param_LOST,sizeof(JF_Param_LOST));
				memcpy(&m_JF_Param_LOST,SetInfoPACKET.Commandbuf+8,sizeof(JF_Param_LOST));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,&lp_DH_VIDEO_LOST_CFG_EX,sizeof(DH_VIDEO_LOST_CFG_EX),&BytesReturned);

				lp_DH_VIDEO_LOST_CFG_EX.byAlarmEn = m_JF_Param_LOST.Lost_AlarmEn;
				memcpy_s(lp_DH_VIDEO_LOST_CFG_EX.stSect,sizeof(DH_TSECT)*7*4, m_JF_Param_LOST.Lost_stSect,sizeof(DH_TSECT)*7*4);
				lp_DH_VIDEO_LOST_CFG_EX.struHandle.dwActionMask = m_JF_Param_LOST.Lost_ActionMask;
				memcpy_s(lp_DH_VIDEO_LOST_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*32,m_JF_Param_LOST.Lost_byRelAlarmOut,sizeof(BYTE)*32);
				memcpy_s(lp_DH_VIDEO_LOST_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*32,m_JF_Param_LOST.Lost_byRecordChannel,sizeof(BYTE)*32);
				lp_DH_VIDEO_LOST_CFG_EX.struHandle.dwRecLatch = m_JF_Param_LOST.Lost_dwRecLatch;
				memcpy_s(lp_DH_VIDEO_LOST_CFG_EX.struHandle.bySnap,sizeof(BYTE)*32,m_JF_Param_LOST.Lost_bySnap,sizeof(BYTE)*32);


				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_VIDEOLOSTALARM_CFG,lChannel,&lp_DH_VIDEO_LOST_CFG_EX,sizeof(DH_VIDEO_LOST_CFG_EX));
				break;
			case 7://遮挡报警-7
				ZeroMemory(&m_JF_Param_BLIND,sizeof(JF_Param_BLIND));
				memcpy(&m_JF_Param_BLIND,SetInfoPACKET.Commandbuf+8,sizeof(JF_Param_BLIND));

				ret = CLIENT_GetDevConfig(lUserID,DH_DEV_BLINDALARM_CFG,lChannel,&lp_DH_BLIND_CFG_EX,sizeof(DH_BLIND_CFG_EX),&BytesReturned);

				lp_DH_BLIND_CFG_EX.byBlindEnable = m_JF_Param_BLIND.byBlindEnable;
				lp_DH_BLIND_CFG_EX.byBlindLevel = m_JF_Param_BLIND.byBlindLevel;
				memcpy_s(lp_DH_BLIND_CFG_EX.stSect,sizeof(DH_TSECT)*7*4,m_JF_Param_BLIND.Blind_stSect,sizeof(DH_TSECT)*7*4);
				lp_DH_BLIND_CFG_EX.struHandle.dwActionMask = m_JF_Param_BLIND.Blind_ActionMask;
				memcpy_s(lp_DH_BLIND_CFG_EX.struHandle.byRelAlarmOut,sizeof(BYTE)*32,m_JF_Param_BLIND.Blind_byRelAlarmOut,sizeof(BYTE)*32);
				memcpy_s(lp_DH_BLIND_CFG_EX.struHandle.byRecordChannel,sizeof(BYTE)*32,m_JF_Param_BLIND.Blind_byRecordChannel,sizeof(BYTE)*32);
				lp_DH_BLIND_CFG_EX.struHandle.dwRecLatch = m_JF_Param_BLIND.Blind_dwRecLatch;
				memcpy_s(lp_DH_BLIND_CFG_EX.struHandle.bySnap,sizeof(BYTE)*32,m_JF_Param_BLIND.Blind_bySnap,sizeof(BYTE)*32);

				ret = CLIENT_SetDevConfig(lUserID,DH_DEV_BLINDALARM_CFG,lChannel,&lp_DH_BLIND_CFG_EX,sizeof(DH_BLIND_CFG_EX));
				break;
			default://other
				return 0;
				break;
			}	
			SetInfoPACKET.CommandHead.CmdLen = 5;
			SetInfoPACKET.CommandID = 0x74000006;
			p[1]= ret;
			memcpy(lpInBuffer,(void*)&SetInfoPACKET,sizeof(stSetInfoPACKET)) ;
		}
		catch (...)
		{
			OutputDebugString(_T("\r\nSetDVRConfig error"));
			return 0;
		}
		
		return ret;
}

// 开始语音对讲
DWORD __stdcall IHikDVRUser::StartVoiceCom(char cCompress,
	IVoiceCom* &pVoiceCom) {
	LONG _lUserID;
	IVoiceCom* _pVoiceCom;
	IHikIVoiceCom _pHikVoiceCom;
	_pVoiceCom = &_pHikVoiceCom;
//	_lUserID = NET_DVR_StartVoiceCom(lUserID, lpVoiceDataCallBack, 0);
	_lUserID = CLIENT_StartTalkEx(lUserID,lpVoiceDataCallBack,0);
	if(lUserID == 0)
	{
		CString cstemp;
		cstemp.Format(_T("\r\nDH-开启语音对讲失败，错误码:%d"),CLIENT_GetLastError());
		OutputDebugString(cstemp);
	}
	else
	{
		_pHikVoiceCom.lVoiceID = _lUserID;
		pVoiceCom = _pVoiceCom;
	}
// 	_pHikVoiceCom.lVoiceID = _lUserID;
// 	 pVoiceCom = _pVoiceCom;
	_pVoiceCom = NULL;
//	delete _pVoiceCom;

	return true;
}

// 查找文件
DWORD __stdcall IHikDVRUser::FindFile(LONG lChannel, DWORD dwFileType,
	DWORD dwStartTime, DWORD dwStopTime, DWORD UserData) {

	try
	// 开线程查找文件列表
	{
		TFindFileThread *FindFileThread = new TFindFileThread();
		FindFileThread->Init(lUserID,
			lChannel, dwFileType, dwStartTime, dwStopTime, this, UserData);

		return true;
	}
	catch(...) {
		return -1;
	}

}

// 结束查找文件
DWORD __stdcall IHikDVRUser::FindFileClose(LONG lFFHandle) {
	return 0;
}
//Data callback 2011-10-11
int CALLBACK DataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer, DWORD dwBufSize, DWORD dwUser)
{
	IHikPlayBack * pa=NULL;
	CString	strtmp;
	if(m_map_playback.Lookup((WORD)lRealHandle,(CObject*&)pa))
	{
		if (pa->lpHsitroyDATACALLBACK!=NULL)
		{
			WaitForSingleObject(pa->controlplay_handle, INFINITE);//08-4-8
			ResetEvent(pa->controlplay_handle);
			if (pa->lpHsitroyDATACALLBACK!=NULL)
			{
				pa->lpHsitroyDATACALLBACK(pa,(LPVOID*)pBuffer,dwBufSize);
			}
			
		}

		strtmp.Format(_T("\r\nhsitroydata:%d"),dwBufSize);
//		OutputDebugString(strtmp);
	}
	return 1;
}
//2011-11-14
void __stdcall IHikPlayBack::controlplay()
{
	IHikPlayBack * pa=NULL;
	if(m_map_playback.Lookup(lPlayBackID,(CObject*&)pa))
	{
		SetEvent(pa->controlplay_handle);
	}
}
//2011-09-08
void __stdcall IHikPlayBack::Release()
{
	//	int iRet = CLIENT_StopPlayBack(lPlayBackID);
	IHikPlayBack * pa=NULL;
	if(m_map_playback.Lookup(lPlayBackID,(CObject*&)pa))
	{
		//2012-05-30  fugl
		pa->lpHsitroyDATACALLBACK = NULL;
		SetEvent(pa->controlplay_handle);
		int iRet = CLIENT_StopPlayBack(lPlayBackID);
		delete pa;
		pa = NULL;
		m_map_playback.RemoveKey(lPlayBackID);
	}
}
//////////////////////////////////////////////////////////////////////////
//2012-10-25 加入回放控制功能，当hWnd有效时
BOOL __stdcall IHikPlayBack::SeekPlayBack(unsigned int unSetPos,DWORD dwTotalSize) //unSetPos[0,100]  
{
//	UINT nOffsetTime = unSetPos*dwTotalSize/100;
//	unsigned int nOffsetByte = unSetPos*dwTotalSize/100;
	BOOL bRet = CLIENT_SeekPlayBack(lPlayBackID,0xFFFFFFFF,unSetPos);
	if (bRet == FALSE)
	{
		CString tmp;
		tmp.Format(_T("\r\n 拖动失败，失败的错误码%lu"),CLIENT_GetLastError());
		OutputDebugString(tmp);
	}
	return bRet;
}
BOOL __stdcall IHikPlayBack::FastPlayBack()
{
	BOOL bRet = CLIENT_FastPlayBack(lPlayBackID);
	return bRet;
}
BOOL __stdcall IHikPlayBack::SlowPlayBack()
{
	BOOL bRet = CLIENT_SlowPlayBack(lPlayBackID);
	return bRet;
}
BOOL __stdcall IHikPlayBack::PausePlayBack(BOOL bPause)
{
	BOOL bRet = CLIENT_PausePlayBack(lPlayBackID,bPause);
	return bRet;
}
BOOL __stdcall IHikPlayBack::NormalPlayBack()
{
	BOOL bRet = CLIENT_NormalPlayBack(lPlayBackID);
	return bRet;
}
//2013 1 5 加入智能回放功能
BOOL __stdcall IHikPlayBack::SmartSearchPlayBack(LPIntelligentSearchPlay lpPlayBackParam)
{
	BOOL bRet = FALSE;
	bRet = CLIENT_SmartSearchPlayBack( lPlayBackID,lpPlayBackParam);
	if (bRet == FALSE)
	{
		CString tmp;
		tmp.Format(_T("\r\n智能检索失败，失败的错误码：%d"),CLIENT_GetLastError());
		OutputDebugString(tmp);
	}
	return bRet;
}
BOOL __stdcall IHikPlayBack::GetPlayBackTime(LPNET_TIME lpOsdTime,LPNET_TIME lpStartTime,LPNET_TIME lpEndTime)
{
	BOOL bRet = FALSE;
	bRet = CLIENT_GetPlayBackOsdTime(lPlayBackID, lpOsdTime, lpStartTime, lpEndTime);
	if (bRet == FALSE)
	{
		CString tmp;
		tmp.Format(_T("\r\n获取回放时间失败，失败的错误码：%d"),CLIENT_GetLastError());
		OutputDebugString(tmp);
	}
	return bRet;
}
//////////////////////////////////////////////////////////////////////////
// 回放 2011-10-10
DWORD __stdcall IHikDVRUser::PlayBack(char *sPlayBackFileName, int chan,DWORD sttime,DWORD edtime,HWND hWnd,IPlayBack* &pPlayBack) 
{
	LONG _lPlayBackID;
	NET_TIME StartTime,StopTime;
	time_t stime = sttime;
	struct tm *today = NULL;

	today = localtime(&stime);
	StartTime.dwYear = today->tm_year + 1900;
	StartTime.dwMonth = today->tm_mon + 1;
	StartTime.dwDay = today->tm_mday;
	StartTime.dwHour = today->tm_hour;
	StartTime.dwMinute = today->tm_min;
	StartTime.dwSecond = today->tm_sec;

	stime = edtime;
	today = localtime(&stime);

	StopTime.dwYear = today->tm_year + 1900;
	StopTime.dwMonth = today->tm_mon + 1;
	StopTime.dwDay = today->tm_mday;
	StopTime.dwHour = today->tm_hour;
	StopTime.dwMinute = today->tm_min;
	StopTime.dwSecond = today->tm_sec;

	//Search
	NET_RECORDFILE_INFO m_netFileInfo[20];    //fgl 2013/2/1  begin
	int nMaxLen = 20 * sizeof(NET_RECORDFILE_INFO);
	int nFileCount = 0;
	BOOL bSuccess = CLIENT_QueryRecordFile(lUserID,chan,0,
		&StartTime,&StopTime,NULL,m_netFileInfo,nMaxLen,&nFileCount,5000,FALSE);
		//_lPlayBackID = CLIENT_PlayBackByTimeEx(lUserID,chan,&StartTime,&StopTime,hWnd,NULL,(DWORD)this,DataCallBack,(DWORD)this);
	if(nFileCount>0)
		_lPlayBackID = CLIENT_PlayBackByRecordFileEx(lUserID,&m_netFileInfo[0],hWnd,NULL,0,DataCallBack,0);  //fgl 2013/2/1   end
	if (_lPlayBackID != 0) 
	{
		IHikPlayBack *lpipbk;
		lpipbk = new IHikPlayBack;
		lpipbk->lPlayBackID = _lPlayBackID;
		//2011-11-14
		lpipbk->controlplay_handle=NULL;
		lpipbk->controlplay_handle = CreateEvent(NULL, TRUE,FALSE,NULL);
		SetEvent(lpipbk->controlplay_handle);

		m_map_playback.SetAt(_lPlayBackID,(CObject*)lpipbk);
		pPlayBack = lpipbk;
		return true;
	}
	else 
	{
		return CLIENT_GetLastError();
		
	}
	//// IPlayBack* _PlayBack;

	//if (_PlayBack.lPlayBackID >= 0)
	//	return false;

	//_lPlayBackID = NET_DVR_PlayBackByName(lUserID, sPlayBackFileName, hWnd);
	//if (_lPlayBackID != -1) {

	//	_PlayBack.lPlayBackID = _lPlayBackID;
	//	*pPlayBack = &_PlayBack;
	//	return true;
	//}
	//else {
	//	 NET_DVR_GetLastError();

	//	return false;
	//}
	return 0;
}

// -----------------|-----------------------------|-----------------------
// -----------------| TFindFIleThread类    线程 　|
// -----------------|-----------------------------|----------------------
bool __stdcall TFindFileThread::Init(LONG lUserID,
	                                 LONG lChannel, DWORD dwFileType, DWORD dwStartTime, DWORD dwStopTime,
	                                 IDVRUser* pUser, DWORD UserData)
{
	_lUserID = lUserID;
	_lChannel = lChannel;
	_dwStartTime = dwStartTime;
	_dwStopTime = dwStopTime;
	_dwFileType = dwFileType;
	_pUser = pUser;
	dwUserData = UserData;
    hThread = (HANDLE)_beginthreadex( NULL, 0, &TFindFileThread::Execute, (LPVOID)this, 0, &threadID );
    if(hThread == NULL)
      return false;
    else
      return true;
}
// 查找文件线程 2011-10-10
unsigned int  __stdcall TFindFileThread::Execute(void * pParam)
	{
	char *data;
	int count = 0;
	TFindFileThread* pFindFileThread = (TFindFileThread*)pParam;
	data = new char[20 * sizeof(FILEINFO)];
	// TList *RecFileList = new TList();
	//struct tm dtStartTime,dtStopTime;
	//TDateTime dtStartTime, dtStopTime;
	//TDateTime dtFileDate, dtFileTime;
	NET_TIME StartTime, StopTime;
	time_t stime = pFindFileThread->_dwStartTime;

	struct tm *today = NULL;
	today = localtime(&stime);

	StartTime.dwYear = today->tm_year + 1900;
	StartTime.dwMonth = today->tm_mon + 1;
	StartTime.dwDay = today->tm_mday;
	StartTime.dwHour = today->tm_hour;
	StartTime.dwMinute = today->tm_min;
	StartTime.dwSecond = today->tm_sec;

	stime = pFindFileThread->_dwStopTime;
	today = localtime(&stime);

	StopTime.dwYear = today->tm_year + 1900;
	StopTime.dwMonth = today->tm_mon + 1;
	StopTime.dwDay = today->tm_mday;
	StopTime.dwHour = today->tm_hour;
	StopTime.dwMinute = today->tm_min;
	StopTime.dwSecond = today->tm_sec;

	//Search
	NET_RECORDFILE_INFO m_netFileInfo[20];
	int nMaxLen = 20 * sizeof(NET_RECORDFILE_INFO);
	int nFileCount = 0;

	BOOL bSuccess = CLIENT_QueryRecordFile(pFindFileThread->_lUserID,pFindFileThread->_lChannel,pFindFileThread->_dwFileType,
		&StartTime,&StopTime,NULL,m_netFileInfo,nMaxLen,&nFileCount,5000,FALSE);
	if(bSuccess)
	{
		if(0 == nFileCount)
		{
			OutputDebugString(_T("\r\nfindfile 0 num"));
		}
		else
		{
			if (nFileCount>20)
				{
				nFileCount =20;
				}
			for(int i=0;i<nFileCount;i++)
				{
				NET_RECORDFILE_INFO netFileInfo = m_netFileInfo[i];
				FILEINFO RecFileInfo;
				ZeroMemory(&RecFileInfo, sizeof(FILEINFO));
				CString strStartTime;
				strStartTime.Format(_T("%d-%d-%d %d:%d:%d.000"),netFileInfo.starttime.dwYear,
					netFileInfo.starttime.dwMonth,netFileInfo.starttime.dwDay,netFileInfo.starttime.dwHour,
					netFileInfo.starttime.dwMinute,netFileInfo.starttime.dwSecond);
				CString strEndTime;
				strEndTime.Format(_T("%d-%d-%d %d:%d:%d.000"),netFileInfo.endtime.dwYear,
					netFileInfo.endtime.dwMonth,netFileInfo.endtime.dwDay,netFileInfo.endtime.dwHour,
					netFileInfo.endtime.dwMinute,netFileInfo.endtime.dwSecond);
				sprintf_s(RecFileInfo.StartTime,"%S",strStartTime);
				sprintf_s(RecFileInfo.StopTime,"%S",strEndTime);
//				sprintf(RecFileInfo.FileName,"%S",netFileInfo.filename);  //fgl
				RecFileInfo.FileSize =(DWORD) netFileInfo.size;

				memcpy(data + count*sizeof(FILEINFO),(char*)&RecFileInfo, sizeof(FILEINFO));
				count = count + 1;
				}

			
		}
	}
	else
	{
		DWORD rt = CLIENT_GetLastError();
		CString str_tmp;
		str_tmp.Format(_T("\r\n findfile err = %u"), rt);
		OutputDebugString(str_tmp);
	}
	// 调回调函数

	pFindFileThread->_pUser->lpFindFileCallBack(pFindFileThread->_pUser, pFindFileThread->_lChannel, false, count, data,
		pFindFileThread->dwUserData);

	delete[]data;
	data = NULL;


	_endthreadex(pFindFileThread->threadID);
	return 0;
	}
// 查找文件线程初始化函数
/*__fastcall TFindFileThread::TFindFileThread(bool CreateSuspended, LONG lUserID,
	LONG lChannel, DWORD dwFileType, DWORD dwStartTime, DWORD dwStopTime,
	IDVRUser* pUser, DWORD UserData) : TThread(CreateSuspended) {
	_lUserID = lUserID;
	_lChannel = lChannel;
	_dwStartTime = dwStartTime;
	_dwStopTime = dwStopTime;
	_dwFileType = dwFileType;
	_pUser = pUser;
	dwUserData = UserData;

}

// 退出线程
void __fastcall TFindFileThread::Exit() {
	NET_DVR_FindClose(lFindID);
	Terminate();
	WaitFor();
}*/

LONG __stdcall TFindFileThread::GetID() {
	return lFindID;
}


 // 查找日志
DWORD __stdcall IHikDVRUser::FindLog(LONG lSelectMode, DWORD dwMajorType,
	DWORD dwMinorType, DWORD StartTime, DWORD StopTime) {
	return 1;
}

// 结束查找日志
DWORD __stdcall IHikDVRUser::FindLogClose(LONG lFLHandle) {
	return 0;
}

// 下载文件
DWORD __stdcall IHikDVRUser::DownloadFileByFile(char *sFileName, char* sSaveFileName,
	IDownloadFile* &pDownloadFile) {
		return 1;
}
DWORD __stdcall IHikDVRUser::DownloadFileByTime(int chan, DWORD sttime, DWORD edtime, char* sSaveFileName, IDownloadFile* &pDownloadFile)
{
	LONG _lFileHandle;
	int iRet = 0;
	NET_TIME StartTime,StopTime;
	time_t stime = sttime;
	struct tm *today = NULL;

	today = localtime(&stime);
	StartTime.dwYear = (DWORD)today->tm_year + 1900;
	StartTime.dwMonth = (DWORD)today->tm_mon + 1;
	StartTime.dwDay = (DWORD)today->tm_mday;
	StartTime.dwHour = (DWORD)today->tm_hour;
	StartTime.dwMinute = (DWORD)today->tm_min;
	StartTime.dwSecond = (DWORD)today->tm_sec;

	stime = edtime;
	today = localtime(&stime);

	StopTime.dwYear = (DWORD)today->tm_year + 1900;
	StopTime.dwMonth = (DWORD)today->tm_mon + 1;
	StopTime.dwDay = (DWORD)today->tm_mday;
	StopTime.dwHour = (DWORD)today->tm_hour;
	StopTime.dwMinute = (DWORD)today->tm_min;
	StopTime.dwSecond = (DWORD)today->tm_sec;

	//Search
	int nMaxLen = 20 * sizeof(NET_RECORDFILE_INFO);
	int nFileCount = 0;

//	BOOL bSuccess = CLIENT_QueryRecordFile(lUserID,chan,0,
//		&StartTime,&StopTime,NULL,m_netFileInfo,nMaxLen,&nFileCount,5000,FALSE);

	//_lFileHandle = CLIENT_DownloadByTime(lUserID,chan,0,&StartTime,&StopTime,sSaveFileName,NULL,0);
	IHikDownloadFile *lpDownloadFile; //fgl 2013/2/1
	lpDownloadFile = new IHikDownloadFile;
	_lFileHandle = CLIENT_DownloadByTime(lUserID,chan,0,&StartTime,&StopTime,sSaveFileName,lpTimeDownLoadPosCallBack,(DWORD)lpDownloadFile);

// 	if(nFileCount>0)
// 		_lFileHandle = CLIENT_DownloadByRecordFile(lUserID,&m_netFileInfo[0],sSaveFileName,PlayCallBack,(DWORD)lpDownloadFile);
	if (_lFileHandle!=0)
	{
		lpDownloadFile->lFileHandle = _lFileHandle;
		lpDownloadFile->dwUserData = chan;
		pDownloadFile = lpDownloadFile;
		return TRUE;
	}
	else
	{
		delete lpDownloadFile;   //当下载失败时，删除指针
		lpDownloadFile = NULL;
		iRet = CLIENT_GetLastError();
	}
		
	return iRet;
}

// 退出DVR
void __stdcall IHikDVRUser::Release() {


	CLIENT_Logout(lUserID);

	DVRListNode *Node = NULL;
	WaitForSingleObject(AlarmEvent,3000);
	ResetEvent(AlarmEvent);
	std::list<void*>::iterator DVRListIt = DVRList.begin();
	while(DVRListIt!=DVRList.end())
	{
		Node = (DVRListNode*)(*DVRListIt);
		if (Node->lUserID == lUserID) {
			DVRList.erase(DVRListIt);
			delete Node;
			Node = NULL;   //glfu 注释掉 先delete 后置空
			break;
		}
		DVRListIt++;
	}
// 	Node = NULL;   //glfu 注释掉 先delete 后置空
// 	delete Node;
	SetEvent(AlarmEvent);


	/************************************************************************/
	/* 清除退出登录设备的通道信息 2014-1-8 TGX                              */
	CString str_tmp;
	DEVICEDES* pd = NULL;
	str_tmp.Format(_T("%ld"), lUserID);
	if (m_chanNameMap.Lookup(str_tmp, (CObject *&)pd))
	{
		m_chanNameMap.RemoveKey(str_tmp);
		if (pd != NULL)
		{
			delete pd;
			pd = NULL;
		}
	}
	//2017-08-09
	NVR_IPC *pa=NULL;
	if(m_map_NVR_IPC.Lookup(str_tmp,(CObject *&)pa))
	{
		m_map_NVR_IPC.RemoveKey(str_tmp);
		if (pa != NULL)
		{
			delete pa;
			pa = NULL;
		}
	}
	/************************************************************************/
	//*******CLEANER
	bUsed = false;//2011-04-21
}

void __stdcall IHikDVRUser::RebootDVR() {
/*	NET_DVR_RebootDVR(lUserID);*/
	int ret = 0;
	ret = CLIENT_RebootDev(lUserID);      //glfu  2012-1-11   重启设备
	////////////////////////////////////////////////////////////////////////// 在设备重启时，把该设备的信息删掉
// 	DVRListNode *Node;
// 	std::list<void*>::iterator DVRListIt = DVRList.begin();
// 	while(DVRListIt!=DVRList.end())
// 	{
// 		Node = (DVRListNode*)(*DVRListIt);
// 		if (Node->lUserID == lUserID) {
// 			DVRList.erase(DVRListIt);
// 			break;
// 		}
// 		DVRListIt++;
// 	}
// 	Node = NULL;
// 	delete Node;
// 	bUsed = false;
}

DWORD __stdcall IHikDVRUser::StartListen( char *hostIP,DWORD port )
{
	BOOL ret=-1 ;
	DWORD wPort = 0;
	wPort = port;
	ret =  CLIENT_StartListenEx(lUserID);
	if (TRUE==ret)
	{
		//	lListenHandle = CLIENT_StartService(wPort,hostIP,MessServiceCallBack);
		OutputDebugString(_T("\r\n------------DH_StartListen-------Success----"));
		CLIENT_SetDVRMessCallBack(MessageCallBack,NULL);

	}
	srand((unsigned)time(0));
	DWORD dwTimes = 600+rand()%2400;
	SetTimer(AgentWnd,2,dwTimes*100,OnTimer);  //4~5分钟重复发一次出移动侦测报警外的其他报警
	dwTimes = 600+rand()%3000;
	SetTimer(AgentWnd,3,dwTimes*100,OnTimer);  //动检报警，5~6分钟重复检测

	return ret;
}

DWORD __stdcall IHikDVRUser::StopListen()
{
	BOOL ret = 0 ;
	ret = CLIENT_StopListen(lUserID);
	if (TRUE==ret)
	{
		ret = CLIENT_StopService(lListenHandle);
	}
	AlarmMotion = 0;   
	AlarmViLost = 0;	
	AlarmShelter = 0;	
	MotionType = 0;		
	ViLostType = 0;			
	ShelterType = 0;		
	preMotion = 0;		
	preViLost = 0;			
	preShelter = 0;			
	preAlarm = 0;			
	preDiskFull = 0;		
	preHardDisk = 0;		
	preMoTimer = 0;		
	KillTimer(AgentWnd,2);
	KillTimer(AgentWnd,3);
	return ret;
}

DWORD __stdcall IHikDVRUser::GetDVRConfig_TCP( LONG lChannel,LPVOID lpOutBuffer, DWORD dwCommand, LPDWORD lpBytesReturned )
{
	DWORD ret = 0;//ok
	int nCommand = 0;
	int lpRet = 0;
	int i = 0 , j = 0;
	DWORD nRetLen = 0;
	
	
	switch(dwCommand)
	{
	case 1:
		nCommand = DH_DEV_CHANNELCFG;      //通道参数
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_CHANNEL_CFG)*16;
		else
			lpRet = sizeof(DHDEV_CHANNEL_CFG);
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,sizeof(DHDEV_CHANNEL_CFG),&nRetLen);
		break;
	case 2:
		nCommand = DH_DEV_COMMCFG;       //串口参数
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_COMM_CFG)*16;
		else
			lpRet = sizeof(DHDEV_COMM_CFG);
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,lpRet,&nRetLen);
		break;
	case 3:
		nCommand = DH_DEV_RECORDCFG;      //定时录像参数
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_RECORD_CFG)*16;
		else
			lpRet = sizeof(DHDEV_RECORD_CFG);
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,lpRet,&nRetLen);
		break;
	case 4:
		nCommand = DH_DEV_NETCFG;			//网络参数
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_NET_CFG)*16;
		else
			lpRet = sizeof(DHDEV_NET_CFG);
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,lpRet,&nRetLen);
		break;
	case 5:
		nCommand = DH_DEV_DEVICECFG;   // 设备参数
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_SYSTEM_ATTR_CFG)*16;
		else
		{
			lpRet = sizeof(DHDEV_SYSTEM_ATTR_CFG);
		}
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,lpRet,&nRetLen);
		break;
	case 6:
		break;
	case 7:
		nCommand = DH_DEV_VIDEO_COVER ; //多区域遮挡
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_VIDEOCOVER_CFG)*16;
		else
			lpRet = sizeof(DHDEV_VIDEOCOVER_CFG);
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,lpRet,&nRetLen);
		break;
	case 8:
		nCommand = DH_DEV_TIMECFG;		//DVR时间
		if(lChannel == -1)
			lpRet = sizeof(NET_TIME)*16;
		else
			lpRet = sizeof(NET_TIME);
		ret = CLIENT_GetDevConfig(lUserID,nCommand,lChannel,lpOutBuffer,lpRet,&nRetLen);
		break;
	case 9:
		nCommand = DH_DEVSTATE_DSP;		//查询设备的处理能力
		if(lChannel == -1)
			lpRet = sizeof(DHDEV_DSP_ENCODECAP)*16;
		else
			lpRet = sizeof(DHDEV_DSP_ENCODECAP);
		ret = CLIENT_QueryDevState(lUserID,nCommand,(char*)lpOutBuffer,lpRet,(int*)&nRetLen);
		break;
	case 10:
		nCommand = DH_DEVSTATE_COMM_ALARM;			//查询设备通道数
		if(lChannel == -1)
			lpRet = sizeof(NET_CLIENT_STATE)*16;
		else
			lpRet = sizeof(NET_CLIENT_STATE);
		ret = CLIENT_QueryDevState(lUserID,nCommand,(char*)lpOutBuffer,lpRet,(int*)&nRetLen);
		break;
	case 11:   //本地报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_LOCALALARM_CFG,-1,lpOutBuffer,sizeof(DH_ALARMIN_CFG_EX)*16,&nRetLen);
		break;
	case 12:   //网络报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_NETALARM_CFG,-1,lpOutBuffer,sizeof(DH_ALARMIN_CFG_EX)*16,&nRetLen);
		break;
	case 13:  //移动报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_MOTIONALARM_CFG,-1,lpOutBuffer,sizeof(DH_MOTION_DETECT_CFG_EX)*16,&nRetLen);
		break;
	case 14: //视频丢失报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_VIDEOLOSTALARM_CFG,-1,lpOutBuffer,sizeof(DH_VIDEO_LOST_CFG_EX)*16,&nRetLen);
		break;
	case 15:   //遮挡报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_BLINDALARM_CFG,-1,lpOutBuffer,sizeof(DH_BLIND_CFG_EX)*16,&nRetLen);
		break;
	case 16:  //硬盘报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_DISKALARM_CFG,0,lpOutBuffer,sizeof(DH_DISK_ALARM_CFG_EX),&nRetLen);
		break;
	case 17:	// 网络异常报警
		ret = CLIENT_GetDevConfig(lUserID,DH_DEV_NETBROKENALARM_CFG,0,lpOutBuffer,sizeof(DH_NETBROKEN_ALARM_CFG_EX),&nRetLen);
		break;
		
	}
	lpBytesReturned = &nRetLen;
	return ret ;
}
AV_CFG_ChannelName stInfo = {sizeof(stInfo)};
DWORD __stdcall IHikDVRUser::SetDVRConfig_TCP( LONG lChannel,LPVOID lpInBuffer, DWORD dwCommand, LPDWORD lpBytesReturned )
{
	DWORD ret = 0;//ok
	int nCommand = 0;
	//2017-08-14
	CString strid;
	NVR_IPC *_pa=NULL;
	int nSpecialValue = 1;
	int nerror;
	NET_DEVICEINFO_Ex DHDeviceInfo;
	strid.Format(_T("%u"),lUserID);	
	DHDEV_CHANNEL_CFG lp_DHDEV_CHANNEL_CFG;
	CString strtmp;
	
	int nBufSize = 1024*1024;
	char* pBuf = new char[nBufSize];
	switch(dwCommand)
	{
	case 1:
		nCommand = DH_DEV_CHANNELCFG;      //通道参数
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,sizeof(DHDEV_CHANNEL_CFG));
		break;
	case 2:
		nCommand = DH_DEV_COMMCFG;       //串口参数
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 3:
		nCommand = DH_DEV_RECORDCFG;      //定时录像参数
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 4:
		nCommand = DH_DEV_NETCFG;			//网络参数
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 5:
		nCommand = DH_DEV_DEVICECFG;   // 设备参数
//		memcpy(&pAlarmSchedule,lpInBuffer,*lpBytesReturned);
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 6:
		nCommand = DH_DEV_ALARMCFG;		// 报警布防参数
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 7:
		nCommand = DH_DEV_VIDEO_COVER ; //多区域遮挡
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 8:
		nCommand = DH_DEV_TIMECFG;   //时间设置
		ret = CLIENT_SetDevConfig(lUserID,nCommand,lChannel,lpInBuffer,*lpBytesReturned);
		break;
	case 11:  //本地报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_LOCALALARM_CFG,-1,lpInBuffer,*lpBytesReturned);
		break;
	case 12:  //网络报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_NETALARM_CFG,-1,lpInBuffer,*lpBytesReturned);
		break;
	case 13:  //移动报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_MOTIONALARM_CFG,-1,lpInBuffer,*lpBytesReturned);
		break;
	case 14:	//视频丢失报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_VIDEOLOSTALARM_CFG,-1,lpInBuffer,*lpBytesReturned);
		break;
	case 15:  //视频遮挡报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_BLINDALARM_CFG,-1,lpInBuffer,*lpBytesReturned);
		break;
	case 16:  //硬盘报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_DISKALARM_CFG,0,lpInBuffer,*lpBytesReturned);
		break;
	case 17:  //网络异常报警
		ret = CLIENT_SetDevConfig(lUserID,DH_DEV_NETBROKENALARM_CFG,0,lpInBuffer,*lpBytesReturned);
		break;

	//2017-08-15 特殊处理连接对应IPC
	case 301://设置通道名
		memcpy_s(&lp_DHDEV_CHANNEL_CFG,sizeof(DHDEV_CHANNEL_CFG),lpInBuffer,sizeof(DHDEV_CHANNEL_CFG));		
		if (m_map_NVR_IPC.Lookup(strid,(CObject*&)_pa))
		{	
			strtmp.Format(_T("\r\n%S:%d:%S:%S"),_pa->ip[lChannel],_pa->port[lChannel],_pa->user[lChannel],_pa->passwd[lChannel]);
			OutputDebugString(strtmp);
			LONG lid = CLIENT_LoginEx2(_pa->ip[lChannel],_pa->port[lChannel],_pa->user[lChannel],_pa->passwd[lChannel],(EM_LOGIN_SPAC_CAP_TYPE)8,&nSpecialValue,&DHDeviceInfo,&nerror);
			if(lid != 0)
			{					
				memcpy_s(stInfo.szName,sizeof(stInfo),lp_DHDEV_CHANNEL_CFG.szChannelName,sizeof(stInfo));
				memset(pBuf,0,nBufSize);
				if(CLIENT_PacketData(CFG_CMD_CHANNELTITLE,&stInfo,sizeof(stInfo),pBuf,nBufSize))
				{
					ret = CLIENT_SetNewDevConfig(lid,CFG_CMD_CHANNELTITLE,0,pBuf,nBufSize,&nerror,0,5000);
				}
				else
				{
					ret =0;
				}			
				CLIENT_Logout(lid);

				
			}
		}

		break;
	}
	if(pBuf !=NULL)
	{
		delete[] pBuf;
		pBuf =NULL;
	}
	return ret ;
}

//关闭设备 2012-1-11
DWORD __stdcall IHikDVRUser::ShutDownDVR()
{
	int ret = 0;
	ret = CLIENT_ShutDownDev(lUserID);
// 	DVRListNode *Node;
// 	std::list<void*>::iterator DVRListIt = DVRList.begin();
// 	while(DVRListIt!=DVRList.end())
// 	{
// 		Node = (DVRListNode*)(*DVRListIt);
// 		if (Node->lUserID == lUserID) {
// 			DVRList.erase(DVRListIt);
// 			break;
// 		}
// 		DVRListIt++;
// 	}
// 	Node = NULL;
// 	delete Node;
// 	bUsed = false;
	return ret ;
}
//glfu 2012-1-12
DWORD __stdcall IHikDVRUser::QueryDVRState()
{
	int ret = 0;
	int  byteReturned = 0;
/*	char buf[1024] = {0};*/
	JF_DVR_WorkState dvrWorkState;
	ZeroMemory(&dvrWorkState,sizeof(JF_DVR_WorkState));
	ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_COMM_ALARM,(char*)&dvrWorkState.clientState,sizeof(NET_CLIENT_STATE),&byteReturned);
	if (ret)
	{
		//获取码流
		const int channum = 3;
		DWORD dwBitRate[channum] = {0};
		ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_BITRATE,(char*)/*dvrWorkState.dwBitRate*/dwBitRate,channum*sizeof(DWORD),&byteReturned);		//2012-2-16 glfu无法查询
	}
	ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_DISK,(char*)&dvrWorkState.hardDiskState,sizeof(DH_HARDDISK_STATE),&byteReturned);
	ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_SHELTER_ALARM,(char*)&dvrWorkState.shelterAlarm,16,&byteReturned);
	ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_RECORDING,(char*)&dvrWorkState.recording,16,&byteReturned);
	ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_RESOURCE,(char*)&dvrWorkState.dwResource,3*sizeof(DWORD),&byteReturned);			//2012-2-16 glfu无法查询
	DWORD dwNetConn[11] = {0};
	ret = CLIENT_QueryDevState(lUserID,DH_DEVSTATE_CONN,(char*)/*&dvrWorkState.dwNetConn*/dwNetConn,11*sizeof(DWORD),&byteReturned);					//2012-2-16 glfu无法查询
	DVRListNode *Node;
	std::list<void*>::iterator DVRListIt = DVRList.begin();
	while(DVRListIt!=DVRList.end())
	{
		Node = (DVRListNode*)(*DVRListIt);
		if (Node->lUserID == lUserID) {
			if ((Node->pDVRUser)->lpFMessCallBack!=NULL)
			{
				(Node->pDVRUser)->lpFMessCallBack(Node->pDVRUser,200,(char*)&dvrWorkState,sizeof(JF_DVR_WorkState));
				OutputDebugString(_T("\r\n DH Work state working once!"));
				break;
			}
		}
		DVRListIt++;
	}
// 	if (ret)
// 	{
// 		memcpy(&clientState,buf,byteReturned);
// 	}
	
	return ret;
}

/*void __fastcall TFindFileThread::Execute() {
	char *data;
	int count = 0;
	data = new char[500 * sizeof(FILEINFO)];
	// TList *RecFileList = new TList();
	TDateTime dtStartTime, dtStopTime;
	TDateTime dtFileDate, dtFileTime;
	NET_DVR_TIME StartTime, StopTime;
	NET_DVR_FIND_DATA FindData;
	DWORD dwStartTime, dwStopTime;
	time_t stime = _dwStartTime;

	struct tm *today;
	today = localtime(&stime);

	StartTime.dwYear = today->tm_year + 1900;
	StartTime.dwMonth = today->tm_mon + 1;
	StartTime.dwDay = today->tm_mday;
	StartTime.dwHour = today->tm_hour;
	StartTime.dwMinute = today->tm_min;
	StartTime.dwSecond = today->tm_sec;

	stime = _dwStopTime;
	today = localtime(&stime);

	StopTime.dwYear = today->tm_year + 1900;
	StopTime.dwMonth = today->tm_mon + 1;
	StopTime.dwDay = today->tm_mday;
	StopTime.dwHour = today->tm_hour;
	StopTime.dwMinute = today->tm_min;
	StopTime.dwSecond = today->tm_sec;


	while (!Terminated) {
		lFindID = NET_DVR_FindFile(_lUserID, _lChannel, _dwFileType,
			&StartTime, &StopTime);
		if (lFindID != -1) {

			LONG lRet;
			do {
				lRet = NET_DVR_FindNextFile(lFindID, &FindData);
				if (lRet == 1002) {
					Sleep(5);
					continue;
				}
				if (lRet == 1000) {
					FILEINFO RecFileInfo;
					ZeroMemory(&RecFileInfo, sizeof(FILEINFO));
					// 记录文件信息

					dtFileDate = TDateTime(FindData.struStartTime.dwYear,
						FindData.struStartTime.dwMonth,
						FindData.struStartTime.dwDay);
					dtFileTime = TDateTime(FindData.struStartTime.dwHour,
						FindData.struStartTime.dwMinute,
						FindData.struStartTime.dwMinute, 0);
					dtFileDate = dtFileDate + dtFileTime;
					dwStartTime = DateTimeToFileDate(dtFileDate);

					dtFileDate = TDateTime(FindData.struStopTime.dwYear,
						FindData.struStopTime.dwMonth,
						FindData.struStopTime.dwDay);
					dtFileTime = TDateTime(FindData.struStopTime.dwHour,
						FindData.struStopTime.dwMinute,
						FindData.struStopTime.dwMinute, 0);
					dtFileDate = dtFileDate + dtFileTime;
					dwStopTime = DateTimeToFileDate(dtFileDate);

					strcpy(RecFileInfo.FileName, FindData.sFileName);
					RecFileInfo.StartTime = dwStartTime;
					RecFileInfo.FileLen = FindData.dwFileSize;
					RecFileInfo.TimeLen = dwStopTime - dwStartTime;
					if (count >= 500) {
						break;
					}
					else {
						memcpy(data + count*sizeof(FILEINFO),
							(char*) & RecFileInfo, sizeof(FILEINFO));
						count = count + 1;
					}
					// RecFileList->Add(&RecFileInfo);

				}

			}
			while (lRet != 1001 && lRet != -1 && lRet != 1003 && lRet != 1004);

			// 调回调函数

			_pUser->lpFindFileCallBack(_pUser, _lChannel, false, count, data,
				dwUserData);

			delete[]data;
			data = NULL;
		}

		Exit();
	}
}

// -----------------|-----------------------------|-----------------------
// -----------------| TFindLogThread类  线程    　|
// -----------------|-----------------------------|----------------------
// 查找日志文件线程初始化函数
__fastcall TFindLogThread::TFindLogThread(bool CreateSuspended, LONG lUserID,
	LONG lSelectMode, DWORD dwMajorType, DWORD dwMinorType, DWORD dwStartTime,
	DWORD dwStopTime, IDVRUser* pUser) : TThread(CreateSuspended) {
	_lSelectMode = lSelectMode;
	_dwMajorType = dwMajorType;
	_dwMinorType = dwMinorType;
	_dwStartTime = dwStartTime;
	_dwStopTime = dwStopTime;
	_pUser = pUser;
	_lUserID = lUserID;

}

// 退出线程
void __fastcall TFindLogThread::Exit() {
	// 退出线程
	Terminate();
	WaitFor();
}

// 查找日志文件线程
void __fastcall TFindLogThread::Execute() // 查找文件线程
{
	// LONG lFindID;
	NET_DVR_TIME StartTime, StopTime;
	NET_DVR_LOG FindLog;
	TDateTime dtStartTime, dtStopTime;
	TDateTime dtFileDate, dtFileTime;
	dtStartTime = FileDateToDateTime(_dwStartTime);
	dtStopTime = FileDateToDateTime(_dwStopTime);
	DWORD dwStartTime, dwStopTime;
	unsigned short Year, Month, Day, Hour, Minute, Second, Msec;
	dtStartTime.DecodeDate(&Year, &Month, &Day);
	dtStartTime.DecodeTime(&Hour, &Minute, &Second, &Msec);
	StartTime.dwYear = Year;
	StartTime.dwMonth = Month;
	StartTime.dwDay = Day;
	StartTime.dwHour = Hour;
	StartTime.dwMinute = Minute;
	StartTime.dwSecond = Second;
	dtStopTime.DecodeDate(&Year, &Month, &Day);
	dtStopTime.DecodeTime(&Hour, &Minute, &Second, &Msec);
	StopTime.dwYear = Year;
	StopTime.dwMonth = Month;
	StopTime.dwDay = Day;
	StopTime.dwHour = Hour;
	StopTime.dwMinute = Minute;
	StopTime.dwSecond = Second;
	LONG lFindLogID;
	while (!Terminated) {
		lFindLogID = NET_DVR_FindDVRLog(_lUserID, _lSelectMode, _dwMajorType,
			_dwMinorType, &StartTime, &StopTime);
		LONG lRet;
		do {
			lRet = NET_DVR_FindNextLog(lFindLogID, &FindLog);
			if (lRet == 1000) {
				LOGINFO *LogInfo;

				// 记录文件信息
				dtFileDate = TDateTime(FindLog.strLogTime.dwYear,
					FindLog.strLogTime.dwMonth, FindLog.strLogTime.dwDay);
				dtFileTime = TDateTime(FindLog.strLogTime.dwHour,
					FindLog.strLogTime.dwMinute, FindLog.strLogTime.dwMinute,
					0);
				dtFileDate = dtFileDate + dtFileTime;
				dwStartTime = DateTimeToFileDate(dtFileDate);
				LogInfo->StartTime = dwStartTime;
				LogInfo->Type = FindLog.dwMajorType;

			}
		}
		while (lRet != 1001 || lRet != -1 || lRet != 1003 || lRet != 1004);
		NET_DVR_FindLogClose(lFindLogID);
		// 调用线程
		Exit();
	}
}*/

// ---------------------------------------------------------------------------


DWORD __stdcall IHikDownloadFile::GetDownloadPos( DWORD* dwPos )
{
	int downloadSize = 0;
	int iPos = 0;
	CString temp;
	BOOL iRet = CLIENT_GetDownloadPos(lFileHandle,&iPos,(int*)dwPos);
// 	temp.Format(_T("\r\niRet=%d,TotalSize%d;DownLoadSize%d"),iRet,iPos,*dwPos);
// 	OutputDebugString(temp);
	return iRet;
}

void  __stdcall IHikDownloadFile::Release()
{
	CLIENT_StopDownload(lFileHandle);
	IHikDownloadFile *pa = NULL;
	pa = this;
	delete pa;
	pa = NULL;

}

//DWORD __stdcall IHikDVRUser::GetAllChanName(LPDEVICEDES *lpDeviceChan)  //2013/3/27  glfu 添加获取全部通道标签
//{
//	DHDEV_SYSTEM_ATTR_CFG sysConfigInfo = {0};
//	DWORD dwRetLen = 0;
//	BOOL bSuccess = FALSE;
//	bSuccess = CLIENT_GetDevConfig(lUserID, DH_DEV_DEVICECFG, 0,&sysConfigInfo, sizeof(DHDEV_SYSTEM_ATTR_CFG), &dwRetLen, 1000);
//	if (bSuccess && dwRetLen == sizeof(DHDEV_SYSTEM_ATTR_CFG))
//	{
//		BYTE nChannelCount = sysConfigInfo.byVideoCaptureNum;
//		DHDEV_CHANNEL_CFG *pChannelInfo = new DHDEV_CHANNEL_CFG[nChannelCount];
//		memset(pChannelInfo, 0, nChannelCount*sizeof(DHDEV_CHANNEL_CFG));
//		bSuccess = CLIENT_GetDevConfig(lUserID, DH_DEV_CHANNELCFG, -1,pChannelInfo, nChannelCount * sizeof(DHDEV_CHANNEL_CFG), &dwRetLen);
//		if (bSuccess && dwRetLen == nChannelCount * sizeof(DHDEV_CHANNEL_CFG))
//		{
//			lpDeviceChan->VideoNum = nChannelCount;
//			for (int i = 0;i<nChannelCount;i++)
//			{
//				//strcpy(lpDeviceChan->VideoLable[i],pChannelInfo[i].szChannelName);
//				memcpy(lpDeviceChan->VideoLable[i],pChannelInfo[i].szChannelName,sizeof(pChannelInfo[i].szChannelName));
//			}
//		}
//		delete[] pChannelInfo;
//	}
//	else
//	{
//		OutputDebugString(_T("\r\n在获取全部通道标签时，获取通道数失败"));
//	}
//		
//	return bSuccess;
//}
//
//DWORD __stdcall IHikDVRUser::SetAllChanName(LPDEVICEDES *lpDeviceChan)      //2013、3、27 设置全部通道标签
//{
//	DHDEV_SYSTEM_ATTR_CFG sysConfigInfo = {0};
//	DWORD dwRetLen = 0;
//	BOOL bSuccess = FALSE;
//	bSuccess = CLIENT_GetDevConfig(lUserID, DH_DEV_DEVICECFG, 0,&sysConfigInfo, sizeof(DHDEV_SYSTEM_ATTR_CFG), &dwRetLen, 1000);
//	if (bSuccess && dwRetLen == sizeof(DHDEV_SYSTEM_ATTR_CFG))
//	{
//		BYTE nChannelCount = sysConfigInfo.byVideoCaptureNum;
//		DHDEV_CHANNEL_CFG *pChannelInfo = new DHDEV_CHANNEL_CFG[nChannelCount];
//		memset(pChannelInfo, 0, nChannelCount*sizeof(DHDEV_CHANNEL_CFG));
//		bSuccess = CLIENT_GetDevConfig(lUserID, DH_DEV_CHANNELCFG, -1,pChannelInfo, nChannelCount * sizeof(DHDEV_CHANNEL_CFG), &dwRetLen);
//		if (bSuccess && dwRetLen == nChannelCount * sizeof(DHDEV_CHANNEL_CFG))
//		{
//			for(int i=0;i<nChannelCount;i++)
//			{
//				memcpy(pChannelInfo[i].szChannelName,lpDeviceChan->VideoLable[i],sizeof(pChannelInfo[i].szChannelName));
//			}
//		}
//		bSuccess = CLIENT_SetDevConfig(lUserID,DH_DEV_DEVICECFG,-1,pChannelInfo,nChannelCount*sizeof(DHDEV_CHANNEL_CFG));
//		delete[] pChannelInfo;
//	}
//	return bSuccess;
//}

DWORD __stdcall IHikDVRUser::GetAllSerialNumber_EX(int TypeSize,LPVOID sSerNumber)
{
	NVR_IPC NimaXu;//=NULL;
	RtlZeroMemory(&NimaXu,sizeof(NimaXu));
	BOOL bRet;
	DH_IN_MATRIX_GET_CAMERAS stInParam = {sizeof(stInParam)};
	DH_OUT_MATRIX_GET_CAMERAS stOutParam = {sizeof(stOutParam)};
	DHDEV_SYSTEM_ATTR_CFG DVRSnumber;
	DWORD outsize;
	CString strid;
	strid.Format(_T("%u"),lUserID);
	 NVR_IPC *_pa=NULL;
	
	 switch(TypeSize)
	 {
		 case 201:
		 case 501:
			 if (m_map_NVR_IPC.Lookup(strid,(CObject*&)_pa))
			 {
				memcpy_s(&NimaXu,sizeof(NVR_IPC),_pa,sizeof(NVR_IPC));
			 }
			 break;
	 }
	 memset(&DVRSnumber,0,sizeof(DHDEV_SYSTEM_ATTR_CFG));
	 bRet = CLIENT_GetDevConfig(lUserID,DH_DEV_DEVICECFG,0,&DVRSnumber,sizeof(DHDEV_SYSTEM_ATTR_CFG),&outsize);
	 if (bRet==FALSE)
	 {
		 return CLIENT_GetLastError();
	 }
	 memcpy_s(NimaXu.MSerNo,48,DVRSnumber.szDevSerialNo,48);  
	//switch(TypeSize)
	//{
	//	case 201:
	//	case 501:	
	//		
	//			//获取设备上的IPC
	//			if (m_map_NVR_IPC.Lookup(strid,(CObject*&)_pa))
	//			{
	//				for (int i=0;i<128;i++)
	//				{	
	//					memcpy_s(NimaXu.SerNO[i],48,_pa->SerNO[i],48);	 
	//				}
	//				//
	//				
	//			}		
	//			
	//}
	memcpy_s(sSerNumber,sizeof(NVR_IPC),&NimaXu,sizeof(NVR_IPC));		 
	 
	 return bRet;
}
//////////////////////////////////////////////////////////////////////////
//char* __stdcall IHikDVRUser::SetDVRConfig_OSD(DWORD dwCommand, LONG lChannel)//,LPVOID OSD_lpInBuffer, LPVOID lpInBuffer,LPVOID ReturnSize)
//char* __stdcall IHikDVRUser::SetDVRConfig_OSD(DWORD dwCommand, LONG lChannel,char* OSD_lpInBuffer, char* lpInBuffer,char* ReturnSize)
DWORD   __stdcall IHikDVRUser::SetDVRConfig_OSD_EX(DWORD dwCommand, LONG lChannel,char* OSD_lpInBuffer, char* lpInBuffer,char* ReturnSize)
 {
	 //DHDEV_CHANNEL_CFG lp_DHDEV_CHANNEL_CFG;
	 CString strid;
	 DWORD   dw_rets =0;//-1 1-1
	 strid.Format(_T("%u"),lUserID);	

	 NVR_IPC *_pa=NULL;
	 CString strtmp;
	 int nBufSize = 1024*1024;
	 char* pBuf = new char[nBufSize];
	 RtlZeroMemory(pBuf,nBufSize);
	 char* alot=new char[nBufSize];
	 RtlZeroMemory(alot,nBufSize);
	 int nSpecialValue = 1;
	 int nerror;
	 NET_DEVICEINFO_Ex DHDeviceInfo;
	 DWORD ret = 0; // 函数返回值
	 char  crets[65]={"1"}; //错误原因
	 char  czifubuf[65]={0};
	 LONG  lid= lUserID;
	 int  setnum=0;
	 int  setnum1=0;
	 //CString  strzif((char*)lpInBuffer);
	 //RtlZeroMemory(&lp_DHDEV_CHANNEL_CFG,sizeof(lp_DHDEV_CHANNEL_CFG));
	// memcpy_s(lp_DHDEV_CHANNEL_CFG.szChannelName,32,(char*)OSD_lpInBuffer,32);

	 int asize;
	 AV_CFG_VideoWidget absd;
	 char    ccmposd1[65]={0};
	 char    ccmposd2[65]={0};
	 char ALLchar[256]={0};
	 strcat_s(ALLchar,OSD_lpInBuffer);
	 strcat_s(ALLchar,lpInBuffer);
	 int allLen=strlen(ALLchar);
	 //------------------------------------------------------------------------------------
	 RtlZeroMemory(stInfo.szName,256);
	 memcpy_s(stInfo.szName,256,ALLchar,allLen);
	 memset(pBuf,0,nBufSize);
	 if(CLIENT_PacketData(CFG_CMD_CHANNELTITLE,&stInfo,sizeof(stInfo),pBuf,nBufSize))
	 {
		 ret = CLIENT_SetNewDevConfig(lid,CFG_CMD_CHANNELTITLE,lChannel,pBuf,nBufSize,&nerror,0,3000);
		 if (ret) //OSD设置成功
		 {

			 //读取OSD
			 GetOneChannelName(&m_pstChannelName[lChannel],lChannel,lid);
			 memcpy(ccmposd1, m_pstChannelName[lChannel].szName, AV_CFG_Channel_Name_Len);
			 //////////////////////////////////////////////////////////////////////////
			 //if (strzif.GetLength()>0)//有字符叠加则叠加

			 // for(int i =0 ;i<AV_CFG_Max_Video_Widget_Custom_Title;i++ )
			 // {
			 //	 absd.stuCustomTitle[i].nStructSize=sizeof(AV_CFG_VideoWidgetCustomTitle );
			 // }
			 // absd.nStructSize=sizeof(absd);

			 // ret =CLIENT_GetNewDevConfig(lid,CFG_CMD_VIDEOWIDGET,0,alot,nBufSize,&asize,2000);


			 // ret =CLIENT_ParseData(CFG_CMD_VIDEOWIDGET,alot,&absd,sizeof(absd),&asize);


			 // //memcpy(absd.stuCustomTitle[0].szText,(char*)lpInBuffer,64);
			 // //absd.stuCustomTitle[0].bEncodeBlend=1;
			 // //absd.stuCustomTitle[0].bPreviewBlend=1;
			 // absd.stuChannelTitle.stuRect.nLeft=0;
			 // absd.stuChannelTitle.stuRect.nRight=0;
			 // absd.stuChannelTitle.stuRect.nTop=7700;
			 // absd.stuChannelTitle.stuRect.nBottom=7700;

			 // //absd.stuCustomTitle[0].stuRect.nLeft=3100;
			 //// absd.stuCustomTitle[0].stuRect.nRight =3100;
			 // //absd.stuCustomTitle[0].stuRect.nTop=7700;
			 // //absd.stuCustomTitle[0].stuRect.nBottom=7700;

			 // ret=CLIENT_PacketData(CFG_CMD_VIDEOWIDGET,&absd,sizeof(absd),alot,nBufSize);
			 // ret=CLIENT_SetNewDevConfig(lid,CFG_CMD_VIDEOWIDGET,0,alot,nBufSize,&asize,&asize);

			 //memset(alot,0,nBufSize);
			 //ret =CLIENT_GetNewDevConfig(lid,CFG_CMD_VIDEOWIDGET,0,alot,nBufSize,&asize,2000);
			 // ret =CLIENT_ParseData(CFG_CMD_VIDEOWIDGET,alot,&absd,sizeof(absd),&asize);
			 //memcpy(ccmposd2,absd.stuCustomTitle[0].szText,64);

			 //if (strcmp(ccmposd1,OSD_lpInBuffer) ==0 && strcmp(ccmposd2,lpInBuffer) ==0)
			 //{
			 if (strncmp(ALLchar,m_pstChannelName[lChannel].szName,allLen)==0)
			 {
				 memcpy_s(ReturnSize,64,("设置成功 !"),strlen(("设置成功 !")));
			 }else
			 {
				 memcpy_s(ReturnSize,64,("设置失败 1!"),strlen(("设置失败 1!")));
			 }
			 dw_rets =1;
		 }
		 else
		 {								
			 memcpy_s(ReturnSize,64,("设置失败 2!"),strlen(("设置失败 2!")));
		 }
	 }
	 OutputDebugString(CA2CT(ReturnSize));
	 if (pBuf!=NULL)
	 {
		 delete[] pBuf;
		 pBuf = NULL;
	 }
	 if (alot!=NULL)
	 {
		 delete[] alot;
		 alot = NULL;
	 }
	 return dw_rets;
	 
 }
 DWORD __stdcall IHikDVRUser::GetNVRWorkState(LPVOID lpInput)
 {
	 NVRState* state=(NVRState*)lpInput;
	 state->HardDisk=1;
	 DWORD LinkState;
	 //连接状态
	 int BackSize;
	 BOOL Ret;//=CLIENT_QueryDevState(lUserID,DH_DEVSTATE_CONN,(char*)&LinkState,sizeof(DWORD),&BackSize);//废止
	 //state.WorkState=LinkState;
	 //存储开关
	 char VideoState[16];

	
	 Ret=CLIENT_QueryDevState(lUserID,DH_DEVSTATE_RECORDING,VideoState,16,&BackSize);
	 for (int i=0;i<16;i++)
	 {
		 if (1==VideoState[i])
		 {
			 state->SaveState=1;
		 }
	 }
	 //硬盘状态和空间
	 DH_HARDDISK_STATE DiskState;
	 RtlZeroMemory(&DiskState,sizeof(DH_HARDDISK_STATE));
	 int AlarmDisk;
	 Ret=CLIENT_QueryDevState(lUserID,DH_DEVSTATE_DISK,(char*)&DiskState,sizeof(DH_HARDDISK_STATE),&BackSize);
	 if(Ret)
	 {
		 for (int i=0;i<DiskState.dwDiskNum&&DiskState.stDisks[i].dwVolume!=0;i++)
		 {
			 state->dwVolume+=DiskState.stDisks[i].dwVolume;
			 state->dwFreeSpace+=DiskState.stDisks[i].dwFreeSpace;
			 AlarmDisk=DiskState.stDisks->dwStatus&15;
			 if (2==AlarmDisk)
			 {
				 state->HardDisk=0;
			 }
		 }
	 }

	 
	 return CLIENT_GetLastError()&(0x7fffffff);
 }
  DWORD __stdcall IHikDVRUser::GetIPCWorkState(LPVOID workstate)
  {
	  //CFG_CAPTURE_SIZE  分辨率对应的高宽
	  IPCState* StateIpc=(IPCState*)workstate;
	  int naBuf=1024*1024;
	  char* aBuf=new char[naBuf];
	  memset(aBuf,0,naBuf);
	  int Aerror;
	  CFG_ENCODE_INFO ImageChannel;
	  BOOL Ret=CLIENT_GetNewDevConfig(lUserID,CFG_CMD_ENCODE,0,aBuf,naBuf,&Aerror);
	  if (Ret)
	  {
		  Ret=CLIENT_ParseData(CFG_CMD_ENCODE,aBuf,(void*)&ImageChannel,sizeof(ImageChannel),NULL);
		  if (Ret)
		  {
			  StateIpc->MainVideoWidth=ImageChannel.stuMainStream[0].stuVideoFormat.nWidth;
			  StateIpc->MainVideoHeight=ImageChannel.stuMainStream[0].stuVideoFormat.nHeight;
			  StateIpc->DeputyStreamWidth=ImageChannel.stuExtraStream[0].stuVideoFormat.nWidth;
			  StateIpc->DeputyStreamHeight=ImageChannel.stuExtraStream[0].stuVideoFormat.nHeight;
		  }
	  }
	  delete []aBuf; 
	  NET_CLIENT_VIDEOLOST_STATE videoLost;
	  videoLost.dwSize=sizeof(videoLost);
	  Ret=CLIENT_QueryDevState(lUserID,DH_DEVSTATE_VIDEOLOST,(char*)&videoLost,sizeof(videoLost),&Aerror);
	  for (int i=0;i<videoLost.channelcount;i++)
	  {
		  if (1==videoLost.dwAlarmState[i])
		  {
			  StateIpc->VideoLost=3;
		  }
	  }
	  NET_CLIENT_VIDEOBLIND_STATE VideoBlind;
	  VideoBlind.dwSize=sizeof(VideoBlind);
	  Ret=CLIENT_QueryDevState(lUserID,DH_DEVSTATE_VIDEOBLIND,(char*)&VideoBlind,sizeof(VideoBlind),&Aerror);
	  for (int i=0;i<VideoBlind.channelcount;i++)
	  {
		  if(1==VideoBlind.dwAlarmState[i])
		  {
			  StateIpc->VideoLost=2;
		  }
	  }
	  return CLIENT_GetLastError()&(0x7fffffff);
  }

