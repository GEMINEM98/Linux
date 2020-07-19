#pragma once   // 头文件只包含一次的说明

#include<stdio.h>
#include<iostream>
#include<vector>
#include<fstream>  // 文件操作的头文件
#include<sstream>
#include<stdint.h>
#include<boost/filesystem.hpp>

#ifdef _WIN32
// windows头文件
#include <WinSock2.h>
#include<WS2tcpip.h>  // Windows Socket 协议
#include<iphlpapi.h>  // 获取网卡信息接口的头文件：PIP_ADAPTER_INFO结构、GetAdaptersInfo接口...
#pragma comment(lib,"Iphlpapi.lib")  // 获取网卡信息接口的库文件包含
#pragma comment(lib,"ws2_32.lib")  // Windows下的socket库：inet_pton
#else
// linux头文件
#endif



class StringUtil  //class Dig
{
public:
	static int Str2Dig(const std::string &num)
	{
		std::stringstream tmp;
		tmp << num;  // 把num写入tmp中
		int res;
		tmp >> res;  // 将tmp写入到res中，这里用了运算符重载，根据后面数据类型不同对数据进行转换。
		return res;  // 返回res
	}
};



class FileUtil
{
public:
	static int64_t GetFileSize(const std::string& name)  // 统一封装一个接口
	{
		return boost::filesystem::file_size(name);
	}

	static bool Write(const std::string& name, const std::string& body, int offset = 0) // 第三个参数是默认参数
	{

		//********************************************************* C++标准库进行文件操作

		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "ab+");//  向文件写入数据是 二进制的追加写
		if (fp == NULL)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		fseek(fp, offset, SEEK_SET); //跳转偏移位置
		size_t ret = fwrite(body.c_str(), 1, body.size(), fp);
		if (ret != body.size())
		{
			std::cerr << "向文件写入数据失败\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;

	}

	// 指针参数表示这是一个输出型参数
	// const& 表示这是一个输入型参数
	// & 表示这是一个输入输出型参数
	static bool Read(const std::string& name, std::string* body)
	{
		//********************************************************* C++标准库进行文件操作

		int64_t filesize = boost::filesystem::file_size(name);  // 通过文件名获取文件大小
		body->resize(filesize);  // 给body分配文件空间
		std::cout << "读取文件数据：" << name << "   size：" << filesize << std::endl;

		FILE *fp = NULL;
		fopen_s(&fp, name.c_str(), "rb+");
		if (fp == NULL)
		{
			std::cerr << "打开文件失败\n";
			return false;
		}
		size_t ret = fread(&(*body)[0], 1, filesize, fp);
		if (ret != filesize)
		{
			std::cerr << "从文件读取数据失败\n";
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
			std::cerr << "打开文件失败\n";
			return false;
		}
		fseek(fp, offset, SEEK_SET); // 跳转到指定位置

		//读取数据
		size_t ret = fread(&(*body)[0], 1, len, fp);
		if (ret != len)
		{
			std::cerr << "从文件读取数据失败\n";
			fclose(fp);
			return false;
		}
		fclose(fp);
		return true;
	}

	//获取分块下载
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

class Adapter  // 网卡信息结构体
{
public:
	uint32_t _ip_addr; // 网卡上的IP地址
	uint32_t _mask_addr; // 网卡上的子网掩码
};

class AdapterUtil
{
public:
#ifdef _WIN32
	// windows下的获取网卡信息实现，
	static bool GetAllAdapter(std::vector<Adapter> *list) {

		// IP_ADAPTER_INFO 存放网卡信息的结构体
		PIP_ADAPTER_INFO p_adapters = new IP_ADAPTER_INFO();
		// 只在堆上开辟一块网卡信息结构的空间

		uint64_t all_adapter_size = sizeof(PIP_ADAPTER_INFO);
		// all_adapter_size用于获取实际所有网卡信息所占空间大小
		int ret = GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size); // 要进行强转成PULONG类型
		// ret获取接口成功与否的返回值

		// 上面先开辟了一块网卡信息的空间，若有多块网卡，那么走进来，先删掉再重新开辟。
		if (ret == ERROR_BUFFER_OVERFLOW) {
			// 表示当前缓冲区空间不足，因此重新给指针申请空间
			delete p_adapters;
			p_adapters = (PIP_ADAPTER_INFO)new BYTE[all_adapter_size];//BYTE指的是字节
			GetAdaptersInfo(p_adapters, (PULONG)&all_adapter_size); // 重新获取网卡信息
		}

		while (p_adapters) { // p_adapters指针指向的是链表，链表中是所有的网卡信息。

			Adapter adapter; // 实例化一个网卡信息结构体的对象

			// inet_pton将一个字符串点分十进制的IP地址转换为网络字节序的IP地址
			inet_pton(AF_INET, p_adapters->IpAddressList.IpAddress.String, &adapter._ip_addr);
			inet_pton(AF_INET, p_adapters->IpAddressList.IpMask.String, &adapter._mask_addr);

			if (adapter._ip_addr != 0) // 因为有些网卡并没有启用，导致IP地址为0； 
			{
				list->push_back(adapter);  // 将网卡信息添加到vector中返回给用户
				std::cout << "网卡名称:" << p_adapters->AdapterName << std::endl;
				std::cout << "网卡描述:" << p_adapters->Description << std::endl;
				std::cout << "IP地址:" << p_adapters->IpAddressList.IpAddress.String << std::endl;
				std::cout << "子网掩码:" << p_adapters->IpAddressList.IpMask.String << std::endl;
				std::cout << std::endl;
			}
			p_adapters = p_adapters->Next;
		}

		delete p_adapters; // 注意要释放，不释放会造成内存泄漏
		return true;
	}
#else
	// linux下的获取网卡信息实现
	bool GetAllAdapter(std::vector<Adapter> *list) {
		return true;
	}
#endif
};