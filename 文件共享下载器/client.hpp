#pragma once

#include<thread>  // 线程的头文件：因为C++中没有pthread_create...
#include"util.hpp"
#include"httplib.h"

#define P2P_PORT 9000  // 端口
#define MAX_IPBUFFER 16  // IP地址空间
#define MAX_RANGE  (100*1024*1024)  // 1 左移10位=1k  左移20位=1M    5个字节
//#define MAX_RANGE  (5)  // 1 左移10位=1k  左移20位=1M    5个字节
#define SHARED_PATH "./Shared/"  // 共享目录
#define DOWNLOAD_PATH "./Download/"  // 下载文件放到这个路径下

class Host
{
public:
	uint32_t  _ip_addr; //要配对的主机IP地址
	bool  _pair_ret;    //用于存放配对结果，配对成功则为true，失败则为false；
};

class Client
{
public:

	// 在外部调用Start接口就可以完成客户端的搭建整合
	bool Start()
	{
		//客户端程序需要循环运行，因为下载文件不是只下一次
		//循环运行每次下载一个文件之后都会进行主机配对，这是不合理的
		while (1)
		{
			GetOnlineHost();  // 获取在线主机
			// 调用这个函数，它自己本身就会逐层调用后面的函数，一层一层嵌套调用，这里就不需要将其他函数单独写出来
		}
		return true;
	}
	//主机配对的线程入口函数
	void HostPair(Host *host)  // 类的成员函数，有第一个隐含的this指针
	{
		//1.组织HTTP协议格式的请求数据
		//2.搭建一个TCP客户端，将数据发送
		//3.等待服务器端的回复，并进行解析
		//使用第三方库httplib实现搭建客户端，注：此处须知httplib的总体上实现流程

		host->_pair_ret = false;

		char buf[MAX_IPBUFFER] = { 0 };  // IP地址长度不会超过16字节
		inet_ntop(AF_INET, &host->_ip_addr, buf, MAX_IPBUFFER);
		// httplib::Client cli(host->_ip_addr, ); // 参数错误，因为httplib.h中，这个参数是string类型，这里的是网络字节序
		httplib::Client cli(buf, P2P_PORT); //实例化httplib客户端对象
		auto rsp = cli.Get("/hostpair");  //向服务端发送资源为/hostpair的Get请求，返回的是智能指针，用auto简单
										  //若连接建立失败，Get会返回NULL，所以下面还需要进行判空，否则会访问空指针。

		if (rsp && rsp->status == 200) //判断响应结果是否正确 
		{
			host->_pair_ret = true;   //重置主机配对结果，表示配对成功
		}
		return;
	}

