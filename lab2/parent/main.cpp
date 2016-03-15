#ifdef _WIN32
#include <windows.h>
#include <conio.h>

int mygetch();
int end(int);

#elif linux
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <linux/sched.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#endif
#include <iostream>
#include "my_proc.h"
#include <cstdlib>
using namespace std;
#define MAX_COUNT 10
int MYGETCH();
int kbhit(void);

#ifdef linux
int flagEnd = 1;
void setEndFlag(int signum)
      {
      flagEnd = 1;
      }
#endif



int main()
{
#ifdef _WIN32

    char completedprintfevent[50]="completedprint";
    HANDLE completedprintevent = CreateEvent(NULL ,FALSE,TRUE,completedprintfevent);
#elif linux
         setendsignal();
#endif

    int count=0;
    int current=1;

    MYPROCESS * ms[10];

    while (true)
    {

        char c = 'a';

        while(c!='q')
        {
            c = mygetch();

          switch(c)
         {
            case '+':
          if(count < MAX_COUNT)
          {

            count++;

            MYPROCESS * process = new MYPROCESS(count);
            ms[count-1]=process;
          };
              break;
          case '-':
          if(count > 0)
          {
            ms[count-1]->SET_EVENTNOPRINT();
            count--;
          };
              break;
         default:
          {

#ifdef _WIN32
             if(end(count) && count > 0)
             {
                 if(current > count)
                     current = 1;
                 ms[current-1]->SET_EVENTPRINT();
                 current++;
             }
#elif linux
              if(flagEnd && count > 0)
             {
                 if(current > count)
                     current = 1;
                 ms[current-1]->SET_EVENTPRINT();
                 current++;
             }
#endif
          }
          break;
        }
        }
            for (int i=0;i<count;i++)
            {
                ms[i]->KILLPROCESS();

             }
            break;
    }
    return 0;
    }


#ifdef _WIN32
int mygetch()
{
    if (_kbhit())
        return _getch();
    else
        return -1;
}


int end(int count)
{
        char eventname[50]="completedprint";
        HANDLE printevent = OpenEvent(EVENT_ALL_ACCESS,FALSE,eventname);
      if(WaitForSingleObject(printevent,100)== WAIT_OBJECT_0)
        {
            if (!count) SetEvent(printevent);
            return 1;
        }
    return 0;
}
#endif
