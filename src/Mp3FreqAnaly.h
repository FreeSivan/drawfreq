#pragma once

#include <stdio.h>
#include <vector>
using namespace std;
#include <math.h>
#include <string>

struct FreqAnalyResult
{
	struct FreqValue
	{
		int Freq;
		int Value;
		FreqValue(int f=0,int v=0)
		{
			Freq	=	f;
			Value	=	v;
		}
	};
	int Frames;
	FreqValue Result[10];
};

int Mp3FreqAnaly(const std::string &FileName ,FreqAnalyResult &result );

void DarwFreqPicture(const std::string &fileName ,const std::string &path ,int beginFrame = 0,int nembers = 0);

bool checkMp3FreqIsValid(const std::string &fileName, const std::string &freqPic);
