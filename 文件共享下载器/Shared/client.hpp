#pragma once

#include<thread>  // �̵߳�ͷ�ļ�����ΪC++��û��pthread_create...
#include"util.hpp"
#include"httplib.h"

#define P2P_PORT 9000  // �˿�
#define MAX_IPBUFFER 16  // IP��ַ�ռ�
#define MAX_RANGE  (100*1024*1024)  // 1 ����10λ=1k  ����20λ=1M    5���ֽ�
//#define MAX_RANGE  (5)  // 1 ����10λ=1k  ����20λ=1M    5���ֽ�
#define SHARED_PATH "./Shared/"  // ����Ŀ¼
#define DOWNLOAD_PATH "./Download/"  // �����ļ��ŵ����·����

class Host
{
public:
	uint32_t  _ip_addr; //Ҫ��Ե�����IP��ַ
	bool  _pair_ret;    //���ڴ����Խ������Գɹ���Ϊtrue��ʧ����Ϊfalse��
};

class Client
{
public:

	// ���ⲿ����Start�ӿھͿ�����ɿͻ��˵Ĵ����
	bool Start()
	{
		//�ͻ��˳�����Ҫѭ�����У���Ϊ�����ļ�����ֻ��һ��
		//ѭ������ÿ������һ���ļ�֮�󶼻����������ԣ����ǲ������
		while (1)
		{
			GetOnlineHost();  // ��ȡ��������
			// ����������������Լ�����ͻ������ú���ĺ�����һ��һ��Ƕ�׵��ã�����Ͳ���Ҫ��������������д����
		}
		return true;
	}
	//������Ե��߳���ں���
	void HostPair(Host *host)  // ��ĳ�Ա�������е�һ��������thisָ��
	{
		//1.��֯HTTPЭ���ʽ����������
		//2.�һ��TCP�ͻ��ˣ������ݷ���
		//3.�ȴ��������˵Ļظ��������н���
		//ʹ�õ�������httplibʵ�ִ�ͻ��ˣ�ע���˴���֪httplib��������ʵ������

		host->_pair_ret = false;

		char buf[MAX_IPBUFFER] = { 0 };  // IP��ַ���Ȳ��ᳬ��16�ֽ�
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);
		// httplib::Client cli(host->_ip_addr, ); // ����������Ϊhttplib.h�У����������string���ͣ�������������ֽ���
		httplib::Client cli(buf, P2P_PORT); //ʵ����httplib�ͻ��˶���
		auto rsp = cli.Get("/hostpair");  //�����˷�����ԴΪ/hostpair��Get���󣬷��ص�������ָ�룬��auto��
										  //�����ӽ���ʧ�ܣ�Get�᷵��NULL���������滹��Ҫ�����пգ��������ʿ�ָ�롣

