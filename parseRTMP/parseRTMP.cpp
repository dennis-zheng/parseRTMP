// parseRTMP.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <assert.h>
using namespace std;

struct RTMP_HEADER
{
	unsigned char frameType;
	unsigned char packetType;
	unsigned char time[3];
};

struct RTMP_HEADER_1
{
	unsigned char configurationVersion;
	unsigned char AVCProfileIndication;
	unsigned char profile_compatibility;
	unsigned char AVCLevelIndication;
	unsigned char lengthSizeMinusOne;
};

struct SPS_PPS_HEADER
{
	//char num;
	unsigned short length;
};

struct FRAME_HEADER
{
	unsigned int length;
};

struct NAL_HEADER
{
	char nal[4];
	NAL_HEADER()
	{
		nal[0] = 0x00;
		nal[1] = 0x00;
		nal[2] = 0x00;
		nal[3] = 0x01;
	}
};

unsigned int intBS(unsigned int n)
{
	return ((n << 24) & 0xFF000000) | ((n << 8) & 0x00FF0000) | ((n >> 8) & 0x0000FF00) | ((n >> 24) & 0x000000FF);
}

unsigned short shortBS(unsigned short n)
{
	return ((n << 8) & 0xFF00) | ((n >> 8) & 0x00FF);
}

void SPS_PPS_process(unsigned char* buf, int len)
{
	NAL_HEADER nal;
	int index = sizeof(RTMP_HEADER_1);
	if (index >= len)
	{
		return;
	}
	RTMP_HEADER_1* h1 = (RTMP_HEADER_1*)buf;
	buf = buf + sizeof(RTMP_HEADER_1);

	while (true)
	{
		index += 1;
		if (index >= len)
		{
			break;
		}
		unsigned char* num = buf;
		buf = buf + 1;
		index += sizeof(SPS_PPS_HEADER);
		if (index >= len)
		{
			break;
		}
		SPS_PPS_HEADER* sps_pps_h = (SPS_PPS_HEADER*)buf;
		buf = buf + sizeof(SPS_PPS_HEADER);
		int nT = shortBS(sps_pps_h->length);
		if (nT + index > len)
		{
			assert(false);
		}
		if (1)
		{
			FILE* fpH = NULL;
			fopen_s(&fpH, "rtmp/rtmp.h264", "ab+");
			fwrite(&nal, 1, sizeof(NAL_HEADER), fpH);
			fwrite(buf, 1, nT, fpH);
			fclose(fpH);
		}
		index += nT;
		buf = buf + nT;
	}
}

void frame_process(unsigned char* buf, int len)
{
	NAL_HEADER nal;
	int index = 0;
	while (true)
	{
		index += sizeof(FRAME_HEADER);
		if (index >= len)
		{
			return;
		}
		FRAME_HEADER* h1 = (FRAME_HEADER*)buf;
		buf = buf + sizeof(FRAME_HEADER);
		int nT = intBS(h1->length);
		if (nT + index > len)
		{
			assert(false);
		}
		if (1)
		{
			FILE* fpH = NULL;
			fopen_s(&fpH, "rtmp/rtmp.h264", "ab+");
			fwrite(&nal, 1, sizeof(NAL_HEADER), fpH);
			fwrite(buf, 1, nT, fpH);
			fclose(fpH);
		}
		index += nT;
		buf = buf + nT;
	}
}

void parseRTMP(unsigned char* buf, int len)
{
	int index = sizeof(RTMP_HEADER);
	if (index > len)
	{
		return;
	}
	RTMP_HEADER* rtmpHeader = (RTMP_HEADER*)buf;
	if (0 == rtmpHeader->packetType)
	{
		SPS_PPS_process(buf + index, len - index);
	}
	else if (1 == rtmpHeader->packetType)
	{
		frame_process(buf + index, len - index);
	}
	else
	{
		assert(false);
	}
}

int main()
{
	cout << "RTMP_HEADER: " << sizeof(RTMP_HEADER) 
		<< ", RTMP_HEADER_1: " << sizeof(RTMP_HEADER_1) 
		<< ", SPS_PPS_HEADER: " << sizeof(SPS_PPS_HEADER) 
		<< ", FRAME_HEADER: " << sizeof(FRAME_HEADER) << endl;

	unsigned char* buf = new unsigned char[1024 * 1024];
	char path[256] = { 0 };
	for (int i = 0; i <= 8; i++)
	{
		sprintf_s(path, 256, "rtmp/%04d.rtmp", i);
		FILE *fp = NULL;
		fopen_s(&fp, path,"rb");
		int rd = fread(buf, 1, 1024 * 1024, fp);
		int len = rd;
		while (0 < rd)
		{
			rd = fread(buf + len, 1, 1024 * 1024, fp);
			len += rd;
		}
		parseRTMP(buf, len);
		fclose(fp);
	}
	delete[]buf;
    return 0;
}

