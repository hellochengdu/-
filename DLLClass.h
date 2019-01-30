//---------------------------------------------------------------------------

#ifndef DLLClassH
#define DLLClassH
#include "HCNetSDK.h"
#include "dhnetsdk.h"
#include "dhconfigsdk.h"
#include "GlobeClass.h"
#include <list>
#include <process.h> 

#define MAX_NUM_DH 512
//-------------------------------------------------------
class TFindFileThread
{
public:
	HANDLE hThread;
	bool __stdcall Init(LONG lUserID,LONG lChannel,DWORD dwFileType,DWORD dwStartTime,DWORD dwStopTime,IDVRUser* pUser,DWORD UserData);
	LONG __stdcall GetID();
private:
	  LONG _lUserID;
	  LONG _lChannel;
	  DWORD _dwFileType;
	  DWORD _dwStartTime;
	  DWORD _dwStopTime;
	  IDVRUser* _pUser;
	  LONG lFindID;
	  DWORD dwUserData;
	  unsigned threadID;
	  static unsigned int  __stdcall Execute(void * pParam);
};
/*class TFindFileThread:public TThread
{
  protected:
	void __fastcall Execute();
  private:
	  LONG _lUserID;
	  LONG _lChannel;
	  DWORD _dwFileType;
	  DWORD _dwStartTime;
	  DWORD _dwStopTime;
	  IDVRUser* _pUser;
	  LONG lFindID;
	  DWORD dwUserData;
  public:

	void __fastcall Exit();
	LONG __fastcall GetID();
	__fastcall TFindFileThread(bool CreateSuspended,LONG lUserID,LONG lChannel,DWORD dwFileType,DWORD dwStartTime,DWORD dwStopTime,IDVRUser* pUser,DWORD UserData);
};*/
//---------------------------------------------------------------
struct RealPlayNode{         //播放列表
  LONG lPlayID;
  IRealPlay* pRealPlay;
  RealPlayNode(LONG _lPlayID,IRealPlay* _pRealPlay)
   {
     lPlayID = _lPlayID;
     pRealPlay = _pRealPlay;
   }
};
#define  AlamrNums  256

struct DVRListNode{
  LONG lUserID;
  IDVRUser* pDVRUser;
  char sDVRIP[16];
  int  lostsigal[AlamrNums];	//2015-07-29
  int  movealarm[AlamrNums];
  int  maskalarm[AlamrNums];
  int  redalarm[AlamrNums];
  DVRListNode(LONG _lUserID,IDVRUser* _pDVRUser,char* _sDVRIP)
   {
     lUserID = _lUserID;
     pDVRUser = _pDVRUser;
     strcpy_s(sDVRIP,_sDVRIP);
//     sDVRIP = _sDVRIP;

	 memset(lostsigal,0,sizeof(int)*AlamrNums);
	 memset(movealarm,0,sizeof(int)*AlamrNums);
	 memset(maskalarm,0,sizeof(int)*AlamrNums);
	 memset(redalarm,0,sizeof(int)*AlamrNums);
   }
};
/*struct FindFileThread{
   LONG lFFID;
   TFindFileThread* Thread;
   FindFileThread(LONG _lFFID,TFindFileThread* _Thread)
    {
      lFFID = _lFFID;
      Thread = _Thread;
    }

};


class TFindLogThread:public TThread
{
  protected:
    void __fastcall Execute();
  private:
      LONG _lUserID;
      LONG _lSelectMode;
      DWORD _dwMajorType;
      DWORD _dwMinorType;
      DWORD _dwStartTime;
      DWORD _dwStopTime;
      IDVRUser* _pUser;

  public:
    void __fastcall Exit();
    __fastcall TFindLogThread(bool CreateSuspended,LONG lUserID,LONG lSelectMode, DWORD dwMajorType,DWORD dwMinorType, DWORD dwStartTime, DWORD dwStopTime,IDVRUser* pUser);
};*/

//-----------------|-----------------------------|------------------------------
//-----------------|          IDownloadFile 类   |------------------------------
//-----------------|-----------------------------|------------------------------
class IHikDownloadFile:public IDownloadFile
{
 public:

