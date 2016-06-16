#include <stdio.h>
#include <sys/stat.h>

#include <math.h>
#include "mad.h"
#include "mixfft.h"
#include "Mp3FreqAnaly.h"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/stitching/stitcher.hpp>
#include <vector>
#include <iostream>
using namespace std;
using namespace cv;

#define INPUT_BUFFER_SIZE	(10*8192)

int Mp3FreqAnaly(const std::string &FileName ,FreqAnalyResult &result )
{	
	double  data[2048];
	double  outdata[2048];

	for(int i =0;i<2048 ;i++)
	{
		outdata[i] =  data[i] = 0.0f;
	}

	if(FileName.empty())
		return 1;

	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	mad_timer_t			Timer;
	unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
						*OutputPtr=NULL,
						*GuardPtr=NULL;

	int					Status=0,i=0;
	unsigned long		FrameCount=0;
	double  r[10];
	int DDFrame=0;
	for(int i=0;i<2048;i++)
	{
		data[i]=0.0f;
		outdata[i]=0.0f;
	}

	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	mad_timer_reset(&Timer);

	FILE * File;

	File=fopen(FileName.c_str(),"rb");

	if(File==NULL)
	{
		return(1);
	}

	for(int i=0;i<10;i++)
		r[i]=0;

	do
	{
		if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			size_t			ReadSize,
							Remaining;
			unsigned char	*ReadStart;


			if(Stream.next_frame!=NULL)
			{
				Remaining=Stream.bufend-Stream.next_frame;
				memmove(InputBuffer,Stream.next_frame,Remaining);
				ReadStart=InputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			}
			else
				ReadSize=INPUT_BUFFER_SIZE,
					ReadStart=InputBuffer,
					Remaining=0;

			ReadSize=fread(ReadStart,1,ReadSize,File);
			if(ReadSize<=0)
			{
				if(ferror(File))
				{

					Status=1;
				}
				if(feof(File))
					NULL;
				break;
			}

			if(feof(File))
			{
				GuardPtr=ReadStart+ReadSize;
				memset(GuardPtr,0,MAD_BUFFER_GUARD);
				ReadSize+=MAD_BUFFER_GUARD;
			}

			mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
			Stream.error=(mad_error)0;
		}

		if(mad_frame_decode(&Frame,&Stream))
		{
			if(MAD_RECOVERABLE(Stream.error))
			{
				if(Stream.error!=MAD_ERROR_LOSTSYNC ||
						Stream.this_frame!=GuardPtr)
				{

					fflush(stderr);
				}
				continue;
			}
			else
				if(Stream.error==MAD_ERROR_BUFLEN)
					continue;
				else
				{

					Status=1;
					break;
				}
		}

		FrameCount++;
		mad_timer_add(&Timer,Frame.header.duration);

		mad_synth_frame(&Synth,&Frame);

		for(int i =0;i<2048 ;i++)
		{
			data[i]=0.0f;
		}

		signed int 	Sample;
		char b1,b2;
		float Samples;
		if(Synth.pcm.length <1024)
			Samples = 512;
		else
			Samples  = 1024;

		for(int k=0;k<Samples;k++)
		{
			if( k<Synth.pcm.length)
			{
				Sample =Synth.pcm.samples[0][k];
			}
			else
			{
				Sample =0;
			}
			data[k]=double(Sample);
		}

		fft(Samples,data,&data[1024],outdata,&outdata[1024]);
		double now;
		double max=65536;
		double av=0,total=0;

		for(int j=0;j<Samples/2.0;j++)
		{ 
			double n1 =(outdata[j]);
			double n2 =(outdata[j+1024]);
			if(n2>0)
				now =sqrt(n2)/2;
			else
				now=0;
			if(now <max)
				max=now;

			if(now<= 0)
				data[j]=0.0f;
			else
				data[j]=now/65536.0;

			total = total+data[j];
		}

		av=total/(Samples/2.0);

		for(int j=Samples/2-1;j>=0;j--)
		{

			if(data[j] < av)
			{
				continue;
			}
			else
			{
				int s=((j)/(Samples/2.0))*22050.0;
				if(s >= 20000-500)
					r[8]++;
				if(s >= 19000-500)
					r[7]++;
				if(s >= 18000-500)
					r[6]++;
				if(s >= 17000-500)
					r[5]++;
				if(s >= 16000-500)
					r[4]++;
				if(s >= 15000-500)
					r[3]++;
				if(s >= 14000-500)
					r[2]++;
				if(s >= 13000-500)
					r[1]++;
				if(s >= 12000-500)
					r[0]++;
				break;
			}
		}
	}while(1);


	fclose(File);

	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);

	result.Frames =FrameCount;
	for(int i=0;i<9;i++)
	{
		result.Result[i].Freq=i*1000+12000;
		result.Result[i].Value = r[i];
	}

	return(Status);
}

