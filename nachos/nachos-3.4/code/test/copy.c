#include "syscall.h"
#include "copyright.h"

#define maxlen 32
#define input 0
#define output 1
int main()
{
	//Phan 4: Copy
	char fileName1[maxlen + 1];
	char fileName2[maxlen + 1];
	int fileSize;
	int i;

	int openID1;
	int openID2;

	char content;

	Write("Nhap vao ten file nguon: ", maxlen, output);
	Read(fileName1, maxlen, input);
	openID1 = Open(fileName1, 0);
	Write("Nhap vao ten file dich: ", maxlen, output);
	Read(fileName2, maxlen, input);
	openID2 = Open(fileName2, 0);
	
	if(openID1 == -1)
	{
		Write("File nguon khong ton tai. Ket thuc chuong trinh", 70, output);
		Halt();
	}
	if(openID2 == -1)
	{
		Write("File dich khong ton tai\n", maxlen, output);
		Write("Dang tao file dich\n", maxlen, output);
		CreateFile(fileName2);
	}
	
	openID2 = Open(fileName2, 0);

	fileSize = Seek(-1, openID1);
	Seek(0, openID1);
	
	for(i = 0; i < fileSize; i++)
	{
		Read(&content, 1, openID1);
		Write(&content, 1, openID2);
	}

	CloseFile(openID1);
	CloseFile(openID2);

	Write("Hoan thanh copy\n", maxlen, output);

	return 0;
}
