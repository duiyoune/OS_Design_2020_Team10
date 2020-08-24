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
#include "keyboard.h"
#include "proto.h"
#include"file.h"

int fileNumber = 0;

PUBLIC void HideFile(char* path, char* file, int op)
{
    char pathName[MAX_PATH];
    char fileName[20];

    int inode = search_file(file);
    if (inode == 0)
    {
        printf("File %s does not exist!\n", file);
        return;
    }
    if (op == 1 && blocks[inode].hide ==1 )
    {
        blocks[inode].hide = 0;
    }
    else if (op == 0 && blocks[inode].hide == 0)
    {
        blocks[inode].hide = 1;
    }
    else
    {
        printf("%s file %s failed!\n", op == 1 ? "Show" : "Hide", file);
        return;
    }

    printf("%s file %s success!\n", op == 1 ? "Show" : "Hide", file);
}

void LockFile(char* path, char* file,int fd_stdin)
{
    char pathName[MAX_PATH];
    char fileName[20];

    int inode = search_file(file);
    if (inode == 0)
    {
        printf("File %s does not exist!\n", file);
        return;
    }
    blocks[inode].lock = 1;

    char szCmd[17] = { 0 };
    printf("Please set the password(no more than 16):");
    int n = read(fd_stdin, szCmd, 17);
    szCmd[n] = 0;

    strcpy(blocks[inode].pwd, szCmd);
    printf("%s\n", blocks[inode].pwd);

    printf("Lock file %s success!\n", file);
}

void UnlockFile(char* path, char* file, int fd_stdin)
{
    char pathName[MAX_PATH];
    char fileName[20];

    int inode = search_file(file);
    if (inode == 0)
    {
        printf("File %s does not exist!\n", file);
        return;
    }

    char szCmd[17] = { 0 };
    printf("Please input the current password:");
    int n = read(fd_stdin, szCmd, 17);
    szCmd[n] = 0;

    if (strcmp(blocks[inode].pwd, szCmd) == 0)
    {
        blocks[inode].lock = 0;
        printf("Unlock file %s success!\n", file);
    }
    else 
    {
        printf("Wrong password!\n");

    }
}

PUBLIC void CreateFile(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);

    int fd = open(absoPath, O_CREAT | O_RDWR);

    if (fd == -1)
    {
        printf("Failed to create a new file with name %s\n", file);
        return;
    }

    char buf[1] = { 0 };
    write(fd, buf, 1);
    printf("File created: %s (fd %d)\n", file, fd);
    close(fd);
}

PUBLIC void DeleteFile(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int m = unlink(absoPath);
    if (m == 0)
        printf("%s deleted!\n", file);
    else
        printf("Failed to delete %s!\n", file);
}

void ReadFile(char* path, char* file,int fd_stdin)
{
    int inode = search_file(file);
    if (inode == 0)
    {
        printf("File %s does not exist!\n", file);
        return;
    }

    if (blocks[inode].lock == 1)//文件被上锁
    {
        char szCmd[17] = { 0 };
        printf("Please input the current password:");
        int n = read(fd_stdin, szCmd, 17);
        szCmd[n] = 0;

        if (strcmp(blocks[inode].pwd, szCmd) != 0)
        {
            printf("Wrong password!\n");
            return;
        }
    }

    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int fd = open(absoPath, O_RDWR);
    if (fd == -1)
    {
        printf("Failed to open %s!\n", file);
        return;
    }

    char buf[4096];
    int n = read(fd, buf, 4096);
    if (n == -1)  // 读取文件内容失败
    {
        printf("An error has occured in reading the file!\n");
        close(fd);
        return;
    }

    printf("%s\n", buf);
    close(fd);
}

PUBLIC void WriteFile(char* path, char* file,int fd_stdin)
{
    int inode = search_file(file);
    if (inode == 0)
    {
        printf("File %s does not exist!\n", file);
        return;
    }

    if (blocks[inode].lock == 1)//文件被上锁
    {
        char szCmd[17] = { 0 };
        printf("Please input the current password:");
        int n = read(fd_stdin, szCmd, 17);
        szCmd[n] = 0;

        if (strcmp(blocks[inode].pwd, szCmd) != 0)
        {
            printf("Wrong password!\n");
            return;
        }
    }

    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int fd = open(absoPath, O_RDWR);
    if (fd == -1)
    {
        printf("Failed to open %s!\n", file);
        return;
    }

    //char tty_name[] = "/dev_tty0";
    //int fd_stdin = open(tty_name, O_RDWR);
    if (fd_stdin == -1)
    {
        printf("An error has occured in writing the file!\n");
        return;
    }
    char writeBuf[4096];  // 写缓冲区
    int endPos = read(fd_stdin, writeBuf, 4096);
    writeBuf[endPos] = 0;
    write(fd, writeBuf, endPos + 1);  // 结束符也应写入
    close(fd);
}

PUBLIC void CreateDir(char* path, char* file)
{
    char absoPath[512];
    convert_to_absolute(absoPath, path, file);
    int fd = open(absoPath, O_RDWR);

    if (fd != -1)
    {
        printf("Failed to create a new directory with name %s\n", file);
        return;
    }
    mkdir(absoPath);
}

