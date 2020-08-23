
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
							main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
													Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
#include "file.h"
#include "2048Game.h"

/*======================================================================*
							kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	struct task* p_task;
	struct proc* p_proc = proc_table;
	char* p_task_stack = task_stack + STACK_SIZE_TOTAL;
	// u16   selector_ldt = SELECTOR_LDT_FIRST;
	u8    privilege;
	u8    rpl;
	int   eflags;
	int   i, j;
	int   prio;
	for (i = 0; i < NR_TASKS + NR_PROCS; i++)
	{
		if (i >= NR_TASKS + NR_NATIVE_PROCS) {
			p_proc->p_flags = FREE_SLOT;
			p_proc++;
			p_task++;
			continue;
		}
		if (i < NR_TASKS)
		{     /* 任务 */
			p_task = task_table + i;
			privilege = PRIVILEGE_TASK;
			rpl = RPL_TASK;
			eflags = 0x1202; /* IF=1, IOPL=1, bit 2 is always 1   1 0010 0000 0010(2)*/
			prio = 15;     //设定优先级为15
		}
		else
		{                  /* 用户进程 */
			p_task = user_proc_table + (i - NR_TASKS);
			privilege = PRIVILEGE_USER;
			rpl = RPL_USER;
			eflags = 0x202; /* IF=1, bit 2 is always 1              0010 0000 0010(2)*/
			prio = 5;     //设定优先级为5
		}

		strcpy(p_proc->name, p_task->name); /* 设定进程名称 */
		p_proc->p_parent = NO_TASK;

		if (strcmp(p_task->name, "Init") != 0) {
			p_proc->ldts[INDEX_LDT_C] = gdt[SELECTOR_KERNEL_CS >> 3];
			p_proc->ldts[INDEX_LDT_RW] = gdt[SELECTOR_KERNEL_DS >> 3];

			/* change the DPLs */
			p_proc->ldts[INDEX_LDT_C].attr1 = DA_C | privilege << 5;
			p_proc->ldts[INDEX_LDT_RW].attr1 = DA_DRW | privilege << 5;
		}
		else {      /* INIT process */
			unsigned int k_base;
			unsigned int k_limit;
			int ret = get_kernel_map(&k_base, &k_limit);
			assert(ret == 0);
			init_descriptor(&p_proc->ldts[INDEX_LDT_C],
				0, /* bytes before the entry point
					* are useless (wasted) for the
					* INIT process, doesn't matter
					*/
				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_C | privilege << 5);

			init_descriptor(&p_proc->ldts[INDEX_LDT_RW],
				0, /* bytes before the entry point
					* are useless (wasted) for the
					* INIT process, doesn't matter
					*/
				(k_base + k_limit) >> LIMIT_4K_SHIFT,
				DA_32 | DA_LIMIT_4K | DA_DRW | privilege << 5);
		}

		p_proc->regs.cs = INDEX_LDT_C << 3 | SA_TIL | rpl;
		p_proc->regs.ds =
			p_proc->regs.es =
			p_proc->regs.fs =
			p_proc->regs.ss = INDEX_LDT_RW << 3 | SA_TIL | rpl;
		p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = eflags;

		/* p_proc->nr_tty       = 0; */

		p_proc->p_flags = 0;
		p_proc->p_msg = 0;
		p_proc->p_recvfrom = NO_TASK;
		p_proc->p_sendto = NO_TASK;
		p_proc->has_int_msg = 0;
		p_proc->q_sending = 0;
		p_proc->next_sending = 0;
		p_proc->pid = i;

		for (j = 0; j < NR_FILES; j++)
			p_proc->filp[j] = 0;

		p_proc->ticks = p_proc->priority = prio;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		// selector_ldt += 1 << 3;
	}

	/* proc_table[NR_TASKS + 0].nr_tty = 0; */
	/* proc_table[NR_TASKS + 1].nr_tty = 1; */
	/* proc_table[NR_TASKS + 2].nr_tty = 1; */

	k_reenter = 0;
	ticks = 0;

	p_proc_ready = proc_table;

	init_clock();
	init_keyboard();

	restart();

	while (1) {}
}


