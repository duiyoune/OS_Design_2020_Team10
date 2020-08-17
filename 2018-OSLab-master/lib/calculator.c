/*************************************************************************//**
 *****************************************************************************
 * @file   mkdir.c
 * @brief  mkdir
 * @author zwz
 * @date   2018
 *****************************************************************************
 *****************************************************************************/

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

#define MAX_SIZE 1024                       /*数组长度*/


int insert_operand(int *operand , int * top_num ,int num)           /*数据压入数据栈*/
{
    (*top_num) ++;
    operand[*top_num] = num;                    /*保存数据*/
    
    return 0;                           /*正常退出*/
}


int insert_oper (char * oper , int *top_oper , char ch)             /*操作符压入符号栈*/
{
    (*top_oper)++;
    oper[*top_oper] = ch;                       /*保存操作符*/
    return 0;                           /*正常退出*/
}
     
int compare(char *oper , int *top_oper , char ch)                   /*比较操作服优先级*/
{
    if((oper[*top_oper] == '-' || oper[*top_oper] == '+')           /*判断当前优先级是否比栈顶操作符优先级高*/
    && (ch == '*' || ch == '/'))
    {
        return 0;                      /*操作符压入栈*/
    }
    else if(*top_oper == -1 || ch == '('|| (oper[*top_oper] == '(' && ch != ')'))  /*判断操作符栈是否为空；栈顶操作   符是否为'('*/
    {
        return 0;                       /*操作符压入栈*/
    }
    else if (oper[*top_oper] =='(' && ch == ')' )       /*判断括号内的表达式是否计算完毕*/
    {
        (*top_oper)--;
        return 1;                       /*对（）进行处理*/
    }
    else
    {
        return -1;                                          /*进行操作符的运算*/
    }
}


int deal_date(int *operand ,char *oper ,int *top_num, int *top_oper)    /*进行数据运算*/
{
    int num_1 = operand[*top_num];              /*取出数据栈中两个数据*/
    int num_2 = operand[*top_num - 1];
    
    int value = 0;
    
    if(oper[*top_oper] == '+')                  /*加法操作*/
    {
        value = num_1 + num_2;
    }
    
    else if(oper[*top_oper] == '-')             /*减法操作*/
    {
        value = num_2 - num_1;
    }
    
    else if(oper[*top_oper] == '*')             /*乘法操作*/
    {
        value = num_2 * num_1;
    }
    
    else if(oper[*top_oper] == '/')             /*除法操作*/
    {
        value = num_2 / num_1;
    }
    
    (*top_num) --;                              /*将数据栈顶下移一位*/
    operand[*top_num] = value;                  /*将得到的值压入数据栈*/
    (*top_oper) --;                             /*将操作符栈顶下移一位*/
}

PUBLIC int Calculator()
{
    int operand[MAX_SIZE] = {0};                /*数据栈，初始化*/
    int  top_num = -1;
    
    char oper[MAX_SIZE] = {0};                  /*操作符栈，初始化*/
    int top_oper = -1;
    char str[100];               /*获取表达式(不带＝)*/
    
    printf("请输入表达式！");
    char tty_name[] = "/dev_tty0";

    char rdbuf[256];
    int fd_stdin  = open(tty_name, O_RDWR);


    int r = read(fd_stdin, rdbuf, 512);
        rdbuf[r] = 0;

    // 解析命令
    int pos = 0;
    while (rdbuf[pos] != ' ' && rdbuf[pos] != 0)  // 读取指令
    {
        str[pos] = rdbuf[pos];
        pos++;
    }
    str[pos] = 0;
    char* temp;
    char dest[MAX_SIZE];
    
    int num = 0;
    int i = 0;
    int ind=0;
    while(str[ind] != '\0')
    {
        temp = dest;
        while(str[ind] >= '0' && str[ind] <= '9')           /*判断是否是数据*/
        {
            *temp = str[ind];
            ind ++;
            temp ++;
        }                               /*遇到符号退出*/
       
        if(str[ind] != '(' && *(temp - 1) != '\0')      /*判断符号是否为'('*/
        {
            *temp = '\0';
            atoi(dest, &num);               /*将字符串转为数字*/
            insert_operand(operand, &top_num,num);      /*将数据压入数据栈*/
        }

        while(1)
        {
            i = compare(oper,&top_oper,str[ind]);      /*判断操作符优先级*/
            if(i == 0)
            {
                insert_oper(oper,&top_oper,str[ind]);   /*压入操作符*/
                break;
            }
   
            else if(i == 1)                         /*判断括号内的表达式是否结束*/
            {
                ind ++;
            }
   
            else if(i == -1)                        /*进行数据处理*/
            {
                deal_date(operand,oper,&top_num,&top_oper);
            }
        }
        ind ++;                 /*指向表达式下一个字符*/
    }
    
    printf("num = %d\n",operand[0]);        /*输出结果*/
    return 0;                       /*正常退出*/
}