PUBLIC void GoDir(char* path, char* file)
{
    int flag = 0;  // 判断是进入下一级目录还是返回上一级目录
    char newPath[512] = { 0 };
    if (file[0] == '.' && file[1] == '.')  // cd ..返回上一级目录
    {
        flag = 1;
        int pos_path = 0;
        int pos_new = 0;
        int i = 0;
        char temp[128] = { 0 };  // 用于存放某一级目录的名称
        while (path[pos_path] != 0)
        {
            if (path[pos_path] == '/')
            {
                pos_path++;
                if (path[pos_path] == 0)  // 已到达结尾
                    break;
                else
                {
                    temp[i] = '/';
                    temp[i + 1] = 0;
                    i = 0;
                    while (temp[i] != 0)
                    {
                        newPath[pos_new] = temp[i];
                        temp[i] = 0;  // 抹掉
                        pos_new++;
                        i++;
                    }
                    i = 0;
                }
            }
            else
            {
                temp[i] = path[pos_path];
                i++;
                pos_path++;
            }
        }
    }
    char absoPath[512];
    char temp[512];
    int pos = 0;
    while (file[pos] != 0)
    {
        temp[pos] = file[pos];
        pos++;
    }
    temp[pos] = '/';
    temp[pos + 1] = 0;
    if (flag == 1)  // 返回上一级目录
    {
        temp[0] = 0;
        convert_to_absolute(absoPath, newPath, temp);
    }
    else  // 进入下一级目录
        convert_to_absolute(absoPath, path, temp);
    int fd = open(absoPath, O_RDWR);
    if (fd == -1)
        printf("%s is not a directory!\n", absoPath);
    else
        memcpy(path, absoPath, 512);
}

void toStr3(char* temp, int i) {
    if (i / 100 < 0)
        temp[0] = (char)48;
    else
        temp[0] = (char)(i / 100 + 48);
    if ((i % 100) / 10 < 0)
        temp[1] = '0';
    else
        temp[1] = (char)((i % 100) / 10 + 48);
    temp[2] = (char)(i % 10 + 48);
}

int toInt(char* temp) {
    int result = 0;
    for (int i = 0; i < 3; i++)
        result = result * 10 + (int)temp[i] - 48;
    return result;
}

void InitBlock(int inode_nr) {
    blocks[inode_nr].inode_nr = inode_nr;
    blocks[inode_nr].hide = 0;
    blocks[inode_nr].lock = 0;
}

void WriteDisk(int len) {

    char temp[MAX_FILE_NUM * 30];
    int i = 0;
    temp[i] = '^';
    i++;
    toStr3(temp + i, fileNumber);
    i += 3;
    temp[i] = '^';
    i++;
    for (int j = 0; j < MAX_FILE_NUM; j++) {
        if (blocks[j].inode_nr >0) {
            toStr3(temp + i, blocks[j].inode_nr);//inode num
            i = i + 3;
            temp[i] = '^';
            i++;

            toStr3(temp + i, blocks[j].hide);//hide flag
            i = i + 1;
            temp[i] = '^';
            i++;

            toStr3(temp + i, blocks[j].lock);//block flag
            i = i + 1;
            temp[i] = '^';
            i++;

            for (int h = 0; h < strlen(blocks[j].pwd); h++) {//pwd
                temp[i + h] = blocks[j].pwd[h];
                if (blocks[j].pwd[h] == '^')
                    temp[i + h] = (char)1;
            }
            i = i + strlen(blocks[j].pwd);
            temp[i] = '^';
            i++;
        }
    }
    
    printl("%s\n", temp);

    int fd = 0;
    int n1 = 0;
    fd = open("ss", O_RDWR);
    assert(fd != -1);
    n1 = write(fd, temp, strlen(temp));
    assert(n1 == strlen(temp));
    close(fd);
}

int ReadDisk() {
    char bufr[1000];
    int fd = 0;
    int n1 = 0;
    fd = open("ss", O_RDWR);
    assert(fd != -1);
    n1 = read(fd, bufr, 1000);
    bufr[n1] = 0;
    int r = 1;
    fileNumber = toInt(bufr + r);
    r = r + 4;
    for (int i = 0; i < fileNumber; i++) {
        int ID = toInt(bufr + r);

        blocks[ID].inode_nr = ID;
        r = r + 4;
        
        blocks[ID].hide = toInt(bufr + r);
        r = r + 2;

        blocks[ID].lock = toInt(bufr + r);
        r = r + 2;

        if (blocks[ID].lock == 1)
        {
            for (int i = 0; i < MAX_PWD_LENGTH; i++) {
                if (bufr[r] == '^')
                    break;
                else if (bufr[r] == (char)1)
                    blocks[ID].pwd[i] = '^';
                else
                    blocks[ID].pwd[i] = bufr[r];
                r++;
            }
        }
        r++;
    }
    return n1;
}

void DeleteBlock(int ID) {
    blocks[ID].inode_nr = -2;
    blocks[ID].hide = -1;
    blocks[ID].lock = -1;
    for (int i = 0; i < MAX_FILE_NUM; i++)
        blocks[ID].pwd[i] = '\0';
}

