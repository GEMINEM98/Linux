#pragma once
#include<thread>// 线程的头文件：因为C++中没有pthread_create...
#include"util.hpp"
#include"httplib.h"

#define P2P_PORT 9000  // 端口
#define MAX_IPBUFFER 16  // IP地址空间
#define MAX_RANGE  (100*1024*1024)  // 1 左移10位=1k  左移20位=1M    5个字节
#define SHARED_PATH "./Shared/"  // 共享目录
#define DOWNLOAD_PATH "./Download/"  // 下载文件放到这个路径下


class Server
{
public:
	// 在外部调用start接口就可以完成服务端的搭建整合
	bool Start()
	{
		//添加对客户端请求的处理方式对应关系
		_srv.Get("/hostpair", HostPair);
		_srv.Get("/list", ShareList);

		//文件名是不定的，用正则表达式：将特殊字符以指定的格式，表示具有关键特征的数据
		// .表示除了回车和换行的任意字符；*表示字符任意次;
		// .*表示匹配除\n和\r之外的字符任意次。
		_srv.Get("/download/.*", Download);
		//防止与上方的请求冲突，因此在请求中加入download路径，只表示功能。

		//server中没有HEAD请求方法，HEAD和GET都用的是Get函数

		_srv.listen("0.0.0.0", P2P_PORT);  // 是一个阻塞函数，程序运行不下去
		return true;
	}

private:
	// 主机配对的相应动作
	static void HostPair(const httplib::Request &req, httplib::Response &rsp)
	{
		rsp.status = 200;
		return;
	}

	//响应共享文件列表，在主机上设置一个共享目录，凡是这个目录下的文件都是要给别人共享的
	static void ShareList(const httplib::Request &req, httplib::Response &rsp)
	{
		//1.查看目录是否存在，不存在则创建目录
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);  // 创建目录
		}
		boost::filesystem::directory_iterator begin(SHARED_PATH);// 实例化目录迭代器
		boost::filesystem::directory_iterator end;   //实例化迭代器的末尾，默认构造就是末尾

		//开始迭代目录
		for (; begin != end; ++begin)
		{
			if (boost::filesystem::is_directory(begin->status()))
			{
				// 当前版本我们只获取普通文件名称，不做多层级目录的操作
				continue;
			}
			//只要文件名，不要路径
			std::string name = begin->path().filename().string();  // 获取到文件名称，要转换成string类型
			rsp.body += name + "\r\n";  //filename1\r\n filename2\r\n...
		}
		rsp.status = 200;
		return;
	}

	// 下载文件的相应动作
	static void Download(const httplib::Request &req, httplib::Response &rsp)
	{
		std::cout << "服务端收到文件下载请求:" << req.path << std::endl;
		//req.path---客户端请求的资源路径   /download/filename
		boost::filesystem::path req_path(req.path); // 实例化一个path对象
		//boost::filesystem::path req_path().filename() 获取文件名称：abc/filename.txt ---> filename.txt

		std::string name = req_path.filename().string(); //只获取文件名称：filename.txt
		std::cout << "服务端收到实际的文件下载名称：" << name << " 路径：" << SHARED_PATH << std::endl;
		std::string realpath = SHARED_PATH + name;  //实际文件路径应该是在共享目录下的

		// boost::filesystem::exists() 判断文件是否存在
		std::cout << "服务端收到实际的文件下载路径：" << realpath << std::endl;
		if (!boost::filesystem::exists(realpath) || boost::filesystem::is_directory(realpath))
		{
			rsp.status = 404; // 你要请求的资源不存在
			return;
		}

		//在httplib中，Get接口不但针对客户端的GET请求方法，也针对客户端的HEAD请求方法
		if (req.method == "GET")
		{
			if (req.has_header("Range"))//判断请求头中是否包含Range字段
			{
				//有Range头部，这就是一个分块传输
				//需要知道分块区间是多少
				std::string range_str = req.get_header_value("Range");//bytes=start-end

				int range_start;
				int range_end;

				FileUtil::GetRange(range_str, &range_start, &range_end);
				int range_len = range_end - range_start + 1;

				std::cout << realpath << ":" << "Range:" << range_start << "-" << range_end << std::endl;
				FileUtil::ReadRange(realpath, &rsp.body, range_len, range_start);
				//区间范围，响应正文，要读取的数据长度，偏移起始位置

				rsp.status = 206;// 分块传输和和整体传输的头部信息是不一样的
				std::cout << "服务端响应区间数据完毕\n";
			}
			else
			{
				//没有Range头部，则是一个完成的文件下载
				if (FileUtil::Read(realpath, &rsp.body) == false)
				{
					rsp.status = 500; // 服务器内部错误
					return;
				}
				rsp.status = 200;// 分块传输和和整体传输的头部信息是不一样的
				// 表示这次文件下载正确处理了
			}
		}
		else   //这个是针对HEAD请求的客户端只要头部不要正文
		{
			int64_t filesize = FileUtil::GetFileSize(realpath);// 获取文件大小

			rsp.set_header("Content-Length", (std::to_string(filesize)).c_str()); // 设置响应头部信息

			rsp.status = 200;
		}
		//std::cout << "服务端文件下载请求响应完毕\n";
		return;
	}
private:
	httplib::Server _srv;
};