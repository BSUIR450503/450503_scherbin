
#ifdef _WIN32
using namespace std;
#define PID PROCESS_INFORMATION

#elif linux
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#define PID int
using namespace std;
#endif

class MYPROCESS
{
public:
    PID proc;
    #ifdef _WIN32
    HANDLE SET_PRINT_EVENT,SET_NO_PRINT_EVENT;
    MYPROCESS(int count)
    {
        char name[50];
        char number[10];
        char eventprint[50]="print";
        char eventnoprint[50]="noprint";

        STARTUPINFO si;
        ZeroMemory( &si, sizeof(si) );
        si.cb = sizeof(si);
        ZeroMemory( &proc, sizeof(proc) );
        wsprintf(name,"C://lab0//process-B//Release//process-B.exe %d",count);
        wsprintf(number,"%d",count);
        strcat(eventprint,number);
        if(!CreateProcess(NULL, name ,NULL,NULL,FALSE,NULL,NULL,NULL,&si,&proc))
            cout<<"FAIL"<<endl;
        SET_PRINT_EVENT = CreateEvent(NULL ,FALSE, FALSE,eventprint);
        strcat(eventnoprint,number);
        SET_NO_PRINT_EVENT = CreateEvent(NULL ,FALSE, FALSE,eventnoprint);
    }
    MYPROCESS(){};
#elif linux
    MYPROCESS(int count)
    {
        char number[20];
             proc = fork ();
             if (proc == 0)
             {  sprintf(number, "%d", count);
                execl("/home/andrey/release/child-process", "child-process",number,NULL);
             }
    }
#endif
    void SET_EVENTPRINT()
    {
        #ifdef linux

        kill(proc,SIGUSR1);
        #elif _WIN32
        SetEvent(SET_PRINT_EVENT);
        #endif
    }
    void SET_EVENTNOPRINT()
    {
        #ifdef linux
        kill(proc,SIGUSR2);
        waitpid(proc,NULL,NULL);
        #elif _WIN32
        SetEvent(SET_NO_PRINT_EVENT);
        #endif
    }
    void KILLPROCESS()
    {
        #ifdef linux
        kill(proc,SIGKILL);
        waitpid(proc,NULL,NULL);
        #elif _WIN32
        DWORD c=0;
        TerminateProcess(proc.hProcess,c);
        #endif
    }
};
#ifdef linux
void setEndFlag(int signum);
void setendsignal()
{
    struct sigaction endSignal;
    endSignal.sa_handler = setEndFlag;
    sigaction(SIGUSR2,&endSignal,NULL);
}
void set_keypress(termios & stored_settings)
    {
        struct termios new_settings;
        tcgetattr(0,&stored_settings);
        new_settings = stored_settings;

        new_settings.c_lflag &= (~ICANON);
        new_settings.c_lflag &= ~ECHO;
        new_settings.c_cc[VTIME] = 0;
        new_settings.c_cc[VMIN] = 1;
        tcsetattr(0,TCSANOW,&new_settings);
    }
    void reset_keypress(termios & stored_settings)
    {
        tcsetattr(0,TCSANOW,&stored_settings);
    }
    char mygetch()
    {
        struct termios stored_settings;
        set_keypress(stored_settings);
        return getchar();
        reset_keypress(stored_settings);
    }

#endif


