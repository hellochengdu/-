#ifndef GlobeClassH
#define GlobeClassH
//----------------------


#define IPC_LOCAL_NOERROR 				0  //û�д���
#define IPC_LOCAL_PASSWORD_ERROR 			1  //�û����������
#define IPC_LOCAL_NETWORK_FAIL_CONNECT		2 //�����������ʧ��
#define IPC_LOCAL_DEFAULT_ERROR             3//��������

//2013-9-22 ֮ǰ�ϰ�
/* 
typedef struct{
    DWORD  VideoNum; 				//��Ƶͨ������,���64��
    char   VideoLable[16][32];	//��Ƶͨ����ǩ���Ը�ͨ��λ�õ���������
    DWORD  AlarmNum; 				//����ͨ�����������64��
    char   AlarmLable[16][32];	//����ͨ����ǩ���Ը�ͨ��λ�õ���������
}DEVICEDES;
*/ 
//2013-9-22 ����DH NVR
typedef struct{
	DWORD  VideoNum; 				//��Ƶͨ������,���64��
	char   VideoLable[128][64];	//��Ƶͨ����ǩ���Ը�ͨ��λ�õ���������
	DWORD  AlarmNum; 				//����ͨ�����������64��
	char   AlarmLable[128][64];	//����ͨ����ǩ���Ը�ͨ��λ�õ���������
}DEVICEDES,*LPDEVICEDES;
//2017-08-09 NVR�����IPC��Ϣ
typedef struct NVR_IPC{
	char    ID[256];
	char    ip[128][24];
	DWORD   port[128];
	char    user[128][32];
	char    passwd[128][32];
	char    SerNO[128][49];//2018-05-07 �洢�ϵ�IPC���к�
	int     devtype;//2-Ѳ��  1- ƽ��  0- ��ͨ
	char    MSerNo[48];//�豸���к�
}NVR_IPC;

typedef struct  
{
	DWORD WorkState;//����״̬ 1����  0����
	BOOL SaveState;//�洢����   1����   0δ����
	BOOL HardDisk;//����Ԥ��״̬   1��Ԥ��   0 ��Ԥ��
	DWORD dwVolume;// ���̿ռ��ܴ�С
	DWORD dwFreeSpace;//���̿��пռ��С
}NVRState;
typedef struct 
{
	DWORD WorkState;//����״̬  1���� 0����
	int   VideoLost;// ��Ƶ������ϣ�1���� -1����쳣 2��Ƶ���� 3��Ƶ��ʧ 4 ��Ƶ�ڵ� 5������ 6�����쳣 99�����쳣
	int   MainVideoWidth;//��������Ƶ��
	int   MainVideoHeight;//��������Ƶ��
	int   DeputyStreamWidth;//��������Ƶ��
	int   DeputyStreamHeight;//��������Ƶ��
}IPCState;

//���ڱ���ͨ����ǩʹ�� 2013��3��27  glfu
//typedef struct{
//	DWORD  VideoNum; 				//��Ƶͨ������,���64��
//	char   VideoLable[64][64];	//��Ƶͨ����ǩ���Ը�ͨ��λ�õ���������
//	DWORD  AlarmNum; 				//����ͨ�����������64��
//	char   AlarmLable[64][64];	//����ͨ����ǩ���Ը�ͨ��λ�õ���������
//}DEVICEDES1,*LPDEVICEDES1;

//���ڱ������к�ʹ��2018-05-04
//typedef struct{
//	BYTE   DVRSerialNumber[48]; //dvr���к�
//	BYTE   IpcSerialNumber[128][48]; //IPC���к�
//}SerNumber;
//2011-09-05
typedef struct{
//	char FileName[128];			//�ļ��� 2012-10-25 fugl
	char  StartTime[128];		//�ļ���ʼʱ��
	char  StopTime[128];		//�ļ���ʼʱ��
	//DWORD TimeLen;		//�ļ����ԻطŶ�����
	//DWORD FileLen;		//�ļ��ֽ���
	DWORD  FileSize;    //�ļ���С
	}FILEINFO;

