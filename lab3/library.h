#ifdef _WIN32
using namespace std;
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <stdio.h>
#define PID PROCESS_INFORMATION

SECURITY_ATTRIBUTES sa;
HANDLE write,read;
HANDLE event_print_client,event_print_server;
HANDLE threadSERVER1,threadSERVER2,threadCLIENT1,threadCLIENT2;

#endif


#ifdef __linux__
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <fcntl.h>
#include <semaphore.h>
using namespace std;
#define PID int
#define SEMAPHORE_NAME_CLIENT "SEMAPHORE_CLIENT"
#define SEMAPHORE_NAME_SERVER "SEMAPHORE_SERVER"
int readl,writel;
int pipedes[2];
sem_t *sem_client,*sem_server;
pthread_t threadSERVER1,threadSERVER2,threadCLIENT1,threadCLIENT2;
#endif

