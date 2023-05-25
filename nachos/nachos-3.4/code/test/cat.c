#include "syscall.h"
#include "copyright.h"

#define maxlen 32
#define input 0
#define output 1
int main()
{
	//Phan 3: Cat
	char *fileName;
	char content;
	int openID;
	int fileSize;
	int i;

	Write("Nhap vao ten file muon doc: ", maxlen, output);
	Read(fileName, maxlen, input);
	openID = Open(fileName, 1);
	if(openID == -1)
	{
		Write("Khong the mo file", maxlen, output);
		CloseFile(openID);
	}
	else
	{
		i = 0;
		Write("Noi dung cua file:\n", maxlen, output);
		fileSize = Seek(-1, openID);
		Seek(0, openID);
		
		for(; i < fileSize; i++)
		{
			Read(&content, 1, openID);
			Write(&content, 1, output);
		}
		CloseFile(openID);
	}
	Halt();
}
