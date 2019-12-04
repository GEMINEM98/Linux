#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

int a=0;
int b=0;
int test()
{
	a++;
	sleep(10);
	b++;
	return a+b;
}

void sigcb(int signo)//回调函数
{
	printf("signal : %d\n",test());
}

int main()
{
	signal(SIGINT,sigcb);//signal设置某一信号的中端动作。2号中断信号，ctrl+c
	printf("main : %d\n",test());
	return 0;
}