  virtual DWORD __stdcall  GetDownloadPos(DWORD* dwPos);
  virtual void  __stdcall  Release();
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IPlayBack 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class IHikPlayBack:public IPlayBack
{
   public:
	   IHikPlayBack(){}
	   ~IHikPlayBack(){}
    // LONG lPlayBackID;
    // IHikPlayBack() {lPlayBackID = -1;}
    // virtual DWORD __stdcall PlayBackControl(DWORD dwControlCode,DWORD dwInValue,DWORD *lpOutValue);
     virtual void  __stdcall  Release();
	 virtual void  __stdcall  controlplay(void);//2011-11-14
	 virtual BOOL __stdcall  SeekPlayBack(unsigned int unSetPos,DWORD dwTotalSize) ;	//2012-10-25 改变回放进度 fugl
	 virtual BOOL __stdcall  FastPlayBack() ;										//2012-10-25 快放
	 virtual BOOL __stdcall  SlowPlayBack() ;										//2012-10-25 慢放
	 virtual BOOL __stdcall  PausePlayBack(BOOL bPause) ;				//2012-10-25 暂停或恢复播放
	 virtual BOOL __stdcall  NormalPlayBack() ;									//2012-10-25 恢复正常播放
	 virtual BOOL __stdcall SmartSearchPlayBack(LPIntelligentSearchPlay lpPlayBackParam) ;  //201315
	 virtual BOOL __stdcall GetPlayBackTime(LPNET_TIME lpOsdTime,LPNET_TIME lpStartTime,LPNET_TIME lpEndTime) ; //201315
};

class IHikIVoiceCom:public IVoiceCom
{
  public:
   virtual DWORD SetVolume(WORD wVol);
   virtual void Release();
   virtual LONG TalkSendData(char* lpInBuf,DWORD dwBufSize) ; //2012/21/1向设备发送数据
   virtual BOOL RecordStart() ; //开启本地音频记录 2012、12、7
};
class IHikRealPlay:public IRealPlay
{
 public:
  bool bUsed;
  IHikRealPlay(){bUsed=false;}
  ~IHikRealPlay(){}
  virtual DWORD  __stdcall SetPlayerBufNumber(DWORD dwBufNum);
  virtual DWORD  __stdcall ThrowBFrame(DWORD dwNum);
  virtual DWORD __stdcall SetVideoEffect(DWORD dwBrightValue, DWORD dwContrastValue, DWORD dwSaturationValue, DWORD dwHueValue);
  virtual DWORD __stdcall GetVideoEffect(DWORD *pBrightValue, DWORD *pContrastValue, DWORD *pSaturationValue, DWORD *pHueValue);
  virtual DWORD __stdcall OpenSound();
  virtual DWORD __stdcall CloseSound();
  virtual DWORD __stdcall SetVolume(WORD wVol);
  virtual DWORD __stdcall SaveRealData(char *sFileName);
  virtual DWORD __stdcall StopSaveRealData();
  virtual DWORD __stdcall CapturePicture(char *sFileName);
 // void  CALLBACK RealDataCallBack(LONG lRealHandle, DWORD dwDataType, BYTE *pBuffer,DWORD dwBufSize,DWORD dwUser);
  virtual void  __stdcall Release();

};

class IHikDVRUser:public IDVRUser
{
 public:
   //LONG lUserID;
  //IRealPlay* _RealPlay;
  //IHikRealPlay HikRealPlay;
  bool bUsed;
  int CommandIndex[20][1];
  IHikDVRUser(){CommandIndex[1][0]=106;CommandIndex[2][0]=104;}
  ~IHikDVRUser(){}

  virtual DWORD __stdcall RealPlay(DWORD dwChannel, char cLinkType, char* sMulticastIP, char cFPS, HWND hWnd, IRealPlay* &pRealPlay);//1
  virtual DWORD __stdcall TransPTZControl(LONG lChannel,char *pPTZCodeBuf,DWORD dwBufSize);//2
  virtual DWORD __stdcall PTZControl(DWORD dwChannel, DWORD dwPTZCommand, DWORD dwParam, DWORD dwAction, BOOL bLock, DWORD dwPriority,DWORD step);//3
  virtual DWORD __stdcall GetDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned);//4
  virtual DWORD __stdcall SetDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpInBuffer, DWORD dwInBufferSize);//5
  virtual DWORD __stdcall StartVoiceCom(char cCompress,IVoiceCom* &pVoiceCom); //fgl 把pVoiceCom改为传指针引用//6
  virtual DWORD __stdcall FindFile(LONG lChannel, DWORD dwFileType, DWORD dwStartTime, DWORD dwStopTime,DWORD UserData);//7
  //------------------------------------211
  virtual DWORD __stdcall FindFileClose(LONG lFFHandle);//8
  virtual DWORD __stdcall PlayBack(char *sPlayBackFileName, int chan,DWORD sttime,DWORD edtime,HWND hWnd,IPlayBack* &pPlayBack);//9
  virtual DWORD __stdcall FindLog(LONG lSelectMode, DWORD dwMajorType, DWORD dwMinorType, DWORD StartTime, DWORD StopTime);//10
  //-----------------------------------------211
  virtual DWORD __stdcall FindLogClose(LONG lFLHandle);//11
