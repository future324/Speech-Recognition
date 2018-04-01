#include "SpeechRecognizer.h"
const char* SpeechRecognizer::SessionID = NULL;
int SpeechRecognizer::SegmentationStatus = 0;
int SpeechRecognizer::ServerOperationAlert = 0;
int SpeechRecognizer::RecognitionStatus = 0;
bool SpeechRecognizer::stop = false;
bool SpeechRecognizer::InService = false;
string SpeechRecognizer::Result = {};
CharacterHandler SpeechRecognizer::callback_CH = NULL;
const char* session_begin_params = "sub = iat, domain = iat, language = zh_cn, accent = mandarin, sample_rate = 16000, result_type = plain, result_encoding = gb2312";
SpeechRecognizer::SpeechRecognizer(CharacterHandler ch)
{	// ��½ 
	ServerOperationAlert = MSPLogin(NULL, NULL, "appid = 59c06558");
	if (MSP_SUCCESS != ServerOperationAlert)
	{
		cout << "��½ʧ��";
	}
	if (ServerOperationAlert != MSP_SUCCESS) exit(0);
	// ��ʼ��
	Recorder.set_Callback(onRec);
	callback_CH = ch;
}
SpeechRecognizer::~SpeechRecognizer()
{
	MSPLogout(); //�˳���¼
}

bool SpeechRecognizer::Start()
{
	// ��ʼ��
	while (InService)
	{
		stop = true;
		Sleep(200);
	}
	Result.clear();
	stop = false;
	SessionID = NULL;
	SegmentationStatus = MSP_EP_LOOKING_FOR_SPEECH;
	ServerOperationAlert = MSP_SUCCESS;
	RecognitionStatus = MSP_REC_STATUS_SUCCESS;
	SessionID = QISRSessionBegin(NULL, session_begin_params.c_str(), &ServerOperationAlert);
	//cout << SessionID << endl;
	Recorder.Start();
	// ���������߳�
	return
		CloseHandle(CreateThread(NULL, 0, receiver, (LPVOID)this, 0, NULL)) ?
		true : false;
}

void SpeechRecognizer::Stop()
{
	stop = true;
}

void CALLBACK SpeechRecognizer::onRec(array <char, CHUNCK_SIZE> ChunkData, UINT ChunksCount, bool bIsLast)
{
	QISRAudioWrite(SessionID, ChunkData.data(), ChunkData.size(),
		bIsLast ? // ��Ƶ����
		MSP_AUDIO_SAMPLE_LAST : ChunksCount > 0 ?
		MSP_AUDIO_SAMPLE_CONTINUE : MSP_AUDIO_SAMPLE_FIRST,
		&SegmentationStatus, &RecognitionStatus);// �ϴ���Ƶ
}

DWORD WINAPI SpeechRecognizer::receiver(LPVOID lpParameter)
{
	SpeechRecognizer *psr = (SpeechRecognizer*)lpParameter;
	psr->InService = true;
	mciSendString(L"close beep", NULL, 0, 0);
	mciSendString(L"open sr_start.wav alias beep", NULL, 0, 0);
	mciSendString(L"play beep", NULL, 0, 0);
	// ��ѯ������ʶ����   �߳�
	while (!stop && RecognitionStatus < MSP_REC_STATUS_COMPLETE && SegmentationStatus <= MSP_EP_AFTER_SPEECH)
	{
		const char* fragment = QISRGetResult(SessionID, &RecognitionStatus, 0, &ServerOperationAlert);
		if (!ServerOperationAlert && fragment)// û�з����������н������
		{
			Result += fragment;// ��ý��
							   // callback����
			callback_CH(fragment, Result);
		}
		else Sleep(1);
	}
	psr->Recorder.Stop();
	//mciSendString(L"close beep", NULL, 0, 0);
	//mciSendString(L"open st_stop.wav alias beep", NULL, 0, 0);
	//mciSendString(L"play beep", NULL, 0, 0);
	int ret = QISRSessionEnd(SessionID, "normal end");
	if (MSP_SUCCESS != ret)
	{
		printf("QISRSessionEnd failed, error code is: %d", ret);
	}

	SessionID = NULL;
	stop = false;
	psr->InService = false;
	return 0;
}
