#ifdef _WIN32
using namespace std;
#include <iostream>
#include <conio.h>
#include <windows.h>
#define PID PROCESS_INFORMATION

#elif linux
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <iostream>
#include <errno.h>
#define PID int
#define EDOM
#define EILSEQ
#define ERANGE
extern int errno;
using namespace std;
#endif

class MYPROCESS
{
public:
    PID proc;
	int rezultat;   
	int return_rezultat()
	{
		return this->rezultat;
	}
#ifdef _WIN32

    MYPROCESS(int count)
    {  
	char stroka[50];
	STARTUPINFO si;	
	HANDLE write,read;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	CreatePipe(&read,&write,&sa,0);	
	ZeroMemory(&si,sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	int a=count;
	DWORD dw;		
	WriteFile(write,&a,sizeof(a),&dw,NULL);	
	wsprintf(stroka,"C://lab0//process-B//Release//process-B.exe %d %d",(int)write,(int)read);
	CreateProcess(NULL,stroka,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&proc); 
	WaitForSingleObject(proc.hProcess,INFINITE);
	ReadFile(read,&a,sizeof(a),&dw,NULL);
	this->rezultat=a;
	CloseHandle(proc.hProcess);
	CloseHandle(proc.hThread);
	CloseHandle(read);
	CloseHandle(write);
	}
#elif linux
	MYPROCESS(int count)
	{
		int pipedes[2];
  int status;
  pid_t pid;
  pipe(pipedes);
  pid = fork();
  if ( pid > 0 )
  {
    int x=cunt;   
    write(pipedes[1], &x, sizeof(x));
    wait(&status);
    read(pipedes[0], &x, sizeof(x));
    this->rezultat=x;
  }
  if (pid == 0)
  {
    char read[50],write[50];
    snprintf(read, sizeof(read), "%d", pipedes[0]);
    snprintf(write, sizeof(write), "%d", pipedes[1]);
    execl("/home/andrey/helloworld", "helloworld", read, write,(char*)NULL);
  } 
	}
#endif
}
	
   