//  virtual DWORD __stdcall DownloadFile(char *sFileName, char* sSaveFileName, IDownloadFile* &pDownloadFile);  //2012-10-25  fugl//
  virtual DWORD __stdcall DownloadFileByFile(char *sFileName, char* sSaveFileName, IDownloadFile* &pDownloadFile);  //2012-10-25  fugl//12
  virtual DWORD __stdcall DownloadFileByTime(int chan, DWORD sttime, DWORD edtime, char* sSaveFileName, IDownloadFile* &pDownloadFile);  //2012-10-25  fugl//13
  virtual void  __stdcall Release();//14
  virtual void __stdcall  RebootDVR();//15
  virtual void __stdcall SetDVRDateTime(DWORD dwYear,DWORD dwMonth,DWORD dwDay,DWORD dwHour,DWORD dwMin,DWORD dwSec);//16
  virtual DWORD __stdcall StartListen(char *hostIP,DWORD port);		//2011-12-8 glfu//17
  virtual DWORD __stdcall StopListen();				//2011-12-8 glfu//18
  virtual DWORD __stdcall GetDVRConfig_TCP(LONG lChannel,LPVOID lpOutBuffer, DWORD dwCommand, LPDWORD lpBytesReturned);  //glfu 2012-1-10//19
  virtual DWORD __stdcall SetDVRConfig_TCP(LONG lChannel,LPVOID lpInBuffer, DWORD dwCommand, LPDWORD lpBytesReturned);   //glfu 2012-1-10//20
  virtual DWORD __stdcall ShutDownDVR();    //glfu    2012-1-11//21
  virtual DWORD __stdcall QueryDVRState();		//glfu 2012-1-11//22
  virtual DWORD __stdcall SetRecordFile2017(int _chan,int  _type);//2017-08-09 设置某通道录像时间段//23
  virtual DWORD __stdcall SetFileCover(int _chan,NET_TIME_EX *_starttime,NET_TIME_EX *_endtime,int _flag);//2017-08-11设置录像锁定与解锁//24
