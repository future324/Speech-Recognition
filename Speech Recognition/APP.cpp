#include "WaveRecorder.h"
#include "conio.h"
#include"SpeechRecognizer.h"
#include"Vocalizer.h"
#include"TlRobot.hpp"
/*����һ���ص�����ʾ��*/
void CALLBACK CharHandler(std::string fragment, std::string rsltAdded);
void VocalFeedback(const char*  sentence);
string Recognizersentence;//ʶ�𵽵����
string txt_feedback;//�ش�����
SpeechVolizer speech;
SpeechRecognizer Recognizer(CharHandler);
int main()
{
	while (true)
	{
		Recognizer.Stop();
		if (!Recognizer.Start())
		{
			cout << "start error";
		}
		Sleep(2000);
	}	
	system("pause");
}

/*����һ���ص�����ʾ��*/

void CALLBACK CharHandler(std::string fragment, std::string resultAdd)
{
	
	const char* result = resultAdd.c_str();
	if (!resultAdd.empty())
	{
		
			//VocalFeedback(GetResult(fragment).c_str());
		cout << fragment << endl;
	}
}

void VocalFeedback(const char* sentence)
{
	Recognizer.Stop();
	txt_feedback = sentence;
	speech.Vocalizer(sentence,"xiaoyan");
}