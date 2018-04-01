#include"Vocalizer.h"
#include"SDKwavefile.h"
SpeechVolizer::SpeechVolizer()
{

}

SpeechVolizer::~SpeechVolizer()
{
	pSource->Stop();                                                // �ر���Դ
	pSource->FlushSourceBuffers();                                  // ���������(��ѡ)
}
int SpeechVolizer::text_to_speech(const char* src_text, const char* des_path, const char* params)
{
	
	if (NULL == src_text || NULL == des_path)
	{
		std::cout << "params is error!"<<endl;
		return ret;
	}
	fopen_s(&fp, des_path, "wb");
	if (NULL == fp)
	{
		std::cout << "open" << des_path << "error" << endl;
		return ret;
	}
	/* ��ʼ�ϳ� */
	sessionID = QTTSSessionBegin(params, &ret);
	if (MSP_SUCCESS != ret)
	{
		fclose(fp);  return ret;
	}
	ret = QTTSTextPut(sessionID, src_text, (unsigned int)strlen(src_text), NULL);
	if (MSP_SUCCESS != ret)
	{
		QTTSSessionEnd(sessionID, "TextPutError");
		fclose(fp);
		return ret;
	}
	fwrite(&wav_hdr, sizeof(wav_hdr), 1, fp); //���wav��Ƶͷ��ʹ�ò�����Ϊ16000
	while (1)
	{
		/* ��ȡ�ϳ���Ƶ */
		const void* data = QTTSAudioGet(sessionID, &audio_len, &synth_status, &ret);
		if (MSP_SUCCESS != ret)
			break;
		if (NULL != data)
		{
			fwrite(data, audio_len,1, fp);
			wav_hdr.data_size += audio_len; //����data_size��С
		}
		if (MSP_TTS_FLAG_DATA_END == synth_status)
			break;
		Sleep(150); //��ֹƵ��ռ��CPU
	}

	if (MSP_SUCCESS != ret)
	{
		QTTSSessionEnd(sessionID, "AudioGetError");
		fclose(fp);
		return ret;
	}
	/* ����wav�ļ�ͷ���ݵĴ�С */
	wav_hdr.size_8 += wav_hdr.data_size + (sizeof(wav_hdr) - 8);

	/* ��������������д���ļ�ͷ��,��Ƶ�ļ�Ϊwav��ʽ */
	fseek(fp, 4, 0);
	fwrite(&wav_hdr.size_8, sizeof(wav_hdr.size_8), 1, fp); //д��size_8��ֵ
	fseek(fp, 40, 0); //���ļ�ָ��ƫ�Ƶ��洢data_sizeֵ��λ��
	fwrite(&wav_hdr.data_size, sizeof(wav_hdr.data_size), 1, fp); //д��data_size��ֵ
	fclose(fp);
	fp = NULL;
	/* �ϳ���� */
	ret = QTTSSessionEnd(sessionID, "Normal");
	if (MSP_SUCCESS != ret)
	{
		cout << "enderror" << endl;
	}

	
	return ret;
}
int  SpeechVolizer::filetospeech()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(XAudio2Create(&XAudioEngine))) return FALSE;
	if (FAILED(XAudioEngine->CreateMasteringVoice(&pmaster)))return FALSE;

	CWaveFile waveFile;
	HRESULT hr = waveFile.Open(L"temp.wav", NULL, WAVEFILE_READ);//�����ļ�  
	if (FAILED(hr))
		return 0;

	WAVEFORMATEX *waveFormat = waveFile.GetFormat();//��ȡ�ļ���ʽ  

	DWORD size = waveFile.GetSize();//��ȡ�ļ��Ĵ�С  
	BYTE *pData = new BYTE[size];//�����ڴ�ռ䣬���ڱ�������  
	hr = waveFile.Read(pData, size, &size);//��ȡ�ļ�����  
	if (FAILED(hr))
		return 0;
	if (FAILED(XAudioEngine->CreateSourceVoice(&pSource, waveFormat)))
	{
		std::cout << "error" << endl; return FALSE;
	}

	pSource->Start();
	sBuffer.pAudioData = pData;           // �ϳ���Ƶ
	sBuffer.AudioBytes = size;                      // һ����������Ƶ������
	if (FAILED(pSource->SubmitSourceBuffer(&sBuffer)))
		return 0;     // װ����Ƶ
					  // �ȴ�������ϻ��ߴ��
	for (pSource->GetState(&pState); pState.BuffersQueued; pSource->GetState(&pState))
		Sleep(10);
	waveFile.Close();//�ر��ļ�  
	delete[]pData;
	pData = NULL;
}
void SpeechVolizer::Vocalizer(const char* src_text, string Speachname)
{
	int         ret = MSP_SUCCESS;
	const char* login_params = "appid = 59c06558, work_dir = .";//��¼����,appid��msc���,��������Ķ�
																/*
																* rdn:           �ϳ���Ƶ���ַ�����ʽ
																* volume:        �ϳ���Ƶ������
																* pitch:         �ϳ���Ƶ������
																* speed:         �ϳ���Ƶ��Ӧ������
																* voice_name:    �ϳɷ�����
																* sample_rate:   �ϳ���Ƶ������
																* text_encoding: �ϳ��ı������ʽ
																*
																*/
	string begin_param_string = "voice_name=" + Speachname + "," + "text_encoding = gb2312, sample_rate = 16000, speed = 50, volume = 50, pitch = 50, rdn = 1";
	const char* session_begin_params = begin_param_string.c_str();
	const char* filename = "temp.wav"; //�ϳɵ������ļ�����
	const char* text = src_text; //�ϳ��ı�
																							 /* �û���¼ */
	//ret = MSPLogin(NULL, NULL, login_params); //��һ���������û������ڶ������������룬�����������ǵ�¼�������û������������http://www.xfyun.cnע���ȡ
	//if (MSP_SUCCESS != ret)
	//{
	//	cout << "��½ʧ��";
	//}

	/* �ı��ϳ� */
	std:: cout << "��ʼ�ϳ�...." << endl;
	ret = text_to_speech(src_text, filename, session_begin_params);
	filetospeech();
	if (MSP_SUCCESS != ret)
	{
		//printf("text_to_speech failed, error code: %d.\n", ret);
		cout << "�ϳ�ʧ��";
	}
	//system("pause");
	//MSPLogout(); //�˳���¼
	remove("temp.wav");
}

