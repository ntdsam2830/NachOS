#include "syscall.h"
#include "copyright.h"

#define maxlen 32
#define input 0
#define output 1
int main()
{
	//Phan 5: Delete
	char* fileName;
	Write("Nhap vao ten file muon xoa: ", maxlen, output);
	Read(fileName, maxlen, input);
	
	if(Delete(fileName) == 0)
	{
		Write("Xoa file thanh cong", maxlen, output);
	}
	Halt();
}