	//1.获取在线主机，将list中存的网卡信息逐一判断哪个是在线主机
	bool GetOnlineHost()
	{
		char ch = 'Y'; //是否重新匹配，默认是进行匹配的，若已经匹配过，online主机不为空，则让用户选择
		if (!_online_host.empty())
		{
			std::cout << "是否重新查看在线主机(Y/N)：";// 因为循环调用客户端程序，这个在线主机列表获取一次就够了
			fflush(stdout);
			std::cin >> ch;
		}
		if (ch == 'Y')
		{
			std::cout << "开始主机匹配...\n";

			//1.获取网卡信息，进而获得所有局域网中所有IP地址列表信息
			std::vector<Adapter> list; // 网卡
			AdapterUtil::GetAllAdapter(&list);

			//2.获取所有主机IP地址，添加到host_list中
			std::vector<Host> host_list;  // 主机
			for (int i = 0; i < list.size(); ++i)//循环的是某一块网卡，目的是：得到所有的主机IP地址列表
			{
				uint32_t ip = list[i]._ip_addr;
				uint32_t mask = list[i]._mask_addr;

				//计算网络号
				uint32_t net = (ntohl(ip & mask)); // ntohl转换成小端字节序

				//计算最大主机号
				uint32_t max_host = (~ntohl(mask));

				for (int j = 1; j < (int32_t)max_host; ++j)  // 规格化，见笔记
				{
					// 主机号为0是网络号，主机号为max_host是udp的广播地址

					uint32_t host_ip = net + j;  // 这个主机IP的计算应该使用主机字节序(小端字节序)的网络号和主机号

					Host host;
					host._ip_addr = htonl(host_ip);// 将这个主机字节序的IP地址转换成网络字节序
					host._pair_ret = false;// 表示配对结果，true表示配对成功，false表示配对失败
					host_list.push_back(host);// 这个host_list中将包含了所有主机地址
				}
			}

			//3.对Host_list中主机创建线程进行配对，取指针是因为std::thread是一个局部变量，为了防止完成后被释放

			std::vector<std::thread*> thr_list(host_list.size());  // 创建线程列表
			for (int i = 0; i < host_list.size(); ++i) // 对host_list中的主机创建线程进行配对
			{
				thr_list[i] = new std::thread(&Client::HostPair, this, &host_list[i]);
			}
			std::cout << "正在主机匹配中，请稍后....\n";

			_online_host.clear();   // 清空，重新匹配

			//3、若配对请求得到响应，则对应主机为在线主机，则将IP添加到_online_host列表中
			for (int i = 0; i < host_list.size(); ++i)//等待所有线程主机配对完毕，判断配对结果，将所有在线主机添加到online_host中
			{
				thr_list[i]->join();
				if (host_list[i]._pair_ret == true)
				{
					_online_host.push_back(host_list[i]);
				}
				delete thr_list[i];
			}
		}
		//4、打印在线主机列表，供用户选择
		for (int i = 0; i < _online_host.size(); ++i)
		{
			char buf[MAX_IPBUFFER] = { 0 };
			inet_ntop(AF_INET, &_online_host[i]._ip_addr, buf, MAX_IPBUFFER);
			std::cout << "\t" << buf << std::endl;
		}


		std::cout << "请选择配对主机，获取共享文件列表：";
		//查看目录是否存在，不存在则创建目录
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}
		fflush(stdout);//刷新标准输出缓冲区
		std::string select_ip;
		std::cin >> select_ip;
		GetShareList(select_ip); //用户选择主机之后，调用获取文件列表接口
		return true;
	}

	//获取文件列表
	bool GetShareList(const std::string &host_ip)
	{
		//向服务端发送一个文件列表获取请求
		//1.先发送请求
		//2.得到相应之后，解析正文(文件名称)

		httplib::Client cli(host_ip.c_str(), P2P_PORT);//实例化客户端对象

		//查看目录是否存在，不存在则创建目录
		if (!boost::filesystem::exists(SHARED_PATH))
		{
			boost::filesystem::create_directory(SHARED_PATH);
		}

		auto rsp = cli.Get("/list");//获取文件列表
		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "获取文件列表响应错误！" << std::endl;
			return false;
		}

		//打印正文--》打印服务端响应的文件名称列表，供用户选择
		//body:filename1\r\nfilename
		std::cout << rsp->body << std::endl;// body是一个string对象，里面存放的就是所有的正文信息
		std::cout << "\n请选择要下载的文件：";
		fflush(stdout);
		std::string filename;
		std::cin >> filename;// 获取完毕

		RangeDownload(host_ip, filename);// 下载文件

		return true;
	}


	bool DownloadFile(const std::string &host_ip, const std::string& filename)
	{
		//若文件一次性下载，遇到大文件比较危险

		//1.向服务端发送文件下载请求--》filename
		//2.得到响应结果，响应中的body正文就是文件数据
		//3.创建文件，将文件写入文件中，关闭文件
		std::string req_path = "/download/" + filename; // 因为我们上面都是以/加文件名的形式，所以这里也要加上/
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		std::cout << "向服务端发送文件下载请求:" << host_ip << req_path << std::endl;
		auto rsp = cli.Get(req_path.c_str());

		if (rsp == NULL || rsp->status != 200)
		{
			std::cerr << "下载文件获取响应失败！/n";
			return false;
		}

		std::cout << "获取下载文件响应成功！\n";
		if (!boost::filesystem::exists(DOWNLOAD_PATH))
		{
			boost::filesystem::create_directory(DOWNLOAD_PATH);
		}

		std::string realpath = DOWNLOAD_PATH + filename; // 实际下载

		if (FileUtil::Write(realpath, rsp->body) == false)
		{
			std::cerr << "文件下载失败！\n";
			return false;
		}
		std::cout << "下载文件成功！\n";
		return true;
	}

	// 下载文件，你要下载哪个主机的哪个文件
	// 小文件 -> 直接调用DownloadFile，DownloadFile调用util中的Write下载
	// 大文件 -> 进行分块处理，调用util中的Write下载
	bool RangeDownload(const std::string &host_ip, const std::string &name)
	{
		//1.发送Head请求，通过响应中的Content_Length获取文件大小	
		std::string req_path = "/download/" + name; // 因为我们上面都是以/加文件名的形式，所以这里也要加上/
		httplib::Client cli(host_ip.c_str(), P2P_PORT);
		auto rsp = cli.Head(req_path.c_str());
		if (rsp == NULL || rsp->status != 200)
		{
			std::cout << "获取文件大小失败！\n";
			return false;
		}
		std::string clen = rsp->get_header_value("Content-Length");//get_header_value通过http头部信息字段名获取值

		int filesize = StringUtil::Str2Dig(clen);


		//2.根据文件大小进行分块
		//int range_count = filesize / MAX_RANGE;
		//1、若文件大小小于块大小，则直接下载文件
		if (filesize < MAX_RANGE)
		{
			std::cout << "文件较小，直接下载文件：\n";
			return DownloadFile(host_ip, name);
		}
		//计算分块个数
		//2、若文件大小不能整除块大小，则分块个数=文件大小/分块大小 + 1
		//3、若文件大小能整除块大小，则分块个数=文件大小/分块大小
		std::cout << "文件过大，大小是：" << filesize << "，分块下载文件。" << std::endl;

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
			range_start = i * MAX_RANGE;// MAX_RANGE 表示分块大小
			if (i == (range_count - 1))// 表示最后一个分块
			{//末尾分块
				range_end = filesize - 1;
			}
			else
			{
				range_end = ((i + 1) * MAX_RANGE) - 1;
			}
			std::cout << "客户端请求分块下载：" << range_start << "-" << range_end << std::endl;


			//3.逐一请求分块区间的数据，得到响应后写入文件的指定位置

			std::stringstream tmp;
			tmp << "bytes=" << range_start << "-" << range_end;// 组织一个Range头信息的区间值

			httplib::Client cli(host_ip.c_str(), P2P_PORT);
			httplib::Headers header;
			header.insert(std::make_pair("Range", tmp.str()));
			//header.insert(httplib::make_range_header({ {range_start, range_end} })); //设置一个range区间

			auto rsp = cli.Get(req_path.c_str(), header);//向服务端发送一个分段请求
			if (rsp == NULL || rsp->status != 206)
			{
				std::cout << "区间下载文件失败!\n";
				return false;
			}
			std::string real_path = DOWNLOAD_PATH + name;
			if (!boost::filesystem::exists(DOWNLOAD_PATH))
			{
				boost::filesystem::create_directory(DOWNLOAD_PATH);
			}

			// 得到响应之后
			std::cout << "客户端分块写入文件" << range_start << "-" << range_end << std::endl;
			std::cout << "客户端分块写入文件:[" << rsp->body << "]\n";
			FileUtil::Write(real_path, rsp->body, range_start); //每次请求的位置就是从range_start开始请求的，所以向文件中写入数据就是从这个位置开始写入的
			// 不涉及锁的问题，因为每个分块请求的区间是不一样的，
			std::cout << "分块写入文件成功\n";

		}
		std::cout << "文件下载成功！\n";
		return true;
	}

private:
	std::vector<Host> _online_host;// 在线主机
};