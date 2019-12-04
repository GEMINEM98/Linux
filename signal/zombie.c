#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

void sigcb(int signo)
{
	//waitpid(-1,NULL,0);//等待任意子进程退出，处理僵尸进程
	
	while(watipid(-1,NULL,WNOHANG)>0)//WNOHANG是一个非阻塞
	{ }
	
}

int main()
{
	signal(SIGCHLD,sigcb);

	int pid=fork();
	if(pid==0)
	{
		sleep(3);
		exit(0);
	}
	while(1)
	{
		printf("damajiang~~\n");
		sleep(1);
	}
	return 0;
}
