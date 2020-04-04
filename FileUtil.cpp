#include "FileUtil.h"
#include<fstream>
#include<string>
#include<iostream>
#include <assert.h>
#include <stdio.h>

FILE *stream, *stream2;
int getfilesize(string fname)
{
	FILE *fp;
	errno_t err;
	if ((err = fopen_s(&fp, fname.c_str(), "r")) != 0)
	{
		printf("���ļ�ʧ��");
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	return ftell(fp);
}
string FileUtil::loadAssetStr(string fname)
{
	ifstream infile;
	infile.open(fname.data());
	cout << "fname>" << fname << endl;
	assert(infile.is_open());
	string ss;
	string s;
	while (getline(infile, s))
	{
		ss += s;
		ss += '\n';
	}
	infile.close();
	return ss;
}
SpvData& FileUtil::loadSPV(string fname)//�����ļ����µ�SPIR-V �����ļ�
{
	errno_t err;
	size_t size = (getfilesize(fname));//��ȡSPIR-V �����ļ������ֽ���
	cout << "len:" << size << endl;
	SpvData spvData;//����SpvData �ṹ��ʵ��
	spvData.size = size;//����SPIR-V �������ֽ���
	spvData.data = (uint32_t*)(malloc(size));//������Ӧ�ֽ������ڴ�
	char* buf = (char*)spvData.data;//���ļ��м������ݽ����ڴ�
	char c_file[1000];
	strcpy_s(c_file, fname.c_str());
	FILE * fpSPV;
	err = fopen_s(&fpSPV,c_file, "rb");
	if (err != 0)
	{
		printf("���ļ�%sʧ��\n", fname.c_str());
	}
	fread(buf, size, 1, fpSPV);
	return spvData;//����SpvData �ṹ��ʵ��
}