void nothing() {
}


void Init_test()
{
	char tty_name[] = "/dev_tty2";

	char rdbuf[128];


	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	printf("Init() is running ...\n");

	while (1) {
		int r = read(fd_stdin, rdbuf, 70);
		rdbuf[r] = 0;
		printf("Before fork()");
		int pid = fork();
		if (pid != 0) { /* parent process */
			printf("parent is running, child pid:%d\n", pid);
			spin("parent");
		}
		else {  /* child process */
			printf("child is running, pid:%d\n", getpid());
			spin("child");
		}
	}
}


/*****************************************************************************
 *                                get_ticks
 *****************************************************************************/
PUBLIC int get_ticks()
{
	MESSAGE msg;
	reset_msg(&msg);
	msg.type = GET_TICKS;
	send_recv(BOTH, TASK_SYS, &msg);
	return msg.RETVAL;
}


/*****************************************************************************
 *                                convert_to_absolute
 *                      将传入的路径和文件名组合成一个完整的绝对路径
 *****************************************************************************/
PUBLIC void convert_to_absolute(char* dest, char* path, char* file)
{
	int i = 0, j = 0;
	while (path[i] != 0)  // 写入路径
	{
		dest[j] = path[i];
		j++;
		i++;
	}
	i = 0;
	while (file[i] != 0)  // 写入文件名
	{
		dest[j] = file[i];
		j++;
		i++;
	}
	dest[j] = 0;  // 结束符
}


/*======================================================================*
							   TestA
 *======================================================================*/

 //1号终端
