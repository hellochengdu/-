#ifndef GlobeClassH
#define GlobeClassH
//----------------------


#define IPC_LOCAL_NOERROR 				0  //没有错误
#define IPC_LOCAL_PASSWORD_ERROR 			1  //用户名密码错误
#define IPC_LOCAL_NETWORK_FAIL_CONNECT		2 //向服务器发送失败
#define IPC_LOCAL_DEFAULT_ERROR             3//其它错误

//2013-9-22 之前老版
/* 
typedef struct{
    DWORD  VideoNum; 				//视频通道个数,最多64个
    char   VideoLable[16][32];	//视频通道标签，对该通道位置的文字描述
    DWORD  AlarmNum; 				//报警通道个数，最多64个
    char   AlarmLable[16][32];	//报警通道标签，对该通道位置的文字描述
}DEVICEDES;
*/ 
//2013-9-22 接入DH NVR
typedef struct{
	DWORD  VideoNum; 				//视频通道个数,最多64个
	char   VideoLable[128][64];	//视频通道标签，对该通道位置的文字描述
	DWORD  AlarmNum; 				//报警通道个数，最多64个
	char   AlarmLable[128][64];	//报警通道标签，对该通道位置的文字描述
}DEVICEDES,*LPDEVICEDES;
//2017-08-09 NVR上面的IPC信息
typedef struct NVR_IPC{
	char    ID[256];
	char    ip[128][24];
	DWORD   port[128];
	char    user[128][32];
	char    passwd[128][32];
	char    SerNO[128][49];//2018-05-07 存储上的IPC序列号
	int     devtype;//2-巡查  1- 平安  0- 普通
	char    MSerNo[48];//设备序列号
}NVR_IPC;

typedef struct  
{
	DWORD WorkState;//工作状态 1正常  0离线
	BOOL SaveState;//存储开关   1开启   0未开启
	BOOL HardDisk;//磁盘预警状态   1无预警   0 有预警
	DWORD dwVolume;// 磁盘空间总大小
	DWORD dwFreeSpace;//磁盘空闲空间大小
}NVRState;
typedef struct 
{
	DWORD WorkState;//工作状态  1正常 0离线
	int   VideoLost;// 视频质量诊断：1正常 -1检测异常 2视频抖动 3视频丢失 4 视频遮挡 5不清晰 6亮度异常 99其他异常
	int   MainVideoWidth;//主码流视频宽
	int   MainVideoHeight;//主码流视频高
	int   DeputyStreamWidth;//副码流视频宽
	int   DeputyStreamHeight;//副码流视频高
}IPCState;

//用于保存通道标签使用 2013、3、27  glfu
//typedef struct{
//	DWORD  VideoNum; 				//视频通道个数,最多64个
//	char   VideoLable[64][64];	//视频通道标签，对该通道位置的文字描述
//	DWORD  AlarmNum; 				//报警通道个数，最多64个
//	char   AlarmLable[64][64];	//报警通道标签，对该通道位置的文字描述
//}DEVICEDES1,*LPDEVICEDES1;

//用于保存序列号使用2018-05-04
//typedef struct{
//	BYTE   DVRSerialNumber[48]; //dvr序列号
//	BYTE   IpcSerialNumber[128][48]; //IPC序列号
//}SerNumber;
//2011-09-05
typedef struct{
//	char FileName[128];			//文件名 2012-10-25 fugl
	char  StartTime[128];		//文件开始时间
	char  StopTime[128];		//文件开始时间
	//DWORD TimeLen;		//文件可以回放多少秒
	//DWORD FileLen;		//文件字节数
	DWORD  FileSize;    //文件大小
	}FILEINFO;

typedef struct {
DWORD StartTime;		//日志记录的时间
DWORD Type;			//日志的类型
char    LogContent[128]; 	//日志的内容
}LOGINFO;


//--------------------------------------------------
struct Control
{
 DWORD dwChannel;
 DWORD dwPriority;
 Control(DWORD _dwChannel,DWORD _dwPriority)
  {
   dwChannel = _dwChannel;
   dwPriority = _dwPriority;
  }
};

//-----------------|-----------------------------|------------------------------
//-----------------|       PTZControlList 类     |------------------------------
//-----------------|-----------------------------|------------------------------
/*class PTZControlList
{
 private:
    TList *ControlList;
 public:
  PTZControlList()
  {
    ControlList = new TList;
  }
  ~PTZControlList(){}
  bool __stdcall AddPTZControl(DWORD dwChannel,DWORD dwPriority); //增加控制
  bool __stdcall DelPTZControl(DWORD dwChannel,DWORD dwPriority); //删除控制
  DWORD __stdcall FindControl(DWORD dwChannel);

};*/

