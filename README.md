# OS_Design_2020_Team10

## Mission1:熟悉bochs
1.配置环境  
2.阅读ORANGES1-5章并跟随书完成实验  

## Deadline：8.1

---------

  

## Mission2:分工看6-10章

###任务分配：  

|  成员   | 任务 |
|  :----:  | :----:  |
| 进程  | 霍定镭 |
| 输入/输出  | 陈子杰 |
| 进程间通信  | 黎力 |
| 文件系统  | 黄伟杰 |
| 内存管理  | 马天放 |

## Deadline：8.11


------

## Mission3:开始创新

### 环境配置：
1.根目录下:
```
cd /mnt
sudo mkdir floppy
ls(显示 floppy即可）
```
2.根目录下:
``` 
sudo apt install libc6-dev-i386
```
3.将github上的2018-OSLab-master复制进ubuntu
4.在OS-master文件夹位置创建Terminal
5.
```
make image
bochs -f bochsrc
```

6.修改时，如果新建xxx.c文件，需要在makefile中加入相应文件名。
7.入口文件在TestA()

---------
### 任务分配


|  成员   | 任务 |
|  :----:  | :----:  |
| 进程&进程间通信  | 霍定镭 |
| 开机动画+计算器/日历  | 陈子杰 |
| 树状文件系统  | 黎力 |
| 小游戏  | 黄伟杰 |
| help && 命令跳转  | 马天放 |


### 修改文件目录
开机动画	/lib/ls.c  
help &&跳转： /kernel/main.c 200行  
树状文件系统： /kernel/main.c 500行  
小游戏：	/game/TTT.c	/include/2048Game.h  
进程： /kernel/main.c 0-150行	500行  
进程间通信：TestA &TestB  


## Deadline：8.19