void TestA()
{
	MESSAGE msg;//消息

	char tty_name[] = "/dev_tty0";
	PUBLIC int timeset = 0;

	char rdbuf[256];
	char cmd[20];
	char filename1[128];
	char filename2[128];

	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	//  char filename[MAX_FILENAME_LEN+1] = "zsp01";
	const char bufw[80] = { 0 };
	//  const int rd_bytes = 3;
	//  char bufr[rd_bytes];

	sl();
	login();
	SetCurrentTime("00:00:00");
	clear();
	welcome();

	char current_dirr[512] = "/";  // 记录当前路径（其实路径字符长度上限为MAX_PATH）

	while (1) {
		if (process_running != TESTA)//当前进程不为进程A
		{
			//接受消息
			send_recv(RECEIVE, ANY, &msg);
			int src = msg.source;
			//处理消息
			switch (msg.type)
			{
			case MESSAGE_A://进程A接受到消息			
				msg.RETVAL = TRUE;
				char src_pro[20];//发送消息的进程
				if (src == TESTB)//发送消息的进程为进程B
				{
					strcpy(src_pro, "processB");
				}
				if (src == TESTC)//发送消息的进程为进程C
				{
					strcpy(src_pro, "processC");
				}
				printf("processA received message from %s:%s\n", src_pro, msg.text);
				send_recv(SEND, src, &msg);
				break;
			case PROCESS_A://切换到进程A
				break;
			default:
				panic("unknown msg type");
				break;
			}
		}

		if (process_running == TESTA)//当前进程为进程A
		{
			printf("[root@LiOS: %s]", current_dirr);  // 打印当前路径
			int r = read(fd_stdin, rdbuf, 512);
			rdbuf[r] = 0;

			// 解析命令
			int pos = 0;
			while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
			{
				cmd[pos] = rdbuf[pos];
				pos++;
			}
			cmd[pos] = 0;
			if (rdbuf[pos] != 0)  // 指令还未结束
			{
				pos++;
				int len = pos;
				while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取第一个文件名
				{
					filename1[pos - len] = rdbuf[pos];
					pos++;
				}
				filename1[pos - len] = 0;
			}
			if (rdbuf[pos] != 0)  // 指令还未结束
			{
				pos++;
				int len = pos;
				while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取第二个文件名
				{
					filename2[pos - len] = rdbuf[pos];
					pos++;
				}
				filename2[pos - len] = 0;
			}
			// printf("%s O %s O %s O", cmd, filename1, filename2);
			//show();
			if (strcmp(cmd, "process") == 0)
			{
				ShowProcess();
			}
			else if (strcmp(cmd, "calendar") == 0)
			{
				while (1)
				{
					printf("Please enter year and month! eg:2020 08 \n");
					int r = read(fd_stdin, rdbuf, 512);
					rdbuf[r] = 0;
					int pos = 0;
					while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)
					{
						cmd[pos] = rdbuf[pos];
						pos++;
					}
					cmd[pos] = 0;
					if (rdbuf[pos] != 0)
					{
						pos++;
						int len = pos;
						while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取第一个文件名
						{
							filename1[pos - len] = rdbuf[pos];
							pos++;
						}
						filename1[pos - len] = 0;
						int year, month;
						atoi(cmd, &year);
						atoi(filename1, &month);
						Calendar(year, month);
						break;
					}
					else
					{
						printf("wrong type of input! Please enter the correct month! eg:2020 08\n");
					}
				}

			}
			else if (strcmp(cmd, "time") == 0)
			{
				if (timeset == 0)
				{
					printf("Please set current time! eg: 18:20:00 \n");
					int r = read(fd_stdin, rdbuf, 512);
					rdbuf[r] = 0;

					// 解析命令
					int pos = 0;
					while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
					{
						cmd[pos] = rdbuf[pos];
						pos++;
					}
					cmd[pos] = 0;
					SetCurrentTime(cmd);
					timeset = 1;

				}
				else
				{
					printf("Enter 'gettime' to get current time; \nEnter a specific time point to change current time. eg:18:20:20 \n");
					int r = read(fd_stdin, rdbuf, 512);
					rdbuf[r] = 0;

					// 解析命令
					int pos = 0;
					while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
					{
						cmd[pos] = rdbuf[pos];
						pos++;
					}
					cmd[pos] = 0;
					if (strcmp(cmd, "gettime") == 0)
						GetCurrentTime();
					else
						SetCurrentTime(cmd);

				}
			}

			else if (strcmp(cmd, "messageB") == 0)//向进程B发送消息
			{
				printf("please input the message!\n");
				//读取文本
				char text_messageB[512];
				int r_messageB = read(fd_stdin, rdbuf, 512);
				rdbuf[r_messageB] = 0;
				int pos_messageB = 0;
				while (rdbuf[pos_messageB] != 0)
				{
					text_messageB[pos_messageB] = rdbuf[pos_messageB];
					pos_messageB++;
				}
				text_messageB[pos_messageB] = 0;

				if (messageB(text_messageB) == TRUE)//成功向进程B发送消息
				{
					printf("processA sent message to processB!\n");
				}
			}
			else if (strcmp(cmd, "messageC") == 0)//向进程C发送消息
			{
				printf("please input the message!\n");
				//读取文本
				char text_messageC[512];
				int r_messageC = read(fd_stdin, rdbuf, 512);
				rdbuf[r_messageC] = 0;
				int pos_messageC = 0;
				while (rdbuf[pos_messageC] != 0)
				{
					text_messageC[pos_messageC] = rdbuf[pos_messageC];
					pos_messageC++;
				}
				text_messageC[pos_messageC] = 0;

				if (messageC(text_messageC) == TRUE)//成功向进程C发送消息
				{
					printf("processA sent message to processC!\n");
				}
			}
			else if (strcmp(cmd, "processB") == 0)//切换到进程B
			{
				processB();
			}
			else if (strcmp(cmd, "processC") == 0)//切换到进程C
			{
				processC();
			}
			else if (strcmp(cmd, "ls") == 0)//显示当前文件夹下文件
			{
				ls(current_dirr);
			}
			else if (strcmp(cmd, "touch") == 0)//创建新文件
			{
				CreateFile(current_dirr, filename1);
			}
			else if (strcmp(cmd, "rm") == 0)  // 删除文件
			{
				DeleteFile(current_dirr, filename1);
			}
			else if (strcmp(cmd, "cat") == 0)  // 打印文件内容
			{
				ReadFile(current_dirr, filename1);
			}
			else if (strcmp(cmd, "vi") == 0)  // 写文件
			{
				WriteFile(current_dirr, filename1);
			}
			else if (strcmp(cmd, "mkdir") == 0)  // 创建目录
			{
				CreateDir(current_dirr, filename1);
			}
			else if (strcmp(cmd, "cd") == 0)//切换目录
			{
				GoDir(current_dirr, filename1);
			}
			else if (strcmp(cmd, "hide") == 0)//隐藏文件
			{
				HideFile(current_dirr, filename1, 0);
			}
			else if (strcmp(cmd, "show") == 0)//显示文件
			{
				HideFile(current_dirr, filename1, 1);
			}
			else if (strcmp(cmd, "game1") == 0)//game 1
			{
				MineSweeper(fd_stdin, fd_stdout);
				clear();
				welcome();
			}
			else if (strcmp(cmd, "game2") == 0)//game 2
			{
				boxPushing(fd_stdin, fd_stdout);
				clear();
				welcome();
			}
			else if (strcmp(cmd, "game3") == 0)//game 3
			{
				TTT(int fd_stdin, int fd_stdout)
				clear();
				welcome();
			}
			else if (strcmp(cmd, "game4") == 0)//game 4
			{
				start2048Game(fd_stdin, fd_stdout);
				clear();
				welcome();
			}
			else if (strcmp(cmd, "help") == 0)//帮助
			{
				help();
			}
			else if (strcmp(cmd, "game") == 0)//game list
			{
				clear();
				game();
			}
			else if (strcmp(cmd, "clear") == 0)//清屏
			{
				clear();
				welcome();
			}

			else
				printf("Command not found, please check!\n");
			// printf("rdbuf:      %s\n", rdbuf);
			// printf("cmd:        %s\n", cmd);
			// printf("filename1:  %s\n", filename1);
			// printf("filename2:  %s\n", filename2);
		}
	}

}

