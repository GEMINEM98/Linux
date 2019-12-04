#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

struct sigaction oldact;

//先定义一个回调函数
//没有返回值，有一个int参数的函数
void sigcb(int signum)
{
	printf("recv a signal:%d\n",signum);
	sigaction (signum,&oldact,NULL);//设置信号的回调动作
}
int main()
{
	//修改信号的处理方式，触发上面的回调函数
	//signal(2,sigcb);//设置信号的对应动作

	//int sigaction (int signum, struct sigaction* act, struct siaction* oldact)
	//signum:信号值
	//act：signum当前要修改的新的动作
	//oldact：用于获取signum信号原有的动作（便于以后再还原回去）
	struct sigaction newact;
	newact.sa_handler=sigcb;//复制函数地址,设置自定义回调函数
	newact.sa_flags=0;//使用的是sa_handler这个回调函数（默认使用）
	sigemptyset(&newact.sa_mask);//清空临时要阻塞的信号集合，指的是，在处理当前信号的过程当中，
	               //我们不希望受到其他信号的影响，就把它添加进去。
	
	sigaction(2,&newact,&oldact);//检查或修改与指定信号相关联的处理动作


	while(1)
	{
		printf("----------------\n");
		sleep(10);
	}
	return 0;
}
