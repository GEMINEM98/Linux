#pragma once   // ͷ�ļ�ֻ����һ�ε�˵��

#include<stdio.h>
#include<iostream>
#include<vector>
#include<fstream>  // �ļ�������ͷ�ļ�
#include<sstream>
#include<stdint.h>
#include<boost/filesystem.hpp>

#ifdef _WIN32
// windowsͷ�ļ�
#include <WinSock2.h>
#include<WS2tcpip.h>  // Windows Socket Э��
#include<iphlpapi.h>  // ��ȡ������Ϣ�ӿڵ�ͷ�ļ���PIP_ADAPTER_INFO�ṹ��GetAdaptersInfo�ӿ�...
#pragma comment(lib,"Iphlpapi.lib")  // ��ȡ������Ϣ�ӿڵĿ��ļ�����
#pragma comment(lib,"ws2_32.lib")  // Windows�µ�socket�⣺inet_pton
#else
// linuxͷ�ļ�
#endif



class StringUtil  //class Dig
{
public:
	static int Str2Dig(const std::string &num)
	{
		std::stringstream tmp;
		tmp << num;  // ��numд��tmp��
		int res;
		tmp >> res;  // ��tmpд�뵽res�У�����������������أ����ݺ����������Ͳ�ͬ�����ݽ���ת����
		return res;  // ����res
	}
};



class FileUtil
{
public:
	static int64_t GetFileSize(const std::string& name)  // ͳһ��װһ���ӿ�
	{
		return boost::filesystem::file_size(name);
	}

	static bool Write(const std::string& name, const std::string& body, int offset = 0) // ������������Ĭ�ϲ���
	{

		//********************************************************* C++��׼������ļ�����

		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "ab+");//  ���ļ�д�������� �����Ƶ�׷��д
		if (fp == NULL)
		{
			std::cerr << "���ļ�ʧ��\n";
			return false;
		}
		fseek(fp, offset, SEEK_SET); //��תƫ��λ��
		size_t ret = fwrite(body.c_str(), 1, body.size(), fp);
		if (ret != body.size())
		{
			std::cerr << "���ļ�д������ʧ��\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;

	}

	// ָ�������ʾ����һ������Ͳ���
	// const& ��ʾ����һ�������Ͳ���
	// & ��ʾ����һ����������Ͳ���
	static bool Read(const std::string& name, std::string* body)
	{
		//********************************************************* C++��׼������ļ�����

		int64_t filesize = boost::filesystem::file_size(name);  // ͨ���ļ�����ȡ�ļ���С
		body->resize(filesize);  // ��body�����ļ��ռ�
		std::cout << "��ȡ�ļ����ݣ�" << name << "   size��" << filesize << std::endl;

		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "rb+");
		if (fp == NULL)
		{
			std::cerr << "���ļ�ʧ��\n";
			return false;
		}
		size_t ret = fread(&(*body)[0], 1, filesize, fp);
		if (ret != filesize)
		{
			std::cerr << "���ļ���ȡ����ʧ��\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	}
	static bool ReadRange(const std::string& name, std::string* body, int64_t len, int64_t offset)
	{
		body->resize(len);
		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "rb+");
		if (fp == NULL)
		{
			std::cerr << "���ļ�ʧ��\n";
			return false;
		}
		fseek(fp, offset, SEEK_SET); // ��ת��ָ��λ��

		//��ȡ����
		size_t ret = fread(&(*body)[0], 1, len, fp);
		if (ret != len)
		{
			std::cerr << "���ļ���ȡ����ʧ��\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	}

	//��ȡ�ֿ�����
	static bool GetRange(const std::string& range_str, int* start, int* end)
	{
		size_t  pos1 = range_str.find('-');
		size_t pos2 = range_str.find('=');
		*start = std::atol(range_str.substr(pos2 + 1, pos1 - pos2 - 1).c_str());
		std::cout << "range_str.substr(pos1 + 1, pos1 - pos2 - 1):" << range_str.substr(pos1 + 1, pos1 - pos2 - 1) << std::endl;
		*end = std::atol(range_str.substr(pos1 + 1).c_str());
		std::cout << "range_str.substr(pos1 + 1):" << range_str.substr(pos1 + 1) << std::endl;
		return true;
	}


};


//*********************************************************************

class Adapter  // ������Ϣ�ṹ��
{
public:
	uint32_t _ip_addr; // �����ϵ�IP��ַ
	uint32_t _mask_addr; // �����ϵ���������
};

class AdapterUtil
{
public:
#ifdef _WIN32
	// windows�µĻ�ȡ������Ϣʵ�֣�
	static bool GetAllAdapter(std::vector<Adapter> *list) {

		// IP_ADAPTER_INFO ���������Ϣ�Ľṹ��
		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
		// ֻ�ڶ��Ͽ���һ��������Ϣ�ṹ�Ŀռ�

		uint64_t all_adapter_size = sizeof(PIP_ADAPTER_INFO);
		// all_adapter_size���ڻ�ȡʵ������������Ϣ��ռ�ռ��С
		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size); // Ҫ����ǿת��PULONG����
		// ret��ȡ�ӿڳɹ����ķ���ֵ

		// �����ȿ�����һ��������Ϣ�Ŀռ䣬���ж����������ô�߽�������ɾ�������¿��١�
		if (ret == ERROR_BUFFER_OVERFLOW) {
			// ��ʾ��ǰ�������ռ䲻�㣬������¸�ָ������ռ�
			delete p_adapters;
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];//BYTEָ�����ֽ�
			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size); // ���»�ȡ������Ϣ
		}

		while (p_adapters) { // p_adaptersָ��ָ��������������������е�������Ϣ��

			Adapter adapter; // ʵ����һ��������Ϣ�ṹ��Ķ���

			// inet_pton��һ���ַ������ʮ���Ƶ�IP��ַת��Ϊ�����ֽ����IP��ַ
			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);

			if (adapter._ip_addr != 0) // ��Ϊ��Щ������û�����ã�����IP��ַΪ0�� 
			{
				list->push_back(adapter);  // ��������Ϣ��ӵ�vector�з��ظ��û�
				std::cout << "��������:" << p_adapters->AdapterName << std::endl;
				std::cout << "��������:" << p_adapters->Description << std::endl;
				std::cout << "IP��ַ:" << p_adapters->IpAddressList.IpAddress.String << std::endl;
				std::cout << "��������:" << p_adapters->IpAddressList.IpMask.String << std::endl;
				std::cout << std::endl;
			}
			p_adapters = p_adapters->Next;
		}

		delete p_adapters; // ע��Ҫ�ͷţ����ͷŻ�����ڴ�й©
		return true;
	}
#else
	// linux�µĻ�ȡ������Ϣʵ��
	bool GetAllAdapter(std::vector<Adapter> *list) {
		return true;
	}
#endif
};