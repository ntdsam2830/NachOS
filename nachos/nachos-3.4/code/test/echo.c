#include "syscall.h"
#include "copyright.h"

#define maxlen 32
#define input 0
#define output 1
int main()
{
	//Phan 2: Echo
	char echo[maxlen];	
	Write("Nhap vao 1 chuoi bat ki: ", maxlen, output);
	Read(echo, maxlen, input);
	Write("Chuoi vua nhap vao la: ", maxlen, output);
	Write(echo, maxlen, output);
	Halt();
}
