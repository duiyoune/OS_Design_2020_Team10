#pragma once
#ifndef __FILE_H__
#define __FILE_H__

void HideFile(char* path, char* file, int op);
void CreateFile(char* path, char* file);
void DeleteFile(char* path, char* file);
void ReadFile(char* path, char* file);
void WriteFile(char* path, char* file);
void CreateDir(char* path, char* file);
void GoDir(char* path, char* file);

#endif // !1
