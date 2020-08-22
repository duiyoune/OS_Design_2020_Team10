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


int time_hour=0;
int time_minute=0;
int time_second=0;
char time[10]="";

PUBLIC int GetCurrentTime()
{
    printf("Current Time:\n");
    printf("%d:%d:%d\n",time_hour,time_minute,time_second);
    
    return time_second;
}

PUBLIC void SetCurrentTime(char *CurrentTime)
{
    char temp[5];
    temp[0]=CurrentTime[0];
    temp[1]=CurrentTime[1];
    atoi(temp,&time_hour);
    temp[0]=CurrentTime[3];
    temp[1]=CurrentTime[4];
    atoi(temp,&time_minute);
    temp[0]=CurrentTime[6];
    temp[1]=CurrentTime[7];
    atoi(temp,&time_second);
    
}

PUBLIC void TimeRunning()
{
    while(1)
    {
        milli_delay(10000);
        time_second++;
        if(time_second>59)
        {
            time_second-=60;
            time_minute++;
        }
        if(time_minute>59)
        {
            time_minute-=60;
            time_hour++;
        } 
        if(time_hour>24)
        {
            time_hour-=24;
        }      
        
    }
}

