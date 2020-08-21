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

PUBLIC int process_running = TESTA;//��ǰλ�ڵĽ���

PUBLIC int messageA(char text[])//����A���ܵ���Ϣ
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = MESSAGE_A;
	strcpy(msg.text, text);//�����ı�
	send_recv(BOTH, TESTA, &msg);
	return msg.RETVAL;
}

PUBLIC int messageB(char text[])//����B���ܵ���Ϣ
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = MESSAGE_B;
	strcpy(msg.text, text);//�����ı�
	send_recv(BOTH, TESTB, &msg);
	return msg.RETVAL;
}

PUBLIC int messageC(char text[])//����B���ܵ���Ϣ
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = MESSAGE_C;
	strcpy(msg.text, text);//�����ı�
	send_recv(BOTH, TESTC, &msg);
	return msg.RETVAL;
}

PUBLIC void processA()//�л�������A
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = PROCESS_A;
	process_running = TESTA;//��ǰλ�ڽ���A
	send_recv(SEND, TESTA, &msg);
	return;
}

PUBLIC void processB()//�л�������B
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = PROCESS_B;
	process_running = TESTB;//��ǰλ�ڽ���B
	send_recv(SEND, TESTB, &msg);
	return;
}

PUBLIC void processC()//�л�������C
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = PROCESS_C;
	process_running = TESTC;//��ǰλ�ڽ���C
	send_recv(SEND, TESTC, &msg);
	return;
}

void ShowProcess()//չʾ������Ϣ
{
	printf("********************************************************\n");
	printf("    pid    |      name      |   priority   |   running  \n");
	int pid;//���̵ı��
	char name[128]; //���̵�����
	int priority;//���̵����ȼ�
	char running[128];//���̵�����״̬
	int num_space1;
	int num_space2;
	for (int i = 0; i < NR_TASKS + NR_PROCS; ++i)//�������̱�
	{
		if (proc_table[i].p_flags != FREE_SLOT)
		{
			pid = proc_table[i].pid;
			strcpy(name, proc_table[i].name);
			num_space1 = (16 - strlen(name)) / 2;//����
			num_space2 = 16 - strlen(name) - num_space1;//����
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