//  virtual DWORD __stdcall GetAllChanName(LPDEVICEDES1 lpDeviceChan) ;  //glfu 2013/3/27 获取全部通道标签
//  virtual DWORD __stdcall SetAllChanName(LPDEVICEDES1 lpDeviceChan) ;  //glfu 2013/3/27 设置全部通道标签
  virtual DWORD __stdcall QueryDeviceTime();   //2014-1-23 TGX 系统时间查询//25
  //virtual DWORD __stdcall GetAllSerialNumber(int TypeSize,LPVOID sSerNumber);
  //virtual char* __stdcall SetDVRConfig_OSD(DWORD dwCommand, LONG lChannel);//,LPVOID OSD_lpInBuffer, LPVOID lpInBuffer,LPVOID ReturnSize);
  virtual DWORD __stdcall SetDVRConfig_OSD_EX(DWORD dwCommand, LONG lChannel,char* OSD_lpInBuffer, char* lpInBuffer,char* ReturnSize);//26
  virtual DWORD __stdcall GetAllSerialNumber_EX(int TypeSize,LPVOID AdSerNumber);//27
  virtual DWORD __stdcall GetNVRWorkState(LPVOID lpInput);//28
  virtual DWORD __stdcall GetIPCWorkState(LPVOID workstate);//29

 
};
struct  stCommandBuffer
{
	DWORD	CmdLen;					//命令长度，四字节对其
	char	DeviceID;				//数据来源，1－DVR发送；2－MC发送；3－CLIENT发送
	char	SubID;					//板卡类
	char	ProtocolVer;			//固定为0X70
	char	DataType;				//加密和压缩类型，0标识透明传输
    stCommandBuffer()
	{
        DeviceID = '1';
        SubID = '1';
        DataType = '1';
        ProtocolVer = 0x72;
	}
};
struct stSetInfoPACKET
{
  stCommandBuffer CommandHead;
  DWORD CommandID;
  char Commandbuf[1012];
 // char Commandbuf[1326];   //glfu 2013/3/23 增大数据包
};
//2011-01-27
/////////////////////////视频设置相关参数 0//////////////////////
/////////////////////////坐标结构/////////////////////
////////////
// 时间段结构	  //付光亮  把结构体改小  2013/3/23														    
typedef struct 
{
	BOOL				bEnable;				// 当表示录像时间段时，按位表示四个使能，从低位到高位分别表示动检录象、报警录象、普通录象、动检和报警同时发生才录像
	BYTE					iBeginHour;
	BYTE					iBeginMin;
	BYTE					iBeginSec;
	BYTE					iEndHour;
	BYTE					iEndMin;
	BYTE					iEndSec;
} DH_TSECT1, *LPDH_TSECT1;
typedef struct JF_ParamVideo
{
	//DH_COLOR_CFG
	BYTE				byBrightness;			// 亮度；0-100
	BYTE				byContrast;				// 对比度；0-100
	BYTE				byHue;					// 色度；0-100
	BYTE				bySaturation;			// 饱和度；0-100	
	char ChanName[32];		//通道名称
	DWORD rgbaFrontground;//物件的前景；按字节表示，分别为红、绿、蓝和透明度
	DWORD rgbaBackground;//物件的背景；按字节表示，分别为红、绿、蓝和透明度
	DH_RECT      OSD_rcRect;	//OSD位置
	char         SzOSD_Name[32];//osd字符串
	DH_RECT      Time_rcRect;	//时间标签位置
	int          maskcount;
	DH_RECT      mask_rcRect[4];	//mask位置
}JF_ParamVideo;
/////////////////////压缩参数设置 1///////////////////////////////////////////
typedef struct JF_ParamCompress
{
	BYTE    RecordQualityLevel;		//存盘质量，1－6，6最好
	BYTE    byAudioEnable;		//音频使能；1：打开，0：关闭 
	BYTE    byBitRateControl;	//1-VBR，0-CBR
	BYTE    CIFSize;					//CIF大小，0－QCIF，1-CIF，2-HD1，3－D1等
	BYTE    FrameRate;              //帧率，取值范围1-25
	BYTE    byEncodeMode;	//MPEG4 0x00000001 	MPEG4 0x00000002 MPEG2 0x00000004 MPEG1 0x00000008 
							//H263 0x00000010 MJPG 0x00000020 MPEG4 0x00000040 H264 0x00000080 
	DWORD dwImageSizeMask;   //按位掩码表示，首先根据不同的位得到该设备支持的码率种类 //fgl
}JF_ParamCompress;
////-------录像计划2---------
typedef struct JF_DHDEV_RECORD_CFG
{
	DWORD    dwSize;
	//DH_TSECT stSect[DH_N_WEEKS][5];
	DH_TSECT1 stSect[DH_N_WEEKS][6];   //glfu 2013/3/23
	BYTE     byPreRecordLen;
	BYTE     byRedundancyEn;
	BYTE     byRecordType;
	BYTE     byReserved;
}JF_DHDEV_RECORD_CFG;
//-----------动检设置---3-----------------
typedef struct JF_ParamMotionDetect
{
	BYTE MotionEn;     //动态检测报警使能
	int SenseLevel;   //灵敏度
	//int MDRectNum; //运动检测区域个数,最多32个
	int MotionRow;		//动态检测区域的行数 
	int MotionCol; //动态检测区域的列数
	DWORD         byDetected[32];//检测区域，最多32*32块区域
	//DH_TSECT      stSect[7][4];//时间段结构
	DH_TSECT1      stSect[7][6];//时间段结构  glfu 2013/3/23
//	DWORD		  ActionMask;   //当前报警所支持的处理方式，按位掩码表示:0x00000001 - 报警上传0x00000002 - 联动录象  fgl
		//0x00000004 - 云台联动		0x00000008 - 发送邮件		0x00000010 - 本地轮巡		0x00000020 - 本地提示
		//0x00000040 - 报警输出		0x00000080 - Ftp上传		0x00000100 - 蜂鸣		0x00000200 - 语音提示		0x00000400 - 抓图 
//	BYTE         byRelAlarmOut[24];//报警触发的输出通道，报警触发的输出，为1表示触发该输出.  //fgl
//	BYTE         byRecordChannel[24];//联动录象，报警触发的录象通道，为1表示触发该通道.  //fgl
	DWORD        dwRecLatch;//录象持续时间. 
//	BYTE         bySnap[24];//抓图通道  //fgl
}JF_ParamMotionDetect;
//---------------报警设置-4-------
typedef struct JF_ParamALARMIN
{
	BYTE            byAlarmType;//报警器类型： 0 常闭 		1 常开 
	BYTE				AlarmEn;     //报警使能
	//DH_TSECT      stSect[7][4];//时间段结构
	DH_TSECT1      stSect[7][6];//时间段结构  glfu 2013/3/23
	DWORD		  ActionMask;   //当前报警所支持的处理方式，按位掩码表示:0x00000001 - 报警上传0x00000002 - 联动录象
	//0x00000004 - 云台联动		0x00000008 - 发送邮件		0x00000010 - 本地轮巡		0x00000020 - 本地提示
	//0x00000040 - 报警输出		0x00000080 - Ftp上传		0x00000100 - 蜂鸣		0x00000200 - 语音提示		0x00000400 - 抓图 
	BYTE         byRelAlarmOut[32];//报警触发的输出通道，报警触发的输出，为1表示触发该输出.
	BYTE         byRecordChannel[32];//联动录象，报警触发的录象通道，为1表示触发该通道. 
	DWORD        dwRecLatch;//录象持续时间. 
	BYTE         bySnap[32];//抓图通道
}JF_ParamALARMIN;
//-------------视频丢失-6-----------
typedef struct JF_Param_LOST
{
	BYTE           Lost_AlarmEn;//视频丢失报警使能
	DH_TSECT      Lost_stSect[7][4];//视频丢失时间段结构
	DWORD		  Lost_ActionMask;   //视频丢失当前报警所支持的处理方式，按位掩码表示:0x00000001 - 报警上传0x00000002 - 联动录象
	//0x00000004 - 云台联动		0x00000008 - 发送邮件		0x00000010 - 本地轮巡		0x00000020 - 本地提示
	//0x00000040 - 报警输出		0x00000080 - Ftp上传		0x00000100 - 蜂鸣		0x00000200 - 语音提示		0x00000400 - 抓图 
	BYTE         Lost_byRelAlarmOut[32];//视频丢失报警触发的输出通道，报警触发的输出，为1表示触发该输出.
	BYTE         Lost_byRecordChannel[32];//视频丢失联动录象，报警触发的录象通道，为1表示触发该通道. 
	DWORD        Lost_dwRecLatch;//视频丢失录象持续时间. 
	BYTE         Lost_bySnap[32];//视频丢失抓图通道
	
}JF_Param_LOST;

