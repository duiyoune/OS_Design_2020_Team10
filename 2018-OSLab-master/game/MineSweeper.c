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

#define ROW 8+2  //雷区大小
#define COL 8+2
#define MINES 8  //雷的数量
#define DEBUG 0  //测试开关

void initgame(char mine[ROW][COL], char show[ROW][COL], int x, int y);
void display(char arr[ROW][COL], int x, int y);
int  play(char mine[ROW][COL], char show[ROW][COL], int x, int y, int fd_stdin);
int checkmine(char mine[ROW][COL], char show[ROW][COL], int i, int j);
void aroundmines(char mine[ROW][COL], char show[ROW][COL], int i, int j);

void meun()//打印菜单
{
	clear();
	printf("     Welcome to mines      \n");
	printf("---------------------------\n");
	printf("-----  1.Start game.   ----\n");
	printf("-----  0.Exit game.    ----\n");
	printf("---------------------------\n");
}

void game(int fd_stdin)//开始游戏
{
	char mine[ROW][COL];
	char show[ROW][COL];
	char ret = 0;

	initgame(mine, show, ROW, COL);

#if DEBUG
	display(mine, ROW, COL);
#endif
	display(show, ROW, COL);
	while (1)
	{
		ret = play(mine, show, ROW, COL, fd_stdin);
		if ('w' == ret)
		{
			display(mine, ROW, COL);
			printf("You win!\n");
			break;
		}
		if ('*' == ret)
		{
			display(mine, ROW, COL);
			printf("You lose!\n");
			break;
		}
#if DEBUG
		display(mine, ROW, COL);
#endif
		display(show, ROW, COL);
	}
}

PUBLIC int MineSweeper(int fd_stdin, int fd_stdout)
{

	int input = 0;
	do
	{
		meun();
		printf("Please select >: ");
		char buf[10] = { 0 };
		read(fd_stdin, buf, 2);
		char input = buf[0];
		switch (input)
		{
		case '1':
			game(fd_stdin);
			break;
		case '0':
			printf("Exit...\n");
			break;
		default:
			printf("Error select! \n");
		}
	} while (input);

	return 0;
}



void initgame(char mine[ROW][COL], char show[ROW][COL], int x, int y)// 初始化并布雷
{
	int i = 0;//坐标
	int j = 0;
	int k = 0;
	memset(mine, ' ', x * y * sizeof(char));
	memset(show, ' ', x * y * sizeof(char));
	/*for (k = 0; k < MINES; k++)
	{
		while (1)
		{
			i = rand() % (x - 2) + 1;//随机生成雷
			j = rand() % (x - 2) + 1;
			if (' ' == mine[i][j])
			{
				mine[i][j] = '*';
				break;
			}
		}
	}*/
	mine[2][3] = '*'; mine[3][3] = '*'; mine[6][6] = '*'; mine[7][5] = '*';
	mine[8][4] = '*'; mine[8][2] = '*'; mine[7][1] = '*'; mine[6][1] = '*';
}

int checkmine(char mine[ROW][COL], char show[ROW][COL], int i, int j)//遍历周围雷的个数
{
	int m = 0;
	int n = 0;
	int nummines = 0;
	for (m = i - 1; m <= i + 1; m++)
	{
		for (n = j - 1; n <= j + 1; n++)
			if ('*' == mine[m][n])nummines++;
	}
	show[i][j] = nummines + '0';
	return nummines;
}

void aroundmines(char mine[ROW][COL], char show[ROW][COL], int i, int j)//遍历一圈的雷的个数
{
	checkmine(mine, show, i - 1, j - 1);
	checkmine(mine, show, i - 1, j);
	checkmine(mine, show, i - 1, j + 1);
	checkmine(mine, show, i, j - 1);
	checkmine(mine, show, i, j + 1);
	checkmine(mine, show, i + 1, j - 1);
	checkmine(mine, show, i + 1, j);
	checkmine(mine, show, i + 1, j + 1);
}

void display(char arr[ROW][COL], int x, int y)//打印游戏界面
{
	int i = 0;
	int j = 0;
	printf("  ");
	for (i = 1; i < x - 1; i++)
	{
		printf("%4d", i);
	}
	printf("\n");
	printf("   ");
	printf("|");
	for (j = 1; j < y - 1; j++)
	{
		printf("---|");
	}
	printf("\n");
	for (i = 1; i < x - 1; i++)
	{
		printf("%2d ", i);
		printf("|");
		for (j = 1; j < y - 1; j++)
		{
			printf(" %c |", arr[i][j]);
		}
		printf("\n");
		printf("   ");
		printf("|");
		for (j = 1; j < y - 1; j++)
		{
			printf("---|", arr[i][j]);
		}
		printf("\n");
	}
}

int play(char mine[ROW][COL], char show[ROW][COL], int x, int y, int fd_stdin)//玩家玩
{
	int i = 0;//坐标变量
	int j = 0;
	int m = 0;
	int n = 0;
	int ret = 0;
	static int pos1 = 0;
	static int pos2 = 0;
	int count = 0;//统计是否排完雷的变量
	while (1)
	{
		char szCmd[80] = { 0 };
		printf("Please Input The Line Position where you search (x): ");
		n = read(fd_stdin, szCmd, 80);
		szCmd[1] = 0;
		atoi(szCmd, &i);
		printf("Please Input The Column Position where you search (y): ");
		n = read(fd_stdin, szCmd, 80);
		szCmd[1] = 0;
		atoi(szCmd, &j);

		if (i >= 1 && i < x - 1 && j >= 1 && j < y - 1)//位置合法性检测
		{
			if ('*' != mine[i][j])//判断是否为雷------------非雷
			{

				ret = checkmine(mine, show, i, j);     //遍历周围雷的个数

				if (0 == ret && 0 == pos2)
				{
					//pos2++;
					aroundmines(mine, show, i, j);
				}
				//pos2++;
				for (i = 1; i <= ROW - 2; i++)//统计是否已经排完雷
				{
					for (j = 1; j <= COL - 2; j++)
					{
						if (' ' == show[i][j])count++;
					}
				}
				if (MINES == count)return 'w';//排完
			}
			else //return '*';//是雷-----(移走)直接炸死
			{
				return '*';
			}
			pos1++;
			return 0;
		}
		else printf("Error input!\n");//位置不合法
	}
	return 0;
}