#ifdef _WIN32
using namespace std;
#include <iostream>
#include <conio.h>
#include <windows.h>

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
using namespace std;
#endif

#ifdef _WIN32
int main(int argc ,char* argv[])
{
	HANDLE write,read;
	DWORD dw;
	int a;
	write = (HANDLE)argv[1];
	read = (HANDLE)argv[2];
	ReadFile(read,&a,sizeof(a),&dw,NULL);
	a=a*a;
	WriteFile(write,&a,sizeof(a),&dw,NULL);
	return 0;
}
#elif linux
int main(int argc ,char* argv[])
{
	int x;
   int readl = atoi(argv[1]);
   int writel = atoi(argv[2]);
   read(readl, &x, sizeof(x));
   x=x*x;
   write(writel, &x, sizeof(x));
	return 0;
}
#endif


	