//-------------遮挡报警--7----------
typedef struct JF_Param_BLIND
{
	BYTE            byBlindEnable;//遮挡使能
	BYTE            byBlindLevel;//遮挡灵敏度1-6 
	DH_TSECT      Blind_stSect[7][4];//遮挡时间段结构
	DWORD		  Blind_ActionMask;   //遮挡当前报警所支持的处理方式，按位掩码表示:0x00000001 - 报警上传0x00000002 - 联动录象
	//0x00000004 - 云台联动		0x00000008 - 发送邮件		0x00000010 - 本地轮巡		0x00000020 - 本地提示
	//0x00000040 - 报警输出		0x00000080 - Ftp上传		0x00000100 - 蜂鸣		0x00000200 - 语音提示		0x00000400 - 抓图 
	BYTE         Blind_byRelAlarmOut[32];//遮挡报警触发的输出通道，报警触发的输出，为1表示触发该输出.
	BYTE         Blind_byRecordChannel[32];//遮挡联动录象，报警触发的录象通道，为1表示触发该通道. 
	DWORD        Blind_dwRecLatch;//遮挡录象持续时间. 
	BYTE         Blind_bySnap[32];//遮挡抓图通道
}JF_Param_BLIND;
typedef struct JF_DVR_WorkState
{
	//大华，普通报警，遮挡报警，录像状态，系统支援状态，通道码流，设备连接状态，硬盘状态
	NET_CLIENT_STATE clientState;			//普通报警
	DH_HARDDISK_STATE hardDiskState;		//硬盘状态
	unsigned char shelterAlarm[16];			//遮挡报警
	unsigned char recording[16];					//录像状态
	DWORD dwResource[3];				//查询资源状态
	DWORD dwBitRate[16];				//获取码流
	DWORD dwNetConn;				//大华表示设备连接状态，
	DWORD dwDeviceState;			//海康表示设备的状态：0-正常；1-CPU占用率太高，超过85%；2-硬件错误
}JF_DVR_WorkState;
//extern TList *DVRList;
//extern TList *RealPlayList;
extern std::list<void*> DVRList;
extern int PTZControlChange[64][1];
#endif
