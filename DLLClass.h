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
struct RealPlayNode{         //�����б�
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
//-----------------|          IDownloadFile ��   |------------------------------
//-----------------|-----------------------------|------------------------------
class IHikDownloadFile:public IDownloadFile
{
 public:

  virtual DWORD __stdcall  GetDownloadPos(DWORD* dwPos);
  virtual void  __stdcall  Release();
};

//-----------------|-----------------------------|------------------------------
//-----------------|          IPlayBack ��       |------------------------------
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
	 virtual BOOL __stdcall  SeekPlayBack(unsigned int unSetPos,DWORD dwTotalSize) ;	//2012-10-25 �ı�طŽ��� fugl
	 virtual BOOL __stdcall  FastPlayBack() ;										//2012-10-25 ���
	 virtual BOOL __stdcall  SlowPlayBack() ;										//2012-10-25 ����
	 virtual BOOL __stdcall  PausePlayBack(BOOL bPause) ;				//2012-10-25 ��ͣ��ָ�����
	 virtual BOOL __stdcall  NormalPlayBack() ;									//2012-10-25 �ָ���������
	 virtual BOOL __stdcall SmartSearchPlayBack(LPIntelligentSearchPlay lpPlayBackParam) ;  //201315
	 virtual BOOL __stdcall GetPlayBackTime(LPNET_TIME lpOsdTime,LPNET_TIME lpStartTime,LPNET_TIME lpEndTime) ; //201315
};

