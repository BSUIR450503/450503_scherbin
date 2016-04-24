#include "library.h"


PID proc;

#ifdef _WIN32
DWORD WINAPI  check_client_send(LPVOID inum)
{   char x[50];
    DWORD dw;
    while(1)
            {
                if(WaitForSingleObject(event_print_client,INFINITE)==WAIT_OBJECT_0)
                {
                    ReadFile(read,x,sizeof(x),&dw,NULL);
                    cout << "client : " << x << endl;
                }
            }
    return 0;
}

DWORD WINAPI server_send(LPVOID inum)
{
    DWORD dw;
    while(1)
        {
        char x[50];

        cin.getline(x,50);
        if(!WriteFile(write,x,sizeof(x),&dw,NULL))
            cout << "EROR" << endl;
        SetEvent(event_print_server);
        }
    return 0;
}
DWORD WINAPI client_send(LPVOID inum)
{
    DWORD dw;
    while(1)
        {
        char x[50];       
        cin.getline(x,50);
        if(!WriteFile(write,x,sizeof(x),&dw,NULL))
            cout << "EROR" << endl;
        SetEvent(event_print_client);
        }
    return 0;
}
DWORD WINAPI  check_server_send(LPVOID inum)
{   char x[50];
    DWORD dw;
    while(1)
            {
                if(WaitForSingleObject(event_print_server,INFINITE)==WAIT_OBJECT_0)
                {
                    ReadFile(read,x,sizeof(x),&dw,NULL);
                    cout << "server : " << x << endl;
                }
            }
    return 0;
}
#endif

#ifdef linux

void * check_client_send(void *arg)
{
     char x[50];
    while(1)
            {
                    sem_wait(sem_client);
                    read(pipedes[0], x, sizeof(x));
                    cout << "client : " << x << endl;
            }
    return 0;
}
void * server_send(void *arg)
{
    while(1)
        {
        char x[50];     
        cin.getline(x,50);
        if(!write(pipedes[1], x, sizeof(x)))
            cout << "EROR" << endl;
          sem_post(sem_server);
        }
}
void * check_server_send(void *arg)
{
     char x[50];
    while(1)
            {
                   sem_wait(sem_server);
                    read(readl, x, sizeof(x));
                    cout << "server : " << x << endl;
            }
    return 0;
}
void * client_send(void *arg)
{
    while(1)
        {     
        char x[50];     
        cin.getline(x,50);
        if(!write(writel, x, sizeof(x)))
            cout << "EROR" << endl;
        sem_post(sem_client);
        }
}
#endif


int main(int argc, char *argv[])
{
#ifdef _WIN32
    if(argc == 1 )
    {
            char stroka[100];
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;
            CreatePipe(&read,&write,&sa,0);
            STARTUPINFO si;
            ZeroMemory(&si,sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            wsprintf(stroka,"lab3.exe %d %d",(int)write,(int)read);
            CreateProcess(NULL,stroka,NULL,NULL,TRUE,CREATE_NEW_CONSOLE,NULL,NULL,&si,&proc);
            event_print_client=CreateEvent(NULL ,FALSE,FALSE,completed_print_client);
            event_print_server=CreateEvent(NULL ,FALSE,FALSE,completed_print_server);
            threadSERVER1 = CreateThread(NULL,0,check_client_send,NULL,0,NULL);
            threadSERVER2 = CreateThread(NULL,0,server_send,NULL,0,NULL);
            while(1){}
    }
    else
    {   write = (HANDLE)atoi(argv[1]);
        read = (HANDLE)atoi(argv[2]);
        event_print_client=OpenEvent(EVENT_ALL_ACCESS,FALSE,completed_print_client);
        event_print_server=OpenEvent(EVENT_ALL_ACCESS,FALSE,completed_print_server);
        threadCLIENT1 = CreateThread(NULL,0,check_server_send,NULL,0,NULL);
        threadCLIENT2 = CreateThread(NULL,0,client_send,NULL,0,NULL);
        while(1)
        {}
    }
#endif
#ifdef linux
    if(argc == 1)
    {
         sem_server = sem_open(SEMAPHORE_NAME_SERVER, O_CREAT, 0777, 0);
         sem_client = sem_open(SEMAPHORE_NAME_CLIENT, O_CREAT, 0777, 0);
         pipe(pipedes);
         proc = fork();
  if ( proc > 0 )
  {
      int id1;
      pthread_create(&threadSERVER1, NULL, check_client_send, &id1);
      pthread_create(&threadSERVER2, NULL, server_send, &id1);
  }
  if (proc == 0)
  {
      char read[50],write[50];
    snprintf(read, sizeof(read), "%d", pipedes[0]);
    snprintf(write, sizeof(write), "%d", pipedes[1]);
    execlp("/usr/bin/xterm","xterm","-hold","-e","./lab3", read, write,NULL);
  }
    }
    else
    {
         sem_server = sem_open(SEMAPHORE_NAME_SERVER, O_CREAT, 0777, 0);
         sem_client = sem_open(SEMAPHORE_NAME_CLIENT, O_CREAT, 0777, 0);
         int id1;
         readl = atoi(argv[1]);
         writel = atoi(argv[2]);
         pthread_create(&threadCLIENT1, NULL, check_server_send, &id1);
         pthread_create(&threadCLIENT2, NULL, client_send, &id1);
    }
    while(1)
    {}
    return 0;
#endif
}