		if (rsp && rsp->status == 200) //�ж���Ӧ����Ƿ���ȷ 
		{
			host->_pair_ret = true;   //����������Խ������ʾ��Գɹ�
		}
		return;
	}

	//1.��ȡ������������list�д��������Ϣ��һ�ж��ĸ�����������
	bool GetOnlineHost()
	{
		char ch = 'Y'; //�Ƿ�����ƥ�䣬Ĭ���ǽ���ƥ��ģ����Ѿ�ƥ�����online������Ϊ�գ������û�ѡ��
		if (!_online_host.empty())
		{
			std::cout << "�Ƿ����²鿴��������(Y/N)��";// ��Ϊѭ�����ÿͻ��˳���������������б��ȡһ�ξ͹���
			fflush(stdout);
			std::cin >> ch;
		}
		if (ch == 'Y')
		{
			std::cout << "��ʼ����ƥ��...\n";

			//1.��ȡ������Ϣ������������о�����������IP��ַ�б���Ϣ
			std::vector<Adapter> list; // ����
			AdapterUtil::GetAllAdapter(&list);

			//2.��ȡ��������IP��ַ����ӵ�host_list��
			std::vector<Host> host_list;  // ����
			for (int i = 0; i < list.size(); ++i)//ѭ������ĳһ��������Ŀ���ǣ��õ����е�����IP��ַ�б�
			{
				uint32_t ip = list[i]._ip_addr;
				uint32_t mask = list[i]._mask_addr;

				//���������
				uint32_t net = (ntohl(ip & mask)); // ntohlת����С���ֽ���

				//�������������
				uint32_t max_host = (~ntohl(mask));

				for (int j = 1; j < (int32_t)max_host; ++j)  // ��񻯣����ʼ�
				{
					// ������Ϊ0������ţ�������Ϊmax_host��udp�Ĺ㲥��ַ

					uint32_t host_ip = net + j;  // �������IP�ļ���Ӧ��ʹ�������ֽ���(С���ֽ���)������ź�������

					Host host;
					host._ip_addr = htonl(host_ip);// ����������ֽ����IP��ַת���������ֽ���
					host._pair_ret = false;// ��ʾ��Խ����true��ʾ��Գɹ���false��ʾ���ʧ��
					host_list.push_back(host);// ���host_list�н�����������������ַ
				}
			}

			//3.��Host_list�����������߳̽�����ԣ�ȡָ������Ϊstd::thread��һ���ֲ�������Ϊ�˷�ֹ��ɺ��ͷ�

			std::vector<std::thread*> thr_list(host_list.size());  // �����߳��б�
			for (int i = 0; i < host_list.size(); ++i) // ��host_list�е����������߳̽������
			{
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			std::cout << "��������ƥ���У����Ժ�....\n";

			_online_host.clear();   // ��գ�����ƥ��

			//3�����������õ���Ӧ�����Ӧ����Ϊ������������IP��ӵ�_online_host�б���
			for (int i = 0; i < host_list.size(); ++i)//�ȴ������߳����������ϣ��ж���Խ��������������������ӵ�online_host��
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
				{
					_online_host.push_back(host_list[i]);
				}
				delete thr_list[i];
			}
		}
		//4����ӡ���������б����û�ѡ��
		for (int i = 0; i < _online_host.size(); ++i)
		{
			char buf[MAX_IPBUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
			std::cout << "\t" << buf << std::endl;
		}


		std::cout << "��ѡ�������������ȡ�����ļ��б�";
		//�鿴Ŀ¼�Ƿ���ڣ��������򴴽�Ŀ¼
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}
		fflush(stdout);//ˢ�±�׼���������
		std::string select_ip;
		std::cin >> select_ip;
		GetShareList(select_ip); //�û�ѡ������֮�󣬵��û�ȡ�ļ��б�ӿ�
		return true;
	}

	//��ȡ�ļ��б�
	bool GetShareList(const std::string &host_ip)
	{
		//�����˷���һ���ļ��б��ȡ����
		//1.�ȷ�������
		//2.�õ���Ӧ֮�󣬽�������(�ļ�����)

		httplib::Client cli(host_ip.c_str(), P2P_PORT);//ʵ�����ͻ��˶���

		//�鿴Ŀ¼�Ƿ���ڣ��������򴴽�Ŀ¼
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}

		auto rsp = cli.Get("/list");//��ȡ�ļ��б�
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "��ȡ�ļ��б���Ӧ����" << std::endl;
			return false;
		}

		//��ӡ����--����ӡ�������Ӧ���ļ������б����û�ѡ��
		//body:filename1\r\nfilename
		std::cout << rsp->body << std::endl;// body��һ��string���������ŵľ������е�������Ϣ
		std::cout << "\n��ѡ��Ҫ���ص��ļ���";
		fflush(stdout);
		std::string filename;
		std::cin >> filename;// ��ȡ���

		RangeDownload(host_ip, filename);// �����ļ�

		return true;
	}


	bool DownloadFile(const std::string &host_ip, const std::string& filename)
	{
		//���ļ�һ�������أ��������ļ��Ƚ�Σ��

		//1.�����˷����ļ���������--��filename
		//2.�õ���Ӧ�������Ӧ�е�body���ľ����ļ�����
		//3.�����ļ������ļ�д���ļ��У��ر��ļ�
		std::string req_path = "/download/" + filename; // ��Ϊ�������涼����/���ļ�������ʽ����������ҲҪ����/
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		std::cout << "�����˷����ļ���������:" << host_ip << req_path << std::endl;
		auto rsp = cli.Get(req_path.c_str());

		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "�����ļ���ȡ��Ӧʧ�ܣ�/n";
			return false;
		}

		std::cout << "��ȡ�����ļ���Ӧ�ɹ���\n";
		if (!boost::filesystem::exists(DOWNLOAD_PATH))
		{
			boost::filesystem::create_directory(DOWNLOAD_PATH);
		}

		std::string realpath = DOWNLOAD_PATH + filename; // ʵ������

		if (FileUtil::Write(realpath, rsp->body) == false)
		{
			std::cerr << "�ļ�����ʧ�ܣ�\n";
			return false;
		}
		std::cout << "�����ļ��ɹ���\n";
		return true;
	}

	// �����ļ�����Ҫ�����ĸ��������ĸ��ļ�
	// С�ļ� -> ֱ�ӵ���DownloadFile��DownloadFile����util�е�Write����
	// ���ļ� -> ���зֿ鴦������util�е�Write����
	bool RangeDownload(const std::string &host_ip, const std::string &name)
	{
		//1.����Head����ͨ����Ӧ�е�Content_Length��ȡ�ļ���С	
		std::string req_path = "/download/" + name; // ��Ϊ�������涼����/���ļ�������ʽ����������ҲҪ����/
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Head(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cout << "��ȡ�ļ���Сʧ�ܣ�\n";
			return false;
		}
		std::string clen = rsp->get_header_value("Content-Length");//get_header_valueͨ��httpͷ����Ϣ�ֶ�����ȡֵ

		int filesize = StringUtil::Str2Dig(clen);


		//2.�����ļ���С���зֿ�
		//int range_count = filesize / MAX_RANGE;
		//1�����ļ���СС�ڿ��С����ֱ�������ļ�
		if (filesize < MAX_RANGE)
		{
			std::cout << "�ļ���С��ֱ�������ļ���\n";
			return DownloadFile(host_ip, name);
		}
		//����ֿ����
		//2�����ļ���С�����������С����ֿ����=�ļ���С/�ֿ��С + 1
		//3�����ļ���С���������С����ֿ����=�ļ���С/�ֿ��С
		std::cout << "�ļ����󣬴�С�ǣ�" << filesize << "���ֿ������ļ���" << std::endl;

		int range_count = 0;
		if (filesize % MAX_RANGE == 0)
		{
			range_count = filesize / MAX_RANGE;
		}
		else
		{
			range_count = (filesize / MAX_RANGE) + 1;
		}

		// 136  100  0~99  100~135
		int range_start = 0;
		int range_end = 0;
		for (int i = 0; i < range_count; i++)
		{
			range_start = i * MAX_RANGE;// MAX_RANGE ��ʾ�ֿ��С
			if (i == (range_count - 1))// ��ʾ���һ���ֿ�
			{//ĩβ�ֿ�
				range_end = filesize - 1;
			}
			else
			{
				range_end = ((i + 1) * MAX_RANGE) - 1;
			}
			std::cout << "�ͻ�������ֿ����أ�" << range_start << "-" << range_end << std::endl;


			//3.��һ����ֿ���������ݣ��õ���Ӧ��д���ļ���ָ��λ��

			std::stringstream tmp;
			tmp << "bytes=" << range_start << "-" << range_end;// ��֯һ��Rangeͷ��Ϣ������ֵ

			httplib::Client cli(host_ip.c_str(), P2P_PORT);
			httplib::Headers header;
			header.insert(std::make_pair("Range", tmp.str()));
			//header.insert(httplib::make_range_header({ {range_start, range_end} })); //����һ��range����

			auto rsp = cli.Get(req_path.c_str(), header);//�����˷���һ���ֶ�����
			if (rsp == NULL || rsp->status != 206)
			{
				std::cout << "���������ļ�ʧ��!\n";
				return false;
			}
			std::string real_path = DOWNLOAD_PATH + name;
			if (!boost::filesystem::exists(DOWNLOAD_PATH))
			{
				boost::filesystem::create_directory(DOWNLOAD_PATH);
			}

			// �õ���Ӧ֮��
			std::cout << "�ͻ��˷ֿ�д���ļ�" << range_start << "-" << range_end << std::endl;
			std::cout << "�ͻ��˷ֿ�д���ļ�:[" << rsp->body << "]\n";
			FileUtil::Write(real_path, rsp->body, range_start); //ÿ�������λ�þ��Ǵ�range_start��ʼ����ģ��������ļ���д�����ݾ��Ǵ����λ�ÿ�ʼд���
			// ���漰�������⣬��Ϊÿ���ֿ�����������ǲ�һ���ģ�
			std::cout << "�ֿ�д���ļ��ɹ�\n";

		}
		std::cout << "�ļ����سɹ���\n";
		return true;
	}

private:
	std::vector<Host> _online_host;// ��������
};