class IHikIVoiceCom:public IVoiceCom
{
  public:
   virtual DWORD SetVolume(WORD wVol);
   virtual void Release();
   virtual LONG TalkSendData(char* lpInBuf,DWORD dwBufSize) ; //2012/21/1���豸��������
   virtual BOOL RecordStart() ; //����������Ƶ��¼ 2012��12��7
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
  virtual DWORD __stdcall StartVoiceCom(char cCompress,IVoiceCom* &pVoiceCom); //fgl ��pVoiceCom��Ϊ��ָ������//6
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
  virtual DWORD __stdcall SetRecordFile2017(int _chan,int  _type);//2017-08-09 ����ĳͨ��¼��ʱ���//23
  virtual DWORD __stdcall SetFileCover(int _chan,NET_TIME_EX *_starttime,NET_TIME_EX *_endtime,int _flag);//2017-08-11����¼�����������//24
//  virtual DWORD __stdcall GetAllChanName(LPDEVICEDES1 lpDeviceChan) ;  //glfu 2013/3/27 ��ȡȫ��ͨ����ǩ
//  virtual DWORD __stdcall SetAllChanName(LPDEVICEDES1 lpDeviceChan) ;  //glfu 2013/3/27 ����ȫ��ͨ����ǩ
  virtual DWORD __stdcall QueryDeviceTime();   //2014-1-23 TGX ϵͳʱ���ѯ//25
  //virtual DWORD __stdcall GetAllSerialNumber(int TypeSize,LPVOID sSerNumber);
  //virtual char* __stdcall SetDVRConfig_OSD(DWORD dwCommand, LONG lChannel);//,LPVOID OSD_lpInBuffer, LPVOID lpInBuffer,LPVOID ReturnSize);
  virtual DWORD __stdcall SetDVRConfig_OSD_EX(DWORD dwCommand, LONG lChannel,char* OSD_lpInBuffer, char* lpInBuffer,char* ReturnSize);//26
  virtual DWORD __stdcall GetAllSerialNumber_EX(int TypeSize,LPVOID AdSerNumber);//27
  virtual DWORD __stdcall GetNVRWorkState(LPVOID lpInput);//28
  virtual DWORD __stdcall GetIPCWorkState(LPVOID workstate);//29

 
};
struct  stCommandBuffer
{
	DWORD	CmdLen;					//����ȣ����ֽڶ���
	char	DeviceID;				//������Դ��1��DVR���ͣ�2��MC���ͣ�3��CLIENT����
	char	SubID;					//�忨��
	char	ProtocolVer;			//�̶�Ϊ0X70
	char	DataType;				//���ܺ�ѹ�����ͣ�0��ʶ͸������
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
 // char Commandbuf[1326];   //glfu 2013/3/23 �������ݰ�
};
//2011-01-27
/////////////////////////��Ƶ������ز��� 0//////////////////////
/////////////////////////����ṹ/////////////////////
////////////
// ʱ��νṹ	  //������  �ѽṹ���С  2013/3/23														    
typedef struct 
{
	BOOL				bEnable;				// ����ʾ¼��ʱ���ʱ����λ��ʾ�ĸ�ʹ�ܣ��ӵ�λ����λ�ֱ��ʾ����¼�󡢱���¼����ͨ¼�󡢶���ͱ���ͬʱ������¼��
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
	BYTE				byBrightness;			// ���ȣ�0-100
	BYTE				byContrast;				// �Աȶȣ�0-100
	BYTE				byHue;					// ɫ�ȣ�0-100
	BYTE				bySaturation;			// ���Ͷȣ�0-100	
	char ChanName[32];		//ͨ������
	DWORD rgbaFrontground;//�����ǰ�������ֽڱ�ʾ���ֱ�Ϊ�졢�̡�����͸����
	DWORD rgbaBackground;//����ı��������ֽڱ�ʾ���ֱ�Ϊ�졢�̡�����͸����
	DH_RECT      OSD_rcRect;	//OSDλ��
	char         SzOSD_Name[32];//osd�ַ���
	DH_RECT      Time_rcRect;	//ʱ���ǩλ��
	int          maskcount;
	DH_RECT      mask_rcRect[4];	//maskλ��
}JF_ParamVideo;
/////////////////////ѹ���������� 1///////////////////////////////////////////
typedef struct JF_ParamCompress
{
	BYTE    RecordQualityLevel;		//����������1��6��6���
	BYTE    byAudioEnable;		//��Ƶʹ�ܣ�1���򿪣�0���ر� 
	BYTE    byBitRateControl;	//1-VBR��0-CBR
	BYTE    CIFSize;					//CIF��С��0��QCIF��1-CIF��2-HD1��3��D1��
	BYTE    FrameRate;              //֡�ʣ�ȡֵ��Χ1-25
	BYTE    byEncodeMode;	//MPEG4 0x00000001 	MPEG4 0x00000002 MPEG2 0x00000004 MPEG1 0x00000008 
							//H263 0x00000010 MJPG 0x00000020 MPEG4 0x00000040 H264 0x00000080 
	DWORD dwImageSizeMask;   //��λ�����ʾ�����ȸ��ݲ�ͬ��λ�õ����豸֧�ֵ��������� //fgl
}JF_ParamCompress;
////-------¼��ƻ�2---------
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
//-----------��������---3-----------------
typedef struct JF_ParamMotionDetect
{
	BYTE MotionEn;     //��̬��ⱨ��ʹ��
	int SenseLevel;   //������
	//int MDRectNum; //�˶�����������,���32��
	int MotionRow;		//��̬������������ 
	int MotionCol; //��̬������������
	DWORD         byDetected[32];//����������32*32������
	//DH_TSECT      stSect[7][4];//ʱ��νṹ
	DH_TSECT1      stSect[7][6];//ʱ��νṹ  glfu 2013/3/23
//	DWORD		  ActionMask;   //��ǰ������֧�ֵĴ���ʽ����λ�����ʾ:0x00000001 - �����ϴ�0x00000002 - ����¼��  fgl
		//0x00000004 - ��̨����		0x00000008 - �����ʼ�		0x00000010 - ������Ѳ		0x00000020 - ������ʾ
		//0x00000040 - �������		0x00000080 - Ftp�ϴ�		0x00000100 - ����		0x00000200 - ������ʾ		0x00000400 - ץͼ 
//	BYTE         byRelAlarmOut[24];//�������������ͨ�������������������Ϊ1��ʾ���������.  //fgl
//	BYTE         byRecordChannel[24];//����¼�󣬱���������¼��ͨ����Ϊ1��ʾ������ͨ��.  //fgl
	DWORD        dwRecLatch;//¼�����ʱ��. 
//	BYTE         bySnap[24];//ץͼͨ��  //fgl
}JF_ParamMotionDetect;
//---------------��������-4-------
typedef struct JF_ParamALARMIN
{
	BYTE            byAlarmType;//���������ͣ� 0 ���� 		1 ���� 
	BYTE				AlarmEn;     //����ʹ��
	//DH_TSECT      stSect[7][4];//ʱ��νṹ
	DH_TSECT1      stSect[7][6];//ʱ��νṹ  glfu 2013/3/23
	DWORD		  ActionMask;   //��ǰ������֧�ֵĴ���ʽ����λ�����ʾ:0x00000001 - �����ϴ�0x00000002 - ����¼��
	//0x00000004 - ��̨����		0x00000008 - �����ʼ�		0x00000010 - ������Ѳ		0x00000020 - ������ʾ
	//0x00000040 - �������		0x00000080 - Ftp�ϴ�		0x00000100 - ����		0x00000200 - ������ʾ		0x00000400 - ץͼ 
	BYTE         byRelAlarmOut[32];//�������������ͨ�������������������Ϊ1��ʾ���������.
	BYTE         byRecordChannel[32];//����¼�󣬱���������¼��ͨ����Ϊ1��ʾ������ͨ��. 
	DWORD        dwRecLatch;//¼�����ʱ��. 
	BYTE         bySnap[32];//ץͼͨ��
}JF_ParamALARMIN;
//-------------��Ƶ��ʧ-6-----------
typedef struct JF_Param_LOST
{
	BYTE           Lost_AlarmEn;//��Ƶ��ʧ����ʹ��
	DH_TSECT      Lost_stSect[7][4];//��Ƶ��ʧʱ��νṹ
	DWORD		  Lost_ActionMask;   //��Ƶ��ʧ��ǰ������֧�ֵĴ���ʽ����λ�����ʾ:0x00000001 - �����ϴ�0x00000002 - ����¼��
	//0x00000004 - ��̨����		0x00000008 - �����ʼ�		0x00000010 - ������Ѳ		0x00000020 - ������ʾ
	//0x00000040 - �������		0x00000080 - Ftp�ϴ�		0x00000100 - ����		0x00000200 - ������ʾ		0x00000400 - ץͼ 
	BYTE         Lost_byRelAlarmOut[32];//��Ƶ��ʧ�������������ͨ�������������������Ϊ1��ʾ���������.
	BYTE         Lost_byRecordChannel[32];//��Ƶ��ʧ����¼�󣬱���������¼��ͨ����Ϊ1��ʾ������ͨ��. 
	DWORD        Lost_dwRecLatch;//��Ƶ��ʧ¼�����ʱ��. 
	BYTE         Lost_bySnap[32];//��Ƶ��ʧץͼͨ��
	
}JF_Param_LOST;

