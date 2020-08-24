#pragma once
#ifndef __FILE_H__
#define __FILE_H__

#define MAX_PWD_LENGTH 16
#define MAX_FILE_NUM 64

struct Block {
	int inode_nr;
	int hide;
	int lock;
	char pwd[MAX_PWD_LENGTH];
};

PUBLIC struct Block blocks[MAX_FILE_NUM];

void HideFile(char* path, char* file, int op);
void CreateFile(char* path, char* file);
void DeleteFile(char* path, char* file);
void ReadFile(char* path, char* file, int fd_stdin);
void WriteFile(char* path, char* file, int fd_stdin);
void CreateDir(char* path, char* file);
void GoDir(char* path, char* file);
void toStr3(char* temp, int i);
int toInt(char* temp);
void InitBlock(int inode_nr);
void WriteDisk(int len);
int ReadDisk();
void DeleteBlock(int ID);
void LockFile(char* path, char* file, int fd_stdin);
void UnlockFile(char* path, char* file, int fd_stdin);

#endif // !1
