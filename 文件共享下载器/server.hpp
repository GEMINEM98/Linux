#pragma once
#include<thread>// �̵߳�ͷ�ļ�����ΪC++��û��pthread_create...
#include"util.hpp"
#include"httplib.h"

#define P2P_PORT 9000  // �˿�
#define MAX_IPBUFFER 16  // IP��ַ�ռ�
#define MAX_RANGE  (100*1024*1024)  // 1 ����10λ=1k  ����20λ=1M    5���ֽ�
#define SHARED_PATH "./Shared/"  // ����Ŀ¼
#define DOWNLOAD_PATH "./Download/"  // �����ļ��ŵ����·����


class Server
{
public:
	// ���ⲿ����start�ӿھͿ�����ɷ���˵Ĵ����
	bool Start()
	{
		//��ӶԿͻ�������Ĵ���ʽ��Ӧ��ϵ
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);

		//�ļ����ǲ����ģ���������ʽ���������ַ���ָ���ĸ�ʽ����ʾ���йؼ�����������
		// .��ʾ���˻س��ͻ��е������ַ���*��ʾ�ַ������;
		// .*��ʾƥ���\n��\r֮����ַ�����Ρ�
		_srv.Get("/download/.*", Download);
		//��ֹ���Ϸ��������ͻ������������м���download·����ֻ��ʾ���ܡ�

		//server��û��HEAD���󷽷���HEAD��GET���õ���Get����

		_srv.listen("0.0.0.0", P2P_PORT);  // ��һ�������������������в���ȥ
		return true;
	}

private:
	// ������Ե���Ӧ����
	static void HostPair(const httplib::Request &req, httplib::Response &rsp)
	{
		rsp.status = 200;
		return;
	}

	//��Ӧ�����ļ��б�������������һ������Ŀ¼���������Ŀ¼�µ��ļ�����Ҫ�����˹����
	static void ShareList(const httplib::Request &req, httplib::Response &rsp)
	{
		//1.�鿴Ŀ¼�Ƿ���ڣ��������򴴽�Ŀ¼
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);  // ����Ŀ¼
		}
		boost::filesystem::directory_iterator begin(SHARED_PATH);// ʵ����Ŀ¼������
		boost::filesystem::directory_iterator end;   //ʵ������������ĩβ��Ĭ�Ϲ������ĩβ

		//��ʼ����Ŀ¼
		for (; begin != end; ++begin)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				// ��ǰ�汾����ֻ��ȡ��ͨ�ļ����ƣ�������㼶Ŀ¼�Ĳ���
				continue;
			}
			//ֻҪ�ļ�������Ҫ·��
			std::string name = begin->path().filename().string();  // ��ȡ���ļ����ƣ�Ҫת����string����
			rsp.body += name + "\r\n";  //filename1\r\n filename2\r\n...
		}
		rsp.status = 200;
		return;
	}

	// �����ļ�����Ӧ����
	static void Download(const httplib::Request &req, httplib::Response &rsp)
	{
		std::cout << "������յ��ļ���������:" << req.path << std::endl;
		//req.path---�ͻ����������Դ·��   /download/filename
		boost::filesystem::path req_path(req.path); // ʵ����һ��path����
		//boost::filesystem::path req_path().filename() ��ȡ�ļ����ƣ�abc/filename.txt ---> filename.txt

		std::string name = req_path.filename().string(); //ֻ��ȡ�ļ����ƣ�filename.txt
		std::cout << "������յ�ʵ�ʵ��ļ��������ƣ�" << name << " ·����" << SHARED_PATH << std::endl;
		std::string realpath = SHARED_PATH + name;  //ʵ���ļ�·��Ӧ�����ڹ���Ŀ¼�µ�

		// boost::filesystem::exists() �ж��ļ��Ƿ����
		std::cout << "������յ�ʵ�ʵ��ļ�����·����" << realpath << std::endl;
		if (!boost::filesystem::exists(realpath) || boost::filesystem::is_directory(realpath))
		{
			rsp.status = 404; // ��Ҫ�������Դ������
			return;
		}

		//��httplib�У�Get�ӿڲ�����Կͻ��˵�GET���󷽷���Ҳ��Կͻ��˵�HEAD���󷽷�
		if (req.method == "GET")
		{
			if (req.has_header("Range"))//�ж�����ͷ���Ƿ����Range�ֶ�
			{
				//��Rangeͷ���������һ���ֿ鴫��
				//��Ҫ֪���ֿ������Ƕ���
				std::string range_str = req.get_header_value("Range");//bytes=start-end

				int range_start;
				int range_end;

				FileUtil::GetRange(range_str, &range_start, &range_end);
				int range_len = range_end - range_start + 1;

				std::cout << realpath << ":" << "Range:" << range_start << "-" << range_end << std::endl;
				FileUtil::ReadRange(realpath, &rsp.body, range_len, range_start);
				//���䷶Χ����Ӧ���ģ�Ҫ��ȡ�����ݳ��ȣ�ƫ����ʼλ��

				rsp.status = 206;// �ֿ鴫��ͺ����崫���ͷ����Ϣ�ǲ�һ����
				std::cout << "�������Ӧ�����������\n";
			}
			else
			{
				//û��Rangeͷ��������һ����ɵ��ļ�����
				if (FileUtil::Read(realpath, &rsp.body) == false)
				{
					rsp.status = 500; // �������ڲ�����
					return;
				}
				rsp.status = 200;// �ֿ鴫��ͺ����崫���ͷ����Ϣ�ǲ�һ����
				// ��ʾ����ļ�������ȷ������
			}
		}
		else   //��������HEAD����Ŀͻ���ֻҪͷ����Ҫ����
		{
			int64_t filesize = FileUtil::GetFileSize(realpath);// ��ȡ�ļ���С

			rsp.set_header("Content-Length", (std::to_string(filesize)).c_str()); // ������Ӧͷ����Ϣ

			rsp.status = 200;
		}
		//std::cout << "������ļ�����������Ӧ���\n";
		return;
	}
private:
	httplib::Server _srv;
};