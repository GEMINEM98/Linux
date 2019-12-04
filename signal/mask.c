#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

void sigcb(int signum)
{
	printf("recv signal:%d\n",signum);
}

int main()
{
	//两个回调函数
	signal(SIGINT,sigcb);
	signal(40,sigcb);

	//阻塞信号
	sigset_t mask_set,old_set;//定义两个信号集合
	sigemptyset(&mask_set);//清空信号集合

	//向集合中添加信号有两个函数：sigfillset将所有信号添加到集合当中
	//                            sigaddset将指定的信号添加到集合当中
	sigfillset(&mask_set);//添加所有信号到信号集合中
	
	//将所有信号都阻塞
	sigprocmask(SIG_BLOCK,&mask_set,&old_set);//阻塞mask_set中所有信号
	printf("press enter to continue!\n");
	getchar();//获取一个回车，不按回车，程序流程会卡在这里
	
	//解除阻塞，下面两种：
	sigprocmask(SIG_UNBLOCK,&mask_set,NULL);//最好用这种
	//sigprocmask(SIG_SETMASK,&old_set,NULL);//将原有的old设置回去，原有的是什么都没有

	return 0;
}