/*======================================================================*
							   TestB
 *======================================================================*/
 //二号终端
void TestB()
{
	MESSAGE msg;//消息

	//输入
	char tty_name[] = "/dev_tty1";
	char rdbuf[128];
	char cmd[8];
	char filename[120];
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	while (1)
	{
		if (process_running != TESTB)////当前进程不为进程B
		{
			//接受消息
			send_recv(RECEIVE, ANY, &msg);
			int src = msg.source;
			//处理消息
			switch (msg.type)
			{
			case MESSAGE_B://进程B接受到消息			
				msg.RETVAL = TRUE;
				char src_pro[20];//发送消息的进程
				if (src == TESTA)//发送消息的进程为进程A
				{
					strcpy(src_pro, "processA");
				}
				if (src == TESTC)//发送消息的进程为进程C
				{
					strcpy(src_pro, "processC");
				}
				printf("processB received message from %s:%s\n", src_pro, msg.text);
				send_recv(SEND, src, &msg);
				break;
			case PROCESS_B://切换到进程B
				break;
			default:
				panic("unknown msg type");
				break;
			}
		}
		if (process_running == TESTB)////当前进程为进程B
		{
			printf("[processB]");

			int r = read(fd_stdin, rdbuf, 512);
			rdbuf[r] = 0;
			// 解析命令
			int pos = 0;
			while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
			{
				cmd[pos] = rdbuf[pos];
				pos++;
			}
			cmd[pos] = 0;

			if (strcmp(cmd, "help") == 0)
			{

			}
			else if (strcmp(cmd, "messageA") == 0)//向进程A发送消息
			{
				printf("please input the message!\n");
				//读取文本
				char text_messageA[512];
				int r_messageA = read(fd_stdin, rdbuf, 512);
				rdbuf[r_messageA] = 0;
				int pos_messageA = 0;
				while (rdbuf[pos_messageA] != 0)
				{
					text_messageA[pos_messageA] = rdbuf[pos_messageA];
					pos_messageA++;
				}
				text_messageA[pos_messageA] = 0;

				if (messageA(text_messageA) == TRUE)//成功向进程A发送消息
				{
					printf("processB sent message to processA!\n");
				}
			}
			else if (strcmp(cmd, "messageC") == 0)//向进程C发送消息
			{
				printf("please input the message!\n");
				//读取文本
				char text_messageC[512];
				int r_messageC = read(fd_stdin, rdbuf, 512);
				rdbuf[r_messageC] = 0;
				int pos_messageC = 0;
				while (rdbuf[pos_messageC] != 0)
				{
					text_messageC[pos_messageC] = rdbuf[pos_messageC];
					pos_messageC++;
				}
				text_messageC[pos_messageC] = 0;

				if (messageC(text_messageC) == TRUE)//成功向进程C发送消息
				{
					printf("processB sent message to processC!\n");
				}
			}
			else if (strcmp(cmd, "processA") == 0)//切换到进程A
			{
				processA();
			}
			else if (strcmp(cmd, "processC") == 0)//切换到进程C
			{
				processC();
			}
			else
			{
				printf("Command not found, please check!\n");
			}
		}
	}
}

