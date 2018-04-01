#pragma once
#pragma region ����
// ��Ҫͷ�ļ�
#include <thread>
#include <iostream>
#include "WaveRecorder.h"
#include "windows.h"
// MSCͷ�ļ�
#include "msp_cmn.h"		//	Mobile Speech Platform ����ͷ�ļ�
#include "msp_types.h"		//	Mobile Speech Platform ö������ͷ�ļ�
#include<msp_errors.h>
//#include "msp_errors.h"	//	Mobile Speech Platform �����붨��
#include "qisr.h"			//	Ѷ������ʶ��ͷ�ļ�
#include "qtts.h"			//	Ѷ�������ϳ�ͷ�ļ�
using namespace std;

#pragma endregion

/* �ص����� */
typedef void(__stdcall *CharacterHandler)(string fragment, string rsltAdded);

class SpeechRecognizer
{
public:
	SpeechRecognizer(CharacterHandler callback_CH);				// ����,��½
	~SpeechRecognizer();										// �������ǳ�
	static int ServerOperationAlert;
	static int SegmentationStatus;
	static int RecognitionStatus;
	static const char* SessionID;
	// todo : ʶ��������Ƭ��handler
	static bool InService;							// ����ʶ��ģ������״̬
	bool Start();									// ��ʼһ���µ�����ʶ��
	void Stop();									// �ֶ�ֹͣ�˴�����ʶ��
private:
	static void CALLBACK onRec(array <char, CHUNCK_SIZE> ChunkData, UINT ChunksCount, bool bIsLast);
	static DWORD WINAPI receiver(LPVOID lpParameter);
	static bool stop;
public:
	int ret;										// ������|����뷵��ֵ
	static string Result;
private:
	WaveRecorder Recorder;
	static CharacterHandler callback_CH;
	string session_begin_params = "sub = iat, domain = iat, language = zh_ch, accent = mandarin, aue = aue=speex-wb;7, sample_rate = 16000, result_type = plain, result_encoding = gb2312";
};
