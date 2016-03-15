#ifdef _WIN32
#include <windows.h>
#include <iostream>
#include <fstream>
using namespace std;

int check_event(char * number);
void PRINTCOMPLETED();
#endif

#ifdef linux
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <iostream>
using namespace std;

struct sigaction printSignal,stopSignal;
int printFlag = 0;
void canPrint(int num)
{

  printFlag = 1;
}
void stopPrint(int num)
{
  printFlag = -1;
}
void PRINTCOMPLETED()
{
    kill(getppid(),SIGUSR2);
}
#endif




int main(int argc, char *argv[])
{

#ifdef linux
    printSignal.sa_handler = canPrint;
    sigaction(SIGUSR1,&printSignal,NULL);
    stopSignal.sa_handler = stopPrint;
    sigaction(SIGUSR2,&stopSignal,NULL);
#endif
   char STRING[10][30] = {{"1) First process"}, {"2) Second process"}, {"3) Third process"}, {"4) Fourth process"}, {"5) Fifth process"},
   {"6) Sixth process"}, {"7) Seventh process"}, {"8) Eighth process"}, {"9) Ninth process"}, {"10) Tenth process"}};

    int number=1;
    number=atoi(argv[1]);
    while (true)
    {
#ifdef _WIN32
        int printFlag = check_event(argv[1]);
        Sleep(100);
#elif linux
        fflush(stdin);
#endif
        if ( printFlag ==1)
        {
            for (int i = 0;STRING[number-1][i]!=0;i++)
            {
                printf("%c",STRING[number-1][i]);
                fflush(stdout);
#ifdef _WIN32
                Sleep(10);
#elif linux
                usleep(10000);
#endif
            }
            putchar('\n');
            fflush(stdout);
            printFlag=0;
            PRINTCOMPLETED();
        }
        if (printFlag== -1)
        {
            exit(0);
            break;
        }
    }
    return 0;
}

#ifdef _WIN32
int check_event(char * number)
{
    while(1)
    {
        char noprint[50]="noprint";
        char print[50]="print";
        strcat(noprint,number);
        strcat(print,number);
        fflush(stdout);
        HANDLE printevent = OpenEvent(EVENT_ALL_ACCESS,FALSE,noprint);
        if(WaitForSingleObject(printevent,10) == WAIT_OBJECT_0)
        {
            return -1;
        }

        printevent = OpenEvent(EVENT_ALL_ACCESS,FALSE,print);
        if(WaitForSingleObject(printevent,10) == WAIT_OBJECT_0)
        {
            return 1;
        }
        Sleep(10);
        return 0;
    }
}
void PRINTCOMPLETED()
{
        char completed[50]="completedprint";
        HANDLE printevent = OpenEvent(EVENT_ALL_ACCESS,FALSE,completed);
        SetEvent(printevent);
}
#endif























