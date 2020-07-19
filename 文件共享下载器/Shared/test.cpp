#define _CRT_SECURE_NO_WARNINGS 1

#include"client.hpp"
#include"server.hpp"
#include<stdlib.h>

void helloworld(const httplib::Request &req, httplib::Response &rsp)
{
	printf("httplib服务端收到一个请求:%s\n", req.path.c_str());
	// set_content给http响应添加一个正文，"text/html"是正文类型
	rsp.set_content("<html><h1>HelloWorld</h1></html>", "text/html");
	rsp.status = 200;
}

void Scandir()  // 浏览目录
{
	//boost::filesystem::path req_path().filename() 获取文件名称：abc/filename.txt ---> filename.txt
	// boost::filesystem::exists() 判断文件是否存在

	const char* ptr = "./";
	boost::filesystem::directory_iterator begin(ptr); // 定义一个目录迭代器对象
	boost::filesystem::directory_iterator end; // 不需要传入任何的对象，不传入对象会调用默认构造，认为这是一个结尾

	// 开始迭代目录
	for (; begin != end; ++begin)
	{
		//begin->status() 目录中当前文件的状态信息
		//boost::filesystem::is_directory() 判断当前文件是否是一个目录
		if (boost::filesystem::is_directory(begin->status()))
		{
			//begin->path().string()获取当前迭代文件的文件名
			std::cout << begin->path().string() << "是一个目录\n";
		}
		else
		{
			std::cout << begin->path().string() << "是一个普通文件\n";
			//begin->path().filename() 获取到文件路径名中的文件名称，不要路径
			std::cout << "文件名：" << begin->path().filename().string() << std::endl;
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