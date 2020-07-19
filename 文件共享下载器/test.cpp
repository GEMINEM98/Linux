#define _CRT_SECURE_NO_WARNINGS 1

#include"client.hpp"
#include"server.hpp"
#include<stdlib.h>

void helloworld(const httplib::Request &req, httplib::Response &rsp)
{
	printf("httplib������յ�һ������:%s\n", req.path.c_str());
	// set_content��http��Ӧ���һ�����ģ�"text/html"����������
	rsp.set_content("<html><h1>HelloWorld</h1></html>", "text/html");
	rsp.status = 200;
}

void Scandir()  // ���Ŀ¼
{
	//boost::filesystem::path req_path().filename() ��ȡ�ļ����ƣ�abc/filename.txt ---> filename.txt
	// boost::filesystem::exists() �ж��ļ��Ƿ����

	const char* ptr = "./";
	boost::filesystem::directory_iterator begin(ptr); // ����һ��Ŀ¼����������
	boost::filesystem::directory_iterator end; // ����Ҫ�����κεĶ��󣬲������������Ĭ�Ϲ��죬��Ϊ����һ����β

	// ��ʼ����Ŀ¼
	for (; begin != end; ++begin)
	{
		//begin->status() Ŀ¼�е�ǰ�ļ���״̬��Ϣ
		//boost::filesystem::is_directory() �жϵ�ǰ�ļ��Ƿ���һ��Ŀ¼
		if (boost::filesystem::is_directory(begin->status()))
		{
			//begin->path().string()��ȡ��ǰ�����ļ����ļ���
			std::cout << begin->path().string() << "��һ��Ŀ¼\n";
		}
		else
		{
			std::cout << begin->path().string() << "��һ����ͨ�ļ�\n";
			//begin->path().filename() ��ȡ���ļ�·�����е��ļ����ƣ���Ҫ·��
			std::cout << "�ļ�����" << begin->path().filename().string() << std::endl;
		}
	}
}

void test()
{
	Scandir();
	Sleep(1000000);
}

void ClientRun()
{
	//Sleep(1);
	Client cli;
	cli.Start();
}

int main(int argc, char *argv[])
{

	//test();

	std::thread thr_client(ClientRun);

	Server srv;
	srv.Start();

	system("pause");
	return 0;
}