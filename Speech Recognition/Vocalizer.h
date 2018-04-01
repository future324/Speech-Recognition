#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <errno.h>
#include<iostream>
#include "qtts.h"
#include "msp_cmn.h"
#include "msp_errors.h"
#include <xaudio2.h>
#include<vector>
#pragma comment(lib,"xaudio2.lib") 

using namespace std;
class SpeechVolizer
{
public:
	SpeechVolizer();
	~SpeechVolizer();
	int SpeechVolizer::text_to_speech(const char* src_text, const char* des_path, const char* params);
	void Vocalizer(const char* src_text, string name);
	int  SpeechVolizer::filetospeech();
private:
	
	typedef struct _wave_pcm_hdr
	{
		char            riff[4];                // = "RIFF"
		int				size_8;                 // = FileSize - 8
		char            wave[4];                // = "WAVE"
		char            fmt[4];                 // = "fmt "
		int				fmt_size;				// = ��һ���ṹ��Ĵ�С : 16

		short int       format_tag;             // = PCM : 1
		short int       channels;               // = ͨ���� : 1
		int				samples_per_sec;        // = ������ : 8000 | 6000 | 11025 | 16000
		int				avg_bytes_per_sec;      // = ÿ���ֽ��� : samples_per_sec * bits_per_sample / 8
		short int       block_align;            // = ÿ�������ֽ��� : wBitsPerSample / 8
		short int       bits_per_sample;        // = ����������: 8 | 16

		char            data[4];                // = "data";
		int				data_size;              // = �����ݳ��� : FileSize - 44 
	} wave_pcm_hdr;
	wave_pcm_hdr default_wav_hdr =
	{
		{ 'R', 'I', 'F', 'F' },
		0,
		{ 'W', 'A', 'V', 'E' },
		{ 'f', 'm', 't', ' ' },
		16,
		1,
		1,
		16000,
		32000,
		2,
		16,
		{ 'd', 'a', 't', 'a' },
		0
	};



	int          ret = -1;
	FILE*        fp = NULL;
	const char*  sessionID = NULL;
	unsigned int audio_len = 0;
	wave_pcm_hdr wav_hdr = default_wav_hdr;
	int          synth_status = MSP_TTS_FLAG_STILL_HAVE_DATA;
	WAVEFORMATEX wfx;

	IXAudio2 * XAudioEngine = NULL;                                 // IXAduio2��Ƶ����
	IXAudio2MasteringVoice * pmaster = NULL;                        // ����������
	IXAudio2SourceVoice * pSource = NULL;                           // ��Դ
	XAUDIO2_BUFFER sBuffer = {};                                    // ��Դ������
	XAUDIO2_VOICE_STATE pState = {};                                // ������״̬
};