void TestC()
{
	MESSAGE msg;//消息

	//输入
	char tty_name[] = "/dev_tty2";
	char rdbuf[128];
	char cmd[8];
	char filename[120];
	int fd_stdin = open(tty_name, O_RDWR);
	assert(fd_stdin == 0);
	int fd_stdout = open(tty_name, O_RDWR);
	assert(fd_stdout == 1);

	while (1)
	{
		if (process_running != TESTC)////当前进程不为进程C
		{
			//接受消息
			send_recv(RECEIVE, ANY, &msg);
			int src = msg.source;
			//处理消息
			switch (msg.type)
			{
			case MESSAGE_C://进程C接受到消息			
				msg.RETVAL = TRUE;
				char src_pro[20];//发送消息的进程
				if (src == TESTA)//发送消息的进程为进程A
				{
					strcpy(src_pro, "processA");
				}
				if (src == TESTB)//发送消息的进程为进程B
				{
					strcpy(src_pro, "processB");
				}
				printf("processC received message from %s:%s\n", src_pro, msg.text);
				send_recv(SEND, src, &msg);
				break;
			case PROCESS_C://切换到进程C
				break;
			default:
				panic("unknown msg type");
				break;
			}
		}
		if (process_running == TESTC)////当前进程为进程C
		{
			printf("[processC]");

			int r = read(fd_stdin, rdbuf, 512);
			rdbuf[r] = 0;
			// 解析命令
			int pos = 0;
			while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
			{
				cmd[pos] = rdbuf[pos];
				pos++;
			}
			cmd[pos] = 0;

			if (strcmp(cmd, "help") == 0)
			{

			}
			else if (strcmp(cmd, "messageA") == 0)//向进程A发送消息
			{
				printf("please input the message!\n");
				//读取文本
				char text_messageA[512];
				int r_messageA = read(fd_stdin, rdbuf, 512);
				rdbuf[r_messageA] = 0;
				int pos_messageA = 0;
				while (rdbuf[pos_messageA] != 0)
				{
					text_messageA[pos_messageA] = rdbuf[pos_messageA];
					pos_messageA++;
				}
				text_messageA[pos_messageA] = 0;

				if (messageA(text_messageA) == TRUE)//成功向进程A发送消息
				{
					printf("processC sent message to processA!\n");
				}
			}
			else if (strcmp(cmd, "messageB") == 0)//向进程B发送消息
			{
				printf("please input the message!\n");
				//读取文本
				char text_messageB[512];
				int r_messageB = read(fd_stdin, rdbuf, 512);
				rdbuf[r_messageB] = 0;
				int pos_messageB = 0;
				while (rdbuf[pos_messageB] != 0)
				{
					text_messageB[pos_messageB] = rdbuf[pos_messageB];
					pos_messageB++;
				}
				text_messageB[pos_messageB] = 0;

				if (messageB(text_messageB) == TRUE)//成功向进程B发送消息
				{
					printf("processC sent message to processB!\n");
				}
			}
			else if (strcmp(cmd, "processA") == 0)//切换到进程A
			{
				processA();
			}
			else if (strcmp(cmd, "processB") == 0)//切换到进程B
			{
				processB();
			}
			else
			{
				printf("Command not found, please check!\n");
			}
		}
	}
}

