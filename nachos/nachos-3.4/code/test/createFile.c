#include "syscall.h"
#include "copyright.h"

#define maxlen 32
#define input 0
#define output 1
int main()
{
	//Phan 1: Create file
	char filename[maxlen];
	Write("Nhap ten file muon tao: ", 100, output);
	Read(filename, maxlen, input);
	CreateFile(filename);
	Halt();
} 