//-------------�ڵ�����--7----------
typedef struct JF_Param_BLIND
{
	BYTE            byBlindEnable;//�ڵ�ʹ��
	BYTE            byBlindLevel;//�ڵ�������1-6 
	DH_TSECT      Blind_stSect[7][4];//�ڵ�ʱ��νṹ
	DWORD		  Blind_ActionMask;   //�ڵ���ǰ������֧�ֵĴ���ʽ����λ�����ʾ:0x00000001 - �����ϴ�0x00000002 - ����¼��
	//0x00000004 - ��̨����		0x00000008 - �����ʼ�		0x00000010 - ������Ѳ		0x00000020 - ������ʾ
	//0x00000040 - �������		0x00000080 - Ftp�ϴ�		0x00000100 - ����		0x00000200 - ������ʾ		0x00000400 - ץͼ 
	BYTE         Blind_byRelAlarmOut[32];//�ڵ��������������ͨ�������������������Ϊ1��ʾ���������.
	BYTE         Blind_byRecordChannel[32];//�ڵ�����¼�󣬱���������¼��ͨ����Ϊ1��ʾ������ͨ��. 
	DWORD        Blind_dwRecLatch;//�ڵ�¼�����ʱ��. 
	BYTE         Blind_bySnap[32];//�ڵ�ץͼͨ��
}JF_Param_BLIND;
typedef struct JF_DVR_WorkState
{
	//�󻪣���ͨ�������ڵ�������¼��״̬��ϵͳ֧Ԯ״̬��ͨ���������豸����״̬��Ӳ��״̬
	NET_CLIENT_STATE clientState;			//��ͨ����
	DH_HARDDISK_STATE hardDiskState;		//Ӳ��״̬
	unsigned char shelterAlarm[16];			//�ڵ�����
	unsigned char recording[16];					//¼��״̬
	DWORD dwResource[3];				//��ѯ��Դ״̬
	DWORD dwBitRate[16];				//��ȡ����
	DWORD dwNetConn;				//�󻪱�ʾ�豸����״̬��
	DWORD dwDeviceState;			//������ʾ�豸��״̬��0-������1-CPUռ����̫�ߣ�����85%��2-Ӳ������
}JF_DVR_WorkState;
//extern TList *DVRList;
//extern TList *RealPlayList;
extern std::list<void*> DVRList;
extern int PTZControlChange[64][1];
#endif