void TestD()
{
	TimeRunning();
	spin("TestD");
}

/*****************************************************************************
 *                                panic
 *****************************************************************************/
PUBLIC void panic(const char* fmt, ...)
{
	int i;
	char buf[256];

	/* 4 is the size of fmt in the stack */
	va_list arg = (va_list)((char*)&fmt + 4);

	i = vsprintf(buf, fmt, arg);

	printl("%c !!panic!! %s", MAG_CH_PANIC, buf);

	/* should never arrive here */
	__asm__ __volatile__("ud2");
}

void clear()
{
	clear_screen(0, console_table[current_console].cursor);
	console_table[current_console].crtc_start = 0;
	console_table[current_console].cursor = 0;
}

void welcome()
{
	printf("                        ==================================\n");
	printf("                                     LiOS v1.0.0         \n");
	printf("                                 Kernel on Orange's \n\n");
	printf("                                     Welcome !\n");
	printf("                        ==================================\n");
}

void help()
{
	printf("==========================LiOS help info====================================\n");
	printf("Command List              :\n");
	printf("1. help                   : Show this help message\n");
	printf("2. process                : A process manage,show you all process-info here\n");
	printf("3. clear                  : Clear the screen\n");
	printf("4. ls                     : List all files in current directory\n");
	printf("5. touch     [filename]   : Create a new file in current directory\n");
	printf("6. rm        [filename]   : Delete a file in current directory\n");
	printf("7. cat       [filename]   : Print the content of a file in current directory\n");
	printf("8. vi        [filename]   : Write new content at the end of the file\n");
	printf("9. mkdir     [dirname]    : Create a new directory in current directory\n");
	printf("10. cd       [dirname]    : Go to a directory in current directory\n");
	printf("11. hide     [filename]   : Hide a file \n");
	printf("12. show     [filename]   : Unhide a file \n");
	printf("13. game                  : Show the game list\n");
	printf("14. time                  : Show the system current time\n");
	printf("15. messageA/B/C          : Send message to the terminal A or B or C\n");
	printf("16. processA/B/C          : Switch to the terminal A or B or C\n");
	printf("17. calendar              : A calendar application\n");
	printf("18. time                  : Set the system time\n");
	printf("==============================================================================\n");
}

void game()
{
	printf("==========================LiOS game info====================================\n");
	printf("Command List              :\n");
	printf("1. game1                  : MineSweeper game\n");
	printf("2. game2                  : Box pushing game\n");
	printf("3. game3                  : Tic-Tac-Toe game\n");
	printf("4. game4                  :     2048    game\n");
	printf("==============================================================================\n");
}

void login()
{
	char tty_name[] = "/dev_tty0";

	char rdbuf[256];
	char cmd[20];

	int fd_stdin = open(tty_name, O_RDWR);

	char password[8] = "lilinb";

	while (1)
	{
		int delay_time = 10000;
		printf("Please enter your password: \n");
		int r = read(fd_stdin, rdbuf, 512);
		rdbuf[r] = 0;

		// 解析命令
		int pos = 0;
		while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
		{
			cmd[pos] = rdbuf[pos];
			pos++;
		}
		cmd[pos] = 0;

		if (strcmp(cmd, "lilinb") == 0)
			break;
		else
		{
			printf("Wrong Password! \n");
			milli_delay(delay_time);
			delay_time *= 5;
		}
	}

}

