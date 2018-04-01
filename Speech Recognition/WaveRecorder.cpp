#include "WaveRecorder.h"


#pragma region �ⲿ����

void WaveRecorder::set_Callback(CNKDATAUpdateCallback fn)
{
	bCallback = true;
	callback = fn;
}

void WaveRecorder::set_FileName(string Target)
{
	// �����ô浵�򱣴�һ��wav�ļ�
	bSaveFile = true;
	dest_path = Target;
	// ���Դ��ļ����������򴴽���
	errno_t err = fopen_s(&fp, dest_path.c_str(), "wb");
	if (err > 0)
	{
#if _DEBUG
		cout << "�ļ�����ʧ�ܣ�" << err << " ����ļ�����ռ��" << endl;
		system("pause");
#endif // _DEBUG
		bSaveFile = false;
	}
}

void WaveRecorder::Start()
{
	for (int layer = 0; layer < BUFFER_LAYER; layer++)
	{
		// ���û�����
		pwh[layer].lpData = new char[CHUNCK_SIZE];
		pwh[layer].dwBufferLength = CHUNCK_SIZE;
		pwh[layer].dwBytesRecorded = 0;
		pwh[layer].dwUser = layer;
		pwh[layer].dwFlags = 0;
		pwh[layer].dwLoops = 0;
		pwh[layer].lpNext = NULL;
		pwh[layer].reserved = 0;
		// �Ž�������
		waveInPrepareHeader(hwi, &pwh[layer], sizeof(WAVEHDR));
		waveInAddBuffer(hwi, &pwh[layer], sizeof(WAVEHDR));
	}
		// ��ʼ�������ݻ���
		RawData.clear();
		RawData.reserve(10);
		// ����¼����ʼ��Ϣ
		waveInStart(hwi);
}
void WaveRecorder::Stop()
{
	// ֹͣ���
	stop = true;
	// �豸ֹͣ
	waveInStop(hwi);
	waveInReset(hwi);
	// �ͷŻ�����
	for (int layer = 0; layer<BUFFER_LAYER; layer++)
	{
		waveInUnprepareHeader(hwi, &pwh[layer], sizeof(WAVEHDR));
		delete pwh[layer].lpData;
	}
	// ����Header+RawData
	if(bSaveFile)
		WaveFileWrite();
	stop = false;
	dat_ignore = false;
	RawData.clear();
	ChunksCount = 0;
}

void WaveRecorder::Reset()
{
	// ����reset
	// ��̬������ʼ��
	RawData.clear();
	ChunksCount = 0;
	bSaveFile = false;
	bCallback = false;
	callback = NULL;
	stop = false;
	dat_ignore = false;
}

#pragma endregion

#pragma region  �ڲ�����
void WaveRecorder::WaveInitFormat(LPWAVEFORMATEX WaveFormat, WORD Ch, DWORD SampleRate, WORD BitsPerSample)
{
	// �Զ����ò���
	WaveFormat->wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat->nChannels = Ch;
	WaveFormat->nSamplesPerSec = SampleRate;
	WaveFormat->nAvgBytesPerSec = SampleRate * Ch * BitsPerSample / 8;
	WaveFormat->nBlockAlign = Ch * BitsPerSample / 8;
	WaveFormat->wBitsPerSample = BitsPerSample;
	WaveFormat->cbSize = 0;
#if _DEBUG
	cout << "����������" << endl;
	cout << "\t������" << Ch << endl;
	cout << "\tÿ�������" << SampleRate << "Hz" << endl;
	cout << "\tλ��" << BitsPerSample << endl;
#endif // _DEBUG
}
void WaveRecorder::WaveFileWrite()
{
	// �༭��д��Waveͷ��Ϣ
	WavHeader.data_size = CHUNCK_SIZE*RawData.size();
	WavHeader.size_8 = WavHeader.data_size + 32;
	fwrite(&WavHeader, sizeof(WavHeader), 1, fp);
	// ׷��RawData
	fwrite(RawData.data(), CHUNCK_SIZE*RawData.size(), 1, fp);
	// д�����
	fclose(fp);
}
DWORD WaveRecorder::WaveXAPI_Callback(HWAVEIN hwavein, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	// ��Ϣswitch
	switch (uMsg)
	{
	case WIM_OPEN:		// �豸�ɹ��Ѵ�
		ChunksCount = 0;// ���"session"�����ݿ����
		break;

	case WIM_DATA:		// ����������������
		// ֹͣ���Ƶ������WIM_DATA,�Ѿ�������ת�����Բ�����������ݡ�����������������������ظ��ġ�
		if (!dat_ignore)
		{
			// �ѻ��������ݿ�������
			memcpy(ChunkData.data(), ((LPWAVEHDR)dwParam1)->lpData, CHUNCK_SIZE);
			// û��¼��ȥ�ı����Ϊ0xcd,�ĳ�0������ĩβ���ֱ�����ֻ�ڽ���¼��ʱ���У���Ӱ����ӻ���Ч�ʡ�
			if (((LPWAVEHDR)dwParam1)->dwBytesRecorded <CHUNCK_SIZE)
			{
				for (size_t i = ((LPWAVEHDR)dwParam1)->dwBytesRecorded; i < CHUNCK_SIZE; i++)
				{
					ChunkData.at(i) = 0;
				}
			}
			// �����һ֡
			RawData.push_back(ChunkData);
			// ����������˻ص�����
			if (bCallback)
			{
				callback(ChunkData, ChunksCount, stop);
			}
		}
		ChunksCount += 1;
		// �����Ҫֹͣ¼���򲻼�����ӻ���
		if (!stop)
		{
			waveInAddBuffer(hwavein, (LPWAVEHDR)dwParam1, sizeof(WAVEHDR));//��ӵ�������
		}
		else
		{// ��ֹ�ظ���¼����
			dat_ignore = true;
		}

		break;

	case WIM_CLOSE:
		// �����ɹ����
		// cout << "¼���豸�Ѿ��ر�..." << endl;
		break;

	default:
		break;
	}
	return 0;
}

WaveRecorder::WaveRecorder()
{
	// ���û�������豸������
	if (!waveInGetNumDevs())
	{
#if _DEBUG
		cout << "Windowsû���ҵ���Ƶ�����豸" << endl;
#endif //_DEBUG
	}
#if _DEBUG
	
	WAVEINCAPS WaveInCaps;
	MMRESULT mmResult = waveInGetDevCaps(0, &WaveInCaps, sizeof(WAVEINCAPS));
	cout << "Ĭ���豸������" << WaveInCaps.szPname<<")\n\n";
#endif //_DEBUG
	WAVEFORMATEX pwfx;
	WaveInitFormat(&pwfx, CHANNEL_MUM, SAMPLE_RATE, SAMPLE_BITS);
	waveInOpen(&hwi, WAVE_MAPPER, &pwfx, (DWORD_PTR)WaveXAPI_Callback, NULL, CALLBACK_FUNCTION);
}

WaveRecorder::~WaveRecorder()
{	
	// �ر��豸������WIM_CLOSE
	waveInClose(hwi);
}
#pragma endregion
// ��̬������ʼ��
WAVEHDR WaveRecorder::pwh[BUFFER_LAYER] = {};
array <char, CHUNCK_SIZE> WaveRecorder::ChunkData = {};
vector<array<char, CHUNCK_SIZE>> WaveRecorder::RawData = { {} };
UINT WaveRecorder::ChunksCount = 0;
BOOL WaveRecorder::bCallback = false;
CNKDATAUpdateCallback WaveRecorder::callback = NULL;
bool WaveRecorder::stop = false;
bool WaveRecorder::dat_ignore = false;