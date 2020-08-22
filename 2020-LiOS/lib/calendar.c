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

int f(int year, int month)
{
    //如果月份<3，则f(年，月)＝年－1；否则，f(年，月)＝年
    if (month < 3) return year - 1;
    else return year;
}

int g(int month)
{
    //如果月份<3，g(月)＝月＋13；否则，g(月)＝月＋1
    if (month < 3) return month + 13;
    else return month + 1;
}


//计算日期的N值
int n(int year, int month, int day)
{
    //N=1461*f(年、月)/4+153*g(月)/5+日
    return 1461L * f(year, month) / 4 + 153L * g(month) / 5 + day;
}

//利用N值算出某年某月某日对应的星期几
int w(int year, int month, int day)
{
    //w=(N-621049)%7(0<=w<7)
    return(int)((n(year, month, day) % 7 - 621049L % 7 + 7) % 7);
}

int date[6][7];

//该数组对应了非闰月和闰月的每个月份的天数情况
int day_month[][12] = { { 31,28,31,30,31,30,31,31,30,31,30,31 },
{ 31,29,31,30,31,30,31,31,30,31,30,31 } };

PUBLIC void Calendar(int year,int month)
{
    int sw, leap, j, k, wd, day;  //leap用来判断闰年

    char title[] = "SUN MON TUE WED THU FRI SAT";

    sw = w(year, month, 1);
    leap = year % 4 == 0 && year % 100 || year % 400 == 0;  //判断闰年
    for (j = 0; j<6; j++)
        for (k = 0; k<7; k++)
            date[j][k] = 0;  //日期表置0

    for (wd = 0, day = 1; day <= day_month[leap][month-1]; day++)
    {    //将第i＋1月的日期填入日期表
        date[wd][sw] = day;
        sw = ++sw % 7;        //每星期七天，以0至6计数
        if (sw == 0) wd++;    //日期表每七天一行，星期天开始新的一行
    }
    printf("|=================================|");

    printf("\n|The Calendar of Year %d Month %d|\n|", year,month);

    
    wd=6;
    printf("%2d  %s  |\n|", month , title);
    for (j = 0; j<wd; j++)
    {
            printf("   ");//输出四个空白符
            
            //左栏为第i+1月
            for (k = 0; k<7; k++)
            {
                if (date[j][k]) printf("%4d", date[j][k]);
                else printf("    ");
                
            }

            printf("  |\n|");

    }
    printf("=================================|\n");

}