typedef struct {
DWORD StartTime;		//��־��¼��ʱ��
DWORD Type;			//��־������
char    LogContent[128]; 	//��־������
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
//-----------------|       PTZControlList ��     |------------------------------
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
  bool __stdcall AddPTZControl(DWORD dwChannel,DWORD dwPriority); //���ӿ���
  bool __stdcall DelPTZControl(DWORD dwChannel,DWORD dwPriority); //ɾ������
  DWORD __stdcall FindControl(DWORD dwChannel);

};*/

//-----------------|-----------------------------|------------------------------
//-----------------|          IDownloadFile ��   |------------------------------
//-----------------|-----------------------------|------------------------------
class IDownloadFile
{
 public:
  DWORD dwUserData;
  LONG   lFileHandle;  //2012-10-25 fugl �����ļ����
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
//-----------------|          IPlayBack ��       |------------------------------
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
	virtual BOOL __stdcall  SeekPlayBack(unsigned int unSetPos,DWORD dwTotalSize) = 0;	//2012-10-25 �ı�طŽ��� fugl
	virtual BOOL __stdcall  FastPlayBack() = 0;										//2012-10-25 ���
	virtual BOOL __stdcall  SlowPlayBack() = 0;										//2012-10-25 ����
	virtual BOOL __stdcall  PausePlayBack(BOOL bPause) = 0;				//2012-10-25 ��ͣ��ָ�����
	virtual BOOL __stdcall  NormalPlayBack() = 0;									//2012-10-25 �ָ���������
	virtual BOOL __stdcall SmartSearchPlayBack(LPIntelligentSearchPlay lpPlayBackParam) = 0;  //201315
	virtual BOOL __stdcall GetPlayBackTime(LPNET_TIME lpOsdTime,LPNET_TIME lpStartTime,LPNET_TIME lpEndTime) = 0; //201315
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IVoiceCOm ��       |------------------------------
//-----------------|-----------------------------|------------------------------
class IVoiceCom
{
  public:
   LONG lVoiceID;
   virtual DWORD SetVolume(WORD wVol) = 0;
   virtual void Release() =0;
   virtual LONG TalkSendData(char* lpInBuf,DWORD dwBufSize) = 0; //2012/21/1���豸��������
   virtual BOOL RecordStart() = 0; //����������Ƶ��¼ 2012��12��7
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IRealPlay ��       |------------------------------
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
//-----------------|          IDVRUser ��        |------------------------------
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
   DWORD dwTypeID;//�豸���� ID 2xx
   DWORD ldwIP;//ip2011-06-23 ����������...
   LONG lListenHandle;			//2011-11-30  glfu
   LONG AlarmMotion;   //glfu  2011-12-19
   LONG AlarmViLost;	//glfu 2011-12-19
   LONG AlarmShelter;	//glfu 2011-12-19
   LONG MotionType;		//glfu 2011-12-19
   LONG ViLostType;			//glfu 2011-12-19;
   LONG ShelterType;		//glfu 2011-12-19
   DWORD preMotion;		//2012-9-29 �ƶ����   ������ϴ��Ƿ�һ��
   DWORD preViLost;			//��Ƶ��ʧ
   DWORD preShelter;			//��Ƶ�ڵ�
   DWORD preAlarm;			//���ⱨ��
   DWORD preDiskFull;		//Ӳ��������
   DWORD preHardDisk;		//Ӳ�̹���
   DWORD preMoTimer;		//Ϊ�˿��Ʒ��ƶ�������  20121116
   PTZCALLBACK lpPTZCALLBACK;      //��̨���ƻص�ָ��
   FINDFILECALLBACK lpFindFileCallBack;   //�����ļ�ָ��
//   PTZControlList clPTZControlList;      //��̨�����б�
   FMESSCALLBACK lpFMessCallBack;          //�豸��Ϣ�ص�
   VoiceDataCallBack lpVoiceDataCallBack;
   IDVRUser(){
	 lpPTZCALLBACK = NULL;

	 lpFindFileCallBack = NULL;
	 lpFMessCallBack = NULL;
	 lpVoiceDataCallBack = NULL;
     lSerialID = -1;
//	 preMoTimer = 0;  //Ϊ�˿��Ʒ��ƶ�������  20121116
   }
   ~IDVRUser(){}
   virtual DWORD __stdcall RealPlay(DWORD dwChannel, char cLinkType, char* sMulticastIP, char cFPS, HWND hWnd, IRealPlay* &pRealPlay)=0;
   virtual DWORD __stdcall TransPTZControl(LONG lChannel,char *pPTZCodeBuf,DWORD dwBufSize) = 0;
   virtual DWORD __stdcall PTZControl(DWORD dwChannel, DWORD dwPTZCommand, DWORD dwParam, DWORD dwAction, BOOL bLock, DWORD dwPriority,DWORD step)=0;
   virtual DWORD __stdcall GetDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpOutBuffer, DWORD dwOutBufferSize, LPDWORD lpBytesReturned)=0;
   virtual DWORD __stdcall SetDVRConfig(DWORD dwCommand, LONG lChannel, LPVOID lpInBuffer, DWORD dwInBufferSize)=0;
   virtual DWORD __stdcall StartVoiceCom(char cCompress,IVoiceCom* &pVoiceCom) = 0;  //fgl ��pVoiceCom��Ϊ��ָ������
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
//   virtual DWORD __stdcall GetAllChanName(LPDEVICEDES1 lpDeviceChan) = 0;  //glfu 2013/3/27 ��ȡȫ��ͨ����ǩ
//   virtual DWORD __stdcall SetAllChanName(LPDEVICEDES1 lpDeviceChan) = 0;  //glfu 2013/3/27 ����ȫ��ͨ����ǩ
   virtual DWORD __stdcall QueryDeviceTime()=0;   //2014-1-23 TGX ϵͳʱ���ѯ
   virtual DWORD __stdcall SetRecordFile2017(int _chan,int  _type) =0;//2017-08-09 ����ĳͨ��¼��ʱ���
   virtual DWORD __stdcall SetFileCover(int _chan,NET_TIME_EX *_starttime,NET_TIME_EX *_endtime,int _flag) =0;//2017-08-11����¼�����������   
  // virtual char* __stdcall SetDVRConfig_OSD(DWORD dwCommand, LONG lChannel)=0;//,LPVOID OSD_lpInBuffer, LPVOID lpInBuffer,LPVOID ReturnSize)=0;
   virtual DWORD __stdcall SetDVRConfig_OSD_EX(DWORD dwCommand, LONG lChannel,char* OSD_lpInBuffer, char* lpInBuffer,char* ReturnSize)=0;
   virtual DWORD __stdcall GetAllSerialNumber_EX(int TypeSize,LPVOID AdSerNumber)=0;
   virtual DWORD __stdcall GetNVRWorkState(LPVOID lpInput)=0;
   virtual DWORD __stdcall GetIPCWorkState(LPVOID workstate)=0;

   
};




 extern "C"{
	 //��ʼ��
//	DWORD  __stdcall  IPC_HIK_Local_Init();
	//��¼
//	DWORD __stdcall   IPC_HIK_DVR_Login(char* dwDVRIP,WORD wDVRPort,char* sUserName,char* sPassword,int IPCType,DWORD dwUserData,DVRLOGINCALLBACK lpDVRLoginCallBack);
	//�ͷ�
/*	DWORD  __stdcall __export IPC_HIK_Local_CleanUp();
	//�õ����Ĵ���
	DWORD  __stdcall __export IPC_HIK_Local_GetLastError();
	//�������ؼ���
	DWORD __stdcall __export IPC_HIK_Local_StartListen(char *sLocalIP,WORD wLocalPort);
	//��������ʱ��
	DWORD  __stdcall __export IPC_HIK_Local_SetConnectTime(DWORD dwWaitTime,DWORD dwTryTimes);
	//������ʾģʽ
	DWORD  __stdcall __export IPC_HIK_Local_SetShowMode(DWORD dwShowType,COLORREF colorKey);   //������ʾģʽ*/

}


#endif
