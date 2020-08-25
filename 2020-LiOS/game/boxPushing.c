
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


int map[9][11] = {

	{0,1,1,1,1,1,1,1,1,1,0},

	{0,1,0,0,0,1,0,0,0,1,0},

	{0,1,0,3,3,3,3,3,0,1,0},

	{0,1,0,3,0,3,0,3,0,1,1},

	{0,1,0,0,0,2,0,0,3,0,1},

	{1,1,0,1,1,1,1,0,3,0,1},

	{1,0,4,4,4,4,4,1,0,0,1},

	{1,0,4,4,4,4,4,0,0,1,1},

	{1,1,1,1,1,1,1,1,1,1,0}

};

int drawmain();

int tuidong(int fd_stdin);

int winshu();

int quit = 0;

int boxPushing(int fd_stdin,int fd_stdout)
{    	
	while (1)

	{

		clear();

		drawmain();

		tuidong(fd_stdin);

		if (quit)
		{
			break;
		}

	}
	printf("GameOver \n");
//	printf("shuchu \n");

	return 0;

}

int drawmain()

{

	int i, j;

	winshu();

	for (i = 0; i < 9; i++)

	{

		for (j = 0; j < 11; j++)

		{

			switch (map[i][j])

			{

			case 0:

				printf("  "); 

				break;

			case 1:

				printf("# "); 

				break;

			case 2:

				printf("P ");

				break;

			case 3:

				printf("* "); 

				break;

			case 4:

				printf("O "); 

				break;

			case 6:

				printf("P ");

				break;

			case 7:

				printf("@ ");

				break;

			}

		}

		printf("\n");

	}
	return 0;
}


int tuidong(int fd_stdin)

{

	int count, caw; 

	for (int i = 0; i < 9; i++)

	{

		for (int j = 0; j < 11; j++)

		{

			if (map[i][j] == 2 || map[i][j] == 6)

			{

				count = i;

				caw = j;

			}

		}

	}

		printf("If you want to quit ths game, please input 'Q',\n");
		printf("Or input your next step(use W,A,S and D on your keyboard to move the character):\n");
		
		char tui;
		char szCmd[80]={0};
    		read(fd_stdin,szCmd,80);
    		tui=szCmd[0];
		
		switch (tui)
		{
		case 'Q':
		case 'q':
			quit = 1;
			break;

		case 'W':
		case 'w':
		case 72:


			if (map[count - 1][caw] == 0 || map[count - 1][caw] == 4)

			{

				map[count][caw] -= 2;

				map[count - 1][caw] += 2;

			}

			else if (map[count - 1][caw] == 3 || map[count - 1][caw] == 7)

			{

				if (map[count - 2][caw] == 0 || map[count - 2][caw] == 4)

				{

					map[count][caw] -= 2;

					map[count - 1][caw] -= 1;

					map[count - 2][caw] += 3;

				}

			}

			break;


		case 'S':
		case 's':
		case 80:

			if (map[count + 1][caw] == 0 || map[count + 1][caw] == 4)

			{

				map[count][caw] -= 2;

				map[count + 1][caw] += 2;

			}



			else if (map[count + 2][caw] == 0 || map[count + 2][caw] == 4)

			{

				if (map[count + 1][caw] == 3 || map[count + 1][caw] == 7)

				{

					map[count][caw] -= 2;

					map[count + 1][caw] -= 1;

					map[count + 2][caw] += 3;

				}

			}

			break;


		case 'A':
		case 'a':
		case 75:

			if (map[count][caw - 1] == 0 || map[count][caw - 1] == 4)

			{

				map[count][caw] -= 2;

				map[count][caw - 1] += 2;

			}



			else if (map[count][caw - 2] == 0 || map[count][caw - 2] == 4)

			{

				if (map[count][caw - 1] == 3 || map[count][caw - 1] == 7)

				{

					map[count][caw] -= 2;

					map[count][caw - 1] -= 1;

					map[count][caw - 2] += 3;

				}

			}

			break;


		case 'D':
		case 'd':
		case 77:

			if (map[count][caw + 1] == 0 || map[count][caw + 1] == 4)

			{

				map[count][caw] -= 2;

				map[count][caw + 1] += 2;

			}



			else if (map[count][caw + 2] == 0 || map[count][caw + 2] == 4)

			{

				if (map[count][caw + 1] == 3 || map[count][caw + 1] == 7)

				{

					map[count][caw] -= 2;

					map[count][caw + 1] -= 1;

					map[count][caw + 2] += 3;

				}

			}

			break;

	}

	return 0;
}

int winshu()

{

	int k = 0;

	for (int i = 0; i < 9; i++)

	{

		for (int j = 0; j < 11; j++)

		{

			if (map[i][j] == 3)

				k++;

		}

	}

	if (k == 0)

		printf("Congratulations! You win! \n");
	return 0;
}