struct RGB
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
};
struct PictureRow
{
	RGB value[512];			
};

void DarwFreqPicture(const std::string &fileName ,const std::string &path ,int beginFrame,int nembers)
{
	if(fileName.empty())
		return;
	PictureRow p;
	vector<PictureRow> bitmapData;

	int RowsDraw=0;
	struct mad_stream	Stream;
	struct mad_frame	Frame;
	struct mad_synth	Synth;
	mad_timer_t			Timer;
	unsigned char		InputBuffer[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD],
						*OutputPtr=NULL,
						*GuardPtr=NULL;

	int					Status=0,i=0;
	unsigned long		FrameCount=0;
	double  data[2048];

	double  outdata[2048];

	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);
	mad_timer_reset(&Timer);

	FILE * File;

	File=fopen(fileName.c_str(),"rb");

	if(File==NULL)
	{
		return;
	}

	do
	{
		if(Stream.buffer==NULL || Stream.error==MAD_ERROR_BUFLEN)
		{
			size_t			ReadSize, Remaining;
			unsigned char	*ReadStart;

			if(Stream.next_frame!=NULL)
			{
				Remaining=Stream.bufend-Stream.next_frame;
				memmove(InputBuffer,Stream.next_frame,Remaining);
				ReadStart=InputBuffer+Remaining;
				ReadSize=INPUT_BUFFER_SIZE-Remaining;
			}
			else
				ReadSize=INPUT_BUFFER_SIZE,
					ReadStart=InputBuffer,
					Remaining=0;

			ReadSize=fread(ReadStart,1,ReadSize,File);
			if(ReadSize<=0)
			{
				if(ferror(File))
				{

					Status=1;
				}
				if(feof(File))
					NULL;
				break;
			}

			if(feof(File))
			{
				GuardPtr=ReadStart+ReadSize;
				memset(GuardPtr,0,MAD_BUFFER_GUARD);
				ReadSize+=MAD_BUFFER_GUARD;
			}

			mad_stream_buffer(&Stream,InputBuffer,ReadSize+Remaining);
			Stream.error=(mad_error)0;
		}

		if(mad_frame_decode(&Frame,&Stream))
		{
			if(MAD_RECOVERABLE(Stream.error))
			{
				if(Stream.error!=MAD_ERROR_LOSTSYNC ||
						Stream.this_frame!=GuardPtr)
				{

					fflush(stderr);
				}
				continue;
			}
			else
				if(Stream.error==MAD_ERROR_BUFLEN)
					continue;
				else
				{

					Status=1;
					break;
				}
		}

		FrameCount++;
		mad_timer_add(&Timer,Frame.header.duration);

		mad_synth_frame(&Synth,&Frame);

		for(int i =0;i<2048 ;i++)
		{
			data[i]=0.0f;
		}

		signed int 	Sample;
		char b1,b2;
		float Samples;
		if(Synth.pcm.length <1024)
			Samples = 512;
		else
			Samples  = 1024;

		for(int k=0;k<Samples;k++)
		{
			if( k<Synth.pcm.length)
			{
				Sample =Synth.pcm.samples[0][k];
			}
			else
			{
				Sample =0;
			}
			data[k]=double(Sample);
		}


		fft(Samples,data,&data[1024],outdata,&outdata[1024]);
		double now;
		double max=65536;
		for(int j=0;j<Samples;j++)
		{ 
			double n1 =(outdata[j]);
			double n2 =(outdata[j+1024]);
			if(n2>0)
				now =sqrt(n2)/2;
			else
				now=0;
			if(now <max)
				max=now;


			if(now<= 0)
				data[j]=0.0f;
			else
				data[j]=now/65536.0;
		}

		if(FrameCount >=beginFrame &&  (RowsDraw < nembers || nembers == 0) )
		{
			double av=0;

			for(int i=0;i<Samples/2;i++)
			{
				av+=data[i];
			}
			av =av /512;

			double av10= av*10,av6=av*6,av4=av*4,av2=av*2,av1_5=av*1.5,av0_5 =av*0.5;

			float r[512],g[512],b[512];
			r[0]=255;g[0]=255;b[0]=255;

			for(int i=1;i<Samples/2;i++)
			{					
				if(data[i]>av10 && data[i]>0.02)
				{
					r[i]=250;g[i]=200+55*((data[i]-av10)/(av10));b[i]=0;
				}else	if(data[i]>av4 && data[i]>0.02 )
				{
					if(data[i]>av10)
						data[i]=av10;

					r[i]=230;g[i]=55+100*((data[i]-av4)/(av10-av4));b[i]=0;
				}
				else if(data[i]>av && data[i]>0.02)
				{
					r[i]=200+55*((data[i]-av)/(av4-av));g[i]=55*((data[i]-av)/(av4-av));b[i]=110;
				}
				else if(data[i]>av0_5 && data[i]>0.02)
				{
					r[i]=30;g[i]=0;b[i]=100+125*((data[i]-av0_5)/0.5);
				}
				else
				{
					r[i]=30;g[i]=0;b[i]=100;
				}
			}
			PictureRow row;
			row.value[0].b = 0;
			row.value[0].g = 0;
			row.value[0].r = 0;

			for(int i=1;i<Samples/2-1;i++)
			{
				row.value[i].r=(r[i]+r[i-1]/2+r[i+1]/2)/2;
				row.value[i].g=(g[i]+g[i-1]/2+g[i+1]/2)/2;
				row.value[i].b=(b[i]+b[i-1]/2+b[i+1]/2)/2;
			}

			row.value[511].r=r[511];
			row.value[511].g=g[511];
			row.value[511].b=b[511];
			bitmapData.insert(bitmapData.end(),row);
			RowsDraw++;
		}
		if(RowsDraw >=nembers && nembers != 0 &&  RowsDraw>102400)
		{
			break;		
		}

	}while(1);


	fclose(File);
	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);

	if(RowsDraw <= 0)
		return;

	Mat FreqImage =  Mat(512, RowsDraw, CV_8UC3, Scalar(255,255,255));

	for(int i=0 ;i<RowsDraw;i++)
	{
		for(int j=510;j >= 0;--j)
		{
			RGB* p = (RGB*)FreqImage.ptr<uchar>(j);
			p[i].r = bitmapData[i].value[511 - j].r;
			p[i].g = bitmapData[i].value[511 - j].g;
			p[i].b = bitmapData[i].value[511 - j].b;
		}
	}
	Size dsize(1024 - 60, 512);
	resize(FreqImage, FreqImage, dsize);

	Mat TextImage = Mat(512, 60, CV_8UC3, Scalar(100,0,30)); 
	int BeginDraw=0,EndDraw=60; 
	Scalar brush(100,0,30), brush2(255,255,0), brush3(0,0,255);
	Scalar pNormal(255,255,0), pRed(0,0,255);
	line( TextImage, Point(EndDraw -1, 0), Point(EndDraw - 1, 511), brush2, 2);

	int n=512/11.0;
	char buf[128];
	for(int i =1;i<12;i++)
	{
		if(i != 8)
		{
			line(TextImage, Point(BeginDraw+4,511-i*n), Point(EndDraw-3,511-i*n), pNormal, 1);
		}
		else
		{
			line(TextImage, Point(BeginDraw+4,511-i*n), Point(EndDraw-3,511-i*n), pRed, 1);
		}
		line(TextImage, Point(EndDraw -6,511-i*n+ n/2), Point(EndDraw-3,511-i*n+ n/2), pNormal, 1);

		sprintf(buf, "%-2dk", i*2);
		if(i != 8)
		{
			putText( TextImage, buf, Point(BeginDraw+5,511-i*n+30), FONT_HERSHEY_SIMPLEX, 0.8, brush2, 1, 2);
		}
		else
		{
			putText( TextImage, buf, Point(BeginDraw+5,511-i*n+30), FONT_HERSHEY_SIMPLEX, 0.8, brush3, 1, 2);
		}
	}

	IplImage pTextImg = (IplImage)TextImage;
	IplImage pFreqImg = (IplImage)FreqImage;

	CvSize dstSize;
	dstSize.width = 1024;
	dstSize.height = 512;
	IplImage* pDstImg = cvCreateImage(dstSize, IPL_DEPTH_8U , pFreqImg.nChannels);
	cvZero(pDstImg);
	cvSetImageROI(pDstImg, cvRect(0, 0, pTextImg.width, pTextImg.height));
	cvCopy(&pTextImg, pDstImg);
	cvResetImageROI(pDstImg);
	cvSetImageROI(pDstImg, cvRect(60, 0, pFreqImg.width, pFreqImg.height));
	cvCopy(&pFreqImg, pDstImg, 0);
	cvResetImageROI(pDstImg);
	cvSaveImage(path.c_str(), pDstImg);
	cvReleaseImage(&pDstImg);
}

bool checkMp3FreqIsValid(const std::string &fileName, const std::string &freqPic)
{
	FreqAnalyResult FAResult;
	int MaxValue=0;
	int MaxFreq=0;
	bool isValid = true;

	if (Mp3FreqAnaly(fileName, FAResult) == 0)
	{
		MaxValue=FAResult.Result[9].Value;
		MaxFreq=FAResult.Result[9].Freq;

		int value;
		for(int i=0;i<9;i++)
		{
			value = abs(FAResult.Result[i].Value-FAResult.Result[i+1].Value);
			if( value> MaxValue)
			{
				MaxValue=value;
				MaxFreq =FAResult.Result[i].Freq;
			}
		}
		if(MaxFreq <= 19000)
		{	
			DarwFreqPicture(fileName, freqPic);
			isValid = false;
		}
	}
	return isValid;
}

