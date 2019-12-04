/*

#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>

//定义一个全局变量
int a=1;
void sigcb(int no)
{
	a=0;
}
int main()
{
	signal(SIGINT,sigcb);

	//kill(getpid(),SIGQUIT);//向任意进程发送任意信号，这是一个系统调用
	//raise(SIGQUIT);//给进程自身发送信号
	//abort();//给自身发送SIGABRT信号，引发进程非正常终止
	//alarm(3);//定时器

	while(a)
	{
		//printf("------------------\n");
		//sleep(10);
	}
	return 0;
}

*/

//*******************************************************************************
//以上代码，前边的变量都是在内存里边，编译的时候就说这块变量在内存中的什么地址，如果你要对这个变量进行判断，就要从内存中取这个数据，在 vim loop.c 写完代码后，vim Makefile 、make 、./loop 命令执行后，是个死循环，按ctrl+c 即可终止。但在vim loop.c后执行 gcc -O2 loop.c -o loop 命令，对代码进行2级优化，然后再 ./loop 运行，你会发现，此时 按ctrl+c 已经不起作用了。因为在2级优化之后，gcc发现a这个变量使用频率非常高，所以把a这个变量的值 直接放到寄存器里面去了，你要是取a这个数据就直接从寄存器里面去取，不需要从内存中取了。因此造成，a这个值在寄存器里，当我们按ctrl+c时，改了a的值，但是我们改的是a这个变量内存中的值，人家并不从内存中直接获取，而是从寄存器中获取，所以导致我们改了之后，它取的是寄存器中的旧值，并不重新获取内存的新值，导致程序紊乱，这就是编译器过度优化。
//*******************************************************************************

//所以我们为了防止这种情况，我们对这个变量进行一个修饰：
//防止一个变量被过度优化，用 volatile

#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>

//定义一个全局变量
volatile int a=1;

void sigcb(int no)
{
	a=0;
}
int main()
{
	signal(SIGINT,sigcb);

	//kill(getpid(),SIGQUIT);//向任意进程发送任意信号，这是一个系统调用
	//raise(SIGQUIT);//给进程自身发送信号
	//abort();//给自身发送SIGABRT信号，引发进程非正常终止
	//alarm(3);//定时器

	while(a)
	{
		//printf("------------------\n");
		//sleep(10);
	}
	return 0;
}