//-----------------|-----------------------------|------------------------------
//-----------------|          IDownloadFile 类   |------------------------------
//-----------------|-----------------------------|------------------------------
class IDownloadFile
{
 public:
  DWORD dwUserData;
  LONG   lFileHandle;  //2012-10-25 fugl 查找文件句柄
  typedef DWORD (__stdcall *ReceivePlayPos)(IDownloadFile* pDownloadFile,DWORD dwTotalSize, DWORD dwDownLoadSize, DWORD dwUser);  //fgl 2013/2/1
  ReceivePlayPos lpReceivePlayPos;
  virtual DWORD __stdcall  GetDownloadPos(DWORD* dwPos) = 0;
  virtual void  __stdcall  Release() = 0;
  IDownloadFile()
  {
	  lpReceivePlayPos = NULL;
  }
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IPlayBack 类       |------------------------------
//-----------------|-----------------------------|------------------------------
//2011-09-07 
class IPlayBack
{
public:

	LONG lPlayBackID;
	typedef DWORD (__stdcall *HsitroyDATACALLBACK)(IPlayBack* pIPlayBack,LPVOID pBuffer,DWORD dwBufSize);
	HsitroyDATACALLBACK lpHsitroyDATACALLBACK;
	LONG lPlayID;		
	HANDLE controlplay_handle;//2011-11-14
	IPlayBack(){
		lpHsitroyDATACALLBACK = NULL;
	}
	~IPlayBack(){}
	virtual void __stdcall  Release() = 0;
	virtual void __stdcall  controlplay() = 0;											//2011-11-14
	virtual BOOL __stdcall  SeekPlayBack(unsigned int unSetPos,DWORD dwTotalSize) = 0;	//2012-10-25 改变回放进度 fugl
	virtual BOOL __stdcall  FastPlayBack() = 0;										//2012-10-25 快放
	virtual BOOL __stdcall  SlowPlayBack() = 0;										//2012-10-25 慢放
	virtual BOOL __stdcall  PausePlayBack(BOOL bPause) = 0;				//2012-10-25 暂停或恢复播放
	virtual BOOL __stdcall  NormalPlayBack() = 0;									//2012-10-25 恢复正常播放
	virtual BOOL __stdcall SmartSearchPlayBack(LPIntelligentSearchPlay lpPlayBackParam) = 0;  //201315
	virtual BOOL __stdcall GetPlayBackTime(LPNET_TIME lpOsdTime,LPNET_TIME lpStartTime,LPNET_TIME lpEndTime) = 0; //201315
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IVoiceCOm 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class IVoiceCom
{
  public:
   LONG lVoiceID;
   virtual DWORD SetVolume(WORD wVol) = 0;
   virtual void Release() =0;
   virtual LONG TalkSendData(char* lpInBuf,DWORD dwBufSize) = 0; //2012/21/1向设备发送数据
   virtual BOOL RecordStart() = 0; //开启本地音频记录 2012、12、7
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IRealPlay 类       |------------------------------
//-----------------|-----------------------------|------------------------------
class IRealPlay
{
  public:
   typedef DWORD (__stdcall *REALDATACALLBACK)(IRealPlay* pRealPlay, char cPacketType, char cDataType, LPVOID pBuffer,DWORD dwBufSize);
   REALDATACALLBACK lpREALDATACALLBACK;
   LONG lPlayID;
   IRealPlay(){
     lpREALDATACALLBACK = NULL;
    }
   ~IRealPlay(){}

   virtual DWORD __stdcall SetPlayerBufNumber(DWORD dwBufNum)=0;
   virtual DWORD __stdcall ThrowBFrame(DWORD dwNum)=0;
   virtual DWORD __stdcall SetVideoEffect(DWORD dwBrightValue, DWORD dwContrastValue, DWORD dwSaturationValue, DWORD dwHueValue) = 0;
   virtual DWORD __stdcall GetVideoEffect(DWORD *pBrightValue, DWORD *pContrastValue, DWORD *pSaturationValue, DWORD *pHueValue) = 0;
   virtual DWORD __stdcall OpenSound() = 0;
   virtual DWORD __stdcall CloseSound() = 0;
   virtual DWORD __stdcall SetVolume(WORD wVol) = 0;
   virtual DWORD __stdcall SaveRealData(char *sFileName) = 0;
   virtual DWORD __stdcall StopSaveRealData() = 0;
   virtual DWORD __stdcall CapturePicture(char *sFileName) = 0;
   virtual void  __stdcall Release() = 0;

};

//-----------------|-----------------------------|------------------------------
//-----------------|          IDVRUser 类        |------------------------------
//-----------------|-----------------------------|------------------------------

class IDVRUser
{
 public:
   typedef DWORD (__stdcall *PTZCALLBACK)(IDVRUser* pUser,DWORD dwChannel,BOOL bSuccess,char* sUserName);
   typedef void (__stdcall *FINDFILECALLBACK)( IDVRUser* pUser,DWORD dwChannel, BOOL bLimited,DWORD dwFileNum,char* lpFileInfo,DWORD UserData);
   typedef bool (__stdcall *FMESSCALLBACK)(IDVRUser* pUser,LONG lCommand,char *pBuf,DWORD dwBufLen);
   typedef void(__stdcall *VoiceDataCallBack)(LONG lVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, BYTE byAudioFlag, DWORD dwUser);
   LONG lUserID;
   LONG lSerialID;
   //2013-10-23
   DWORD dwTypeID;//设备类型 ID 2xx
   DWORD ldwIP;//ip2011-06-23 忘记增加了...
   LONG lListenHandle;			//2011-11-30  glfu
   LONG AlarmMotion;   //glfu  2011-12-19
   LONG AlarmViLost;	//glfu 2011-12-19
   LONG AlarmShelter;	//glfu 2011-12-19
   LONG MotionType;		//glfu 2011-12-19
   LONG ViLostType;			//glfu 2011-12-19;
   LONG ShelterType;		//glfu 2011-12-19
   DWORD preMotion;		//2012-9-29 移动侦测   检测与上次是否一样
   DWORD preViLost;			//视频丢失
   DWORD preShelter;			//视频遮挡
   DWORD preAlarm;			//红外报警
   DWORD preDiskFull;		//硬盘满报警
   DWORD preHardDisk;		//硬盘故障
   DWORD preMoTimer;		//为了控制发移动侦测快慢  20121116
   PTZCALLBACK lpPTZCALLBACK;      //云台控制回调指针
   FINDFILECALLBACK lpFindFileCallBack;   //查找文件指针
//   PTZControlList clPTZControlList;      //云台锁定列表
   FMESSCALLBACK lpFMessCallBack;          //设备消息回调
   VoiceDataCallBack lpVoiceDataCallBack;
   IDVRUser(){
	 lpPTZCALLBACK = NULL;

	 lpFindFileCallBack = NULL;
	 lpFMessCallBack = NULL;
	 lpVoiceDataCallBack = NULL;
     lSerialID = -1;
//	 preMoTimer = 0;  //为了控制发移动侦测快慢  20121116
   }
   ~IDVRUser(){}
   virtual DWORD __stdcall RealPlay(DWORD dwChannel, char cLinkType, char* sMulticastIP, char cFPS, HWND hWnd, IRealPlay* &pRealPlay)=0;
   virtual DWORD __stdcall TransPTZControl(LONG lChannel,char *pPTZCodeBuf,DWORD dwBufSize) = 0;
   virtual DWORD __stdcall PTZControl(DWORD dwChannel, DWORD dwPTZCommand, DWORD dwParam, DWORD dwAction, BOOL bLock, DWORD dwPriority,DWORD step)=0;
   virtual DWORD __stdcall GetDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)=0;
   virtual DWORD __stdcall SetDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpInBuffer, DWORD dwInBufferSize)=0;
   virtual DWORD __stdcall StartVoiceCom(char cCompress,IVoiceCom* &pVoiceCom) = 0;  //fgl 把pVoiceCom改为传指针引用
   virtual DWORD __stdcall FindFile(LONG lChannel, DWORD dwFileType, DWORD dwStartTime, DWORD dwStopTime,DWORD UserData) = 0;
   //------------------------------------211
   virtual DWORD __stdcall FindFileClose(LONG lFFHandle) = 0;
   virtual DWORD __stdcall PlayBack(char *sPlayBackFileName, int chan,DWORD sttime,DWORD edtime,HWND hWnd,IPlayBack* &pPlayBack) = 0;
   virtual DWORD __stdcall FindLog(LONG lSelectMode, DWORD dwMajorType, DWORD dwMinorType, DWORD StartTime, DWORD StopTime) = 0;
   //-----------------------------------------211
   virtual DWORD __stdcall FindLogClose(LONG lFLHandle) = 0;
 //  virtual DWORD __stdcall DownloadFile(char *sFileName, char* sSaveFileName, IDownloadFile* &pDownloadFile) = 0;
   virtual DWORD __stdcall DownloadFileByFile(char *sFileName, char* sSaveFileName, IDownloadFile* &pDownloadFile) = 0;  //2012-10-25  fugl
   virtual DWORD __stdcall DownloadFileByTime(int chan, DWORD sttime, DWORD edtime, char* sSaveFileName, IDownloadFile* &pDownloadFile) = 0;  //2012-10-25  fugl
   virtual void __stdcall  Release() = 0;
   virtual void __stdcall  RebootDVR() = 0;
   virtual void __stdcall SetDVRDateTime(DWORD dwYear,DWORD dwMonth,DWORD dwDay,DWORD dwHour,DWORD dwMin,DWORD dwSec) = 0;
   virtual DWORD __stdcall StartListen(char *hostIP,DWORD port) = 0;		//2011-11-30 glfu
   virtual DWORD __stdcall StopListen() = 0;				//2011-11-30 glfu
   virtual DWORD __stdcall GetDVRConfig_TCP(LONG lChannel,LPVOID lpOutBuffer, DWORD dwCommand, LPDWORD lpBytesReturned) = 0;  //glfu 2012-1-10
   virtual DWORD __stdcall SetDVRConfig_TCP(LONG lChannel,LPVOID lpInBuffer, DWORD dwCommand, LPDWORD lpBytesReturned) = 0;   //glfu 2012-1-10 
   virtual DWORD __stdcall ShutDownDVR() = 0;    //glfu    2012-1-11
   virtual DWORD __stdcall QueryDVRState() = 0;		//glfu 2012-1-11
//   virtual DWORD __stdcall GetAllChanName(LPDEVICEDES1 lpDeviceChan) = 0;  //glfu 2013/3/27 获取全部通道标签
//   virtual DWORD __stdcall SetAllChanName(LPDEVICEDES1 lpDeviceChan) = 0;  //glfu 2013/3/27 设置全部通道标签
   virtual DWORD __stdcall QueryDeviceTime()=0;   //2014-1-23 TGX 系统时间查询
   virtual DWORD __stdcall SetRecordFile2017(int _chan,int  _type) =0;//2017-08-09 设置某通道录像时间段
   virtual DWORD __stdcall SetFileCover(int _chan,NET_TIME_EX *_starttime,NET_TIME_EX *_endtime,int _flag) =0;//2017-08-11设置录像锁定与解锁   
  // virtual char* __stdcall SetDVRConfig_OSD(DWORD dwCommand, LONG lChannel)=0;//,LPVOID OSD_lpInBuffer, LPVOID lpInBuffer,LPVOID ReturnSize)=0;
   virtual DWORD __stdcall SetDVRConfig_OSD_EX(DWORD dwCommand, LONG lChannel,char* OSD_lpInBuffer, char* lpInBuffer,char* ReturnSize)=0;
   virtual DWORD __stdcall GetAllSerialNumber_EX(int TypeSize,LPVOID AdSerNumber)=0;
   virtual DWORD __stdcall GetNVRWorkState(LPVOID lpInput)=0;
   virtual DWORD __stdcall GetIPCWorkState(LPVOID workstate)=0;

   
};




 extern "C"{
	 //初始化
//	DWORD  __stdcall  IPC_HIK_Local_Init();
	//登录
//	DWORD __stdcall   IPC_HIK_DVR_Login(char* dwDVRIP,WORD wDVRPort,char* sUserName,char* sPassword,int IPCType,DWORD dwUserData,DVRLOGINCALLBACK lpDVRLoginCallBack);
	//释放
/*	DWORD  __stdcall __export IPC_HIK_Local_CleanUp();
	//得到最后的错误
	DWORD  __stdcall __export IPC_HIK_Local_GetLastError();
	//启动本地监听
	DWORD __stdcall __export IPC_HIK_Local_StartListen(char *sLocalIP,WORD wLocalPort);
	//设置连接时间
	DWORD  __stdcall __export IPC_HIK_Local_SetConnectTime(DWORD dwWaitTime,DWORD dwTryTimes);
	//设置显示模式
	DWORD  __stdcall __export IPC_HIK_Local_SetShowMode(DWORD dwShowType,COLORREF colorKey);   //设置显示模式*/

}


#endif
