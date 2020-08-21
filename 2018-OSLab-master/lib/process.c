#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"

PUBLIC int process_running = TESTA;//当前位于的进程

PUBLIC int messageA(char text[])//进程A接受到消息
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = MESSAGE_A;
	strcpy(msg.text, text);//复制文本
	send_recv(BOTH, TESTA, &msg);
	return msg.RETVAL;
}

PUBLIC int messageB(char text[])//进程B接受到消息
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = MESSAGE_B;
	strcpy(msg.text, text);//复制文本
	send_recv(BOTH, TESTB, &msg);
	return msg.RETVAL;
}

PUBLIC int messageC(char text[])//进程B接受到消息
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = MESSAGE_C;
	strcpy(msg.text, text);//复制文本
	send_recv(BOTH, TESTC, &msg);
	return msg.RETVAL;
}

PUBLIC void processA()//切换到进程A
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = PROCESS_A;
	process_running = TESTA;//当前位于进程A
	send_recv(SEND, TESTA, &msg);
	return;
}

PUBLIC void processB()//切换到进程B
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = PROCESS_B;
	process_running = TESTB;//当前位于进程B
	send_recv(SEND, TESTB, &msg);
	return;
}

PUBLIC void processC()//切换到进程C
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = PROCESS_C;
	process_running = TESTC;//当前位于进程C
	send_recv(SEND, TESTC, &msg);
	return;
}

void ShowProcess()//展示进程信息
{
	printf("********************************************************\n");
	printf("    pid    |      name      |   priority   |   running  \n");
	int pid;//进程的编号
	char name[128]; //进程的名称
	int priority;//进程的优先级
	char running[128];//进程的运行状态
	int num_space1;
	int num_space2;
	for (int i = 0; i < NR_TASKS + NR_PROCS; ++i)//遍历进程表
	{
		if (proc_table[i].p_flags != FREE_SLOT)
		{
			pid = proc_table[i].pid;
			strcpy(name, proc_table[i].name);
			num_space1 = (16 - strlen(name)) / 2;//对齐
			num_space2 = 16 - strlen(name) - num_space1;//对齐
			priority = proc_table[i].priority;
			if (proc_table[i].p_flags == FREE_SLOT)
			{
				strcpy(running, " NO");
			}
			else strcpy(running, "YES");
			printf("     %d      ", pid);
			for (int j = 0; j < num_space1; j++)
			{
				printf(" ");
			}
			printf("%s", name);
			for (int k = 0; k < num_space2; k++)
			{
				printf(" ");
			}
			printf("       %d            %s    \n", priority, running);
		}
	}
	printf("********************************************************\n");
}