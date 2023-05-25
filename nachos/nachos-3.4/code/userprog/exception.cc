// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
#define MaxFileLength 32 

void IncreasePC()								//HAM TANG THANH GHI
{
	int pcAfter = machine->registers[NextPCReg] + 4;			//Tao bien pcAfter bang voi gia tri thanh ghi tiep theo cua thanh ghi next
	machine->registers[PrevPCReg] = machine->registers[PCReg];		//Gan gia tri thanh ghi prev bang gia tri thanh ghi hien tai
	machine->registers[PCReg] = machine->registers[NextPCReg];		//Gan gia tri thanh ghi hien tai bang gia tri thanh ghi next 
	machine->registers[NextPCReg] = pcAfter;				//Gan gia tri cua thanh ghi next bang pcAfter
}


char* User2System(int virtAddr,int limit)					//HAM CHUYEN 1 CHUOI TU USER SPACE -> KERNEL SPACE
{										
	 int i;									
	 int oneChar;
	 char* kernelBuf = NULL;
	 
	 kernelBuf = new char[limit +1];					//Khoi tao chuoi kernelBuf voi vung nho limit + 1
	 if (kernelBuf == NULL)							//Truong hop khong du bo nho -> kernelBuf = NULL
	 	return kernelBuf;
	 memset(kernelBuf,0,limit+1);						//Gan gia tri cho cac phan tu cua kernelBuf = 0
	 for (i = 0 ; i < limit ;i++)						//Thuc hien chep lan luot 1 ki tu den tu dia chi virtAddr vao kernelBuf 
	 {
	 	machine->ReadMem(virtAddr+i,1,&oneChar);
	 	kernelBuf[i] = (char)oneChar;					
	 	if (oneChar == 0)						//Ket thuc khi i = limit hoac da truyen het ki tu cua chuoi nguon
	 		break;
	 }
	 return kernelBuf;							//Tra ve kernelBuf cho Kernel Space
} 


int System2User(int virtAddr,int len,char* buffer)				//HAM CHUYEN 1 CHUOI TU KERNEL SPACE -> USER SPACE
{
	 if (len < 0) return -1;						
	 if (len == 0)return len;
	 int i = 0;
	 int oneChar = 0 ;
	 
	 do{									//Thuc hien chep lan luot 1 ki tu den tu chuoi buffer vao dia chi 						     			 								virtAddr						
	 	oneChar= (int) buffer[i];
	 	machine->WriteMem(virtAddr+i,1,oneChar);
	 	i ++;
	 }while(i < len && oneChar != 0);					//Ket thuc khi i = len hoac da truyen het ki tu cua chuoi nguon
	 return i;								//Tra ve so luong tu vua ghi vao dia chi virtAddr cho User Space 
} 

bool validFileName(char fileName[])						//HAM KIEM TRA TEN FILE CO HOP LE HAY KHONG
{
	if(fileName[0] == ' ')
	{
		printf("Ten file khong the bat dau bang khoang trong");
		return false;
	}
	
	for(int i = 0; i < strlen(fileName); i++)
	{
		if(fileName[i]=='/'||fileName[i]==':'||fileName[i]=='*'||fileName[i]=='?'||fileName[i] =='"'||fileName[i]=='<'||fileName[i]=='>'||fileName[i] == '|')
		{
			printf("Ten file khong the cac ki tu dac biet");
			return false;
		}
	}
	return true;
}


void ExceptionHandler(ExceptionType which)									//HAM PHAN NHANH CAC EXCEPTION
{
   	int type = machine->ReadRegister(2);
	
	switch(which)								
	{
		case NoException:										//Cac truong hop loi khi goi cac system call
			return;
		case PageFaultException:
			DEBUG('a', "\nNo valid translation found\n");
			printf("\nNo valid translation found\n");
			interrupt->Halt();
			break;
		case ReadOnlyException:
			DEBUG('a', "\nWrite attempted to page marked 'read-only' \n");
			printf("\nWrite attempted to page marked 'read-only' \n");
			interrupt->Halt();
			break;
		case BusErrorException:
			DEBUG('a', "\nTranslation resulted in an invalid physical address \n");
			printf("\nTranslation resulted in an invalid physical address  \n");
			interrupt->Halt();
			break;
		case AddressErrorException:
			DEBUG('a', "\nUnaligned reference or one that was beyond the end of the address space \n");
			printf("\nUnaligned reference or one that was beyond the end of the address space \n");
			interrupt->Halt();
			break;
		case OverflowException:
			DEBUG('a', "\nInteger overflow in add or sub \n");
			printf("\nInteger overflow in add or sub \n");
			interrupt->Halt();
			break;
		case IllegalInstrException:
			DEBUG('a', "\nUnimplemented or reserved instr \n");
			printf("\nUnimplemented or reserved instr \n");
			interrupt->Halt();
			break;
		case NumExceptionTypes:
			DEBUG('a', "\nNumber exception types \n");
			printf("\nNumber exception types \n");
			interrupt->Halt();
			break;	
		case SyscallException:										//Cac system call
			switch (type)
			{
				case SC_Halt:									//HALT: dung he dieu hanh
				{
						printf("\nShutdown,initated by user programer.\n");
						interrupt->Halt();
						break;
				}
				case SC_CreateFile:								//CREATEFILE: tao 1 file moi
				{
						int virtAddr;
						char* filename = new char[MaxFileLength];
						bool success;
						int i;
						
						virtAddr = machine->ReadRegister(4);				//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4 
						filename = User2System(virtAddr, MaxFileLength);		//Bien filename duoc gan du lieu bang voi ten file truyen vao
						
						if (filename == NULL)						//Truong hop khong du bo nho de cap phat cho bien filename
						{
							printf("Bo nho khong du de tao file '%s'.", filename);
							machine->WriteRegister(2, -1);				//Ghi -1 vao thanh ghi so 2
							delete filename;
							break;
						}
						if(strlen(filename) == 0)					//Truong hop ten file la rong (khong the dat ten file la L)
						{
							printf("Khong the tao file voi ten trong.", filename);
							machine->WriteRegister(2, -1);				//Ghi -1 vao thanh ghi so 2
							delete filename;
							break;
						}
						if(!validFileName(filename))
						{
							machine->WriteRegister(2, -1);				//Ghi -1 vao thanh ghi so 2
							delete filename;
							break;
						}
						success = fileSystem->Create(filename, 0);			//Thuc hien ham tao file cua fileSystem
						if (!success)							//Truong hop tao file that bai
						{
							printf("Loi khi tao file '%s'.", filename);
							machine->WriteRegister(2, -1);				//Ghi -1 vao thanh ghi so 2
							delete filename;
							break;
						}
						else								//Truong hop tao file thanh cong
						{
							printf("Tao file '%s' thanh cong.", filename);			
							machine->WriteRegister(2, 0);				//Ghi 0 vao thanh ghi so 2				
							delete filename;	
							break;
						}
				}
				case SC_ReadString:								//READSTRING: doc chuoi
			    	{
					int virtAddr;
					int length;
					char* buffer;
				
					virtAddr = machine->ReadRegister(4); 					//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4 
					length = machine->ReadRegister(5); 					//Doc do dai toi da cua chuoi tu thanh ghi so 5
					buffer = User2System(virtAddr, length); 				//Bien buffer duoc gan du lieu bang voi chuoi truyen vao
					gSynchConsole->Read(buffer, length); 					//Ham read cua gSynchoConsole yeu cau nhap 1 chuoi ki tu va gan vao bien buffer, tra ve so 															ki tu cua chuoi vua nhap
					System2User(virtAddr, length, buffer); 					//Gan gia tri vua nhap vao o dia chi cua virtAddr
					delete buffer; 
					break;
				}
            			case SC_PrintString:								//PRINTSTRING: xuat chuoi
            			{
					int virtAddr;
					char* buffer;
					int length;
					length = 0;

					virtAddr = machine->ReadRegister(4); 					//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4
					buffer = User2System(virtAddr, 255); 					//Bien buffer duoc gan du lieu bang voi chuoi truyen vao
					while (buffer[length] != 0) 						//Dem do xai thuc su cua chuoi
					    length++; 
					gSynchConsole->Write(buffer, length + 1); 				//Xuat chuoi thong qua ham Write cua gsynchconsole
					delete buffer; 
					break;
				}
				case SC_Open:												//OPEN: mo file
				{
					int virtAddr; 								
					int type;			 					
					char* filename;
					int freeSlot;

					virtAddr = machine->ReadRegister(4);								//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4
					type = machine->ReadRegister(5); 								//Doc dang mo file tu thanh ghi so 5
					filename = User2System(virtAddr, MaxFileLength + 1); 						//Bien filename duoc gan du lieu bang voi ten file truyen vao
					freeSlot = fileSystem->FindFreeSlot();								//Bien freeSlot dung de kiem trong openf con cho trong hay khong
					if (freeSlot != -1) 										//Truong hop con cho trong
					{
						if (type == 0 || type == 1)								//Doi voi type 0 (Read n Write) va type 1 (Only read)
						{
							if ((fileSystem->openf[freeSlot] = fileSystem->Open(filename, type)) != NULL) 	//Thuc hien mo file cua fileSystem va truyen gia tri vao phan tu 																		freeslot cua openf
							{
								machine->WriteRegister(2, freeSlot); 					//Ghi OpenfileID vao thanh ghi so 2
								fileSystem->openf[freeSlot]->SetFileName(filename);
								delete filename;
								break;	
							}
						}
						if (type == 2) 										//Doi voi type 2 (stdin)
						{
							machine->WriteRegister(2, 0); 							//Ghi 0 vao thanh ghi so 2
							delete filename;
							break;
						}
						if(type == 3)										//Doi voi type 3 (stdout)
						{
							machine->WriteRegister(2, 1); 							//Ghi 1 vao thanh ghi so 2
							delete filename;
							break;
						}
					}
	
					machine->WriteRegister(2, -1); 									//Truong hop openf khong con cho trong hoac khong mo duoc 																		file
					delete filename;
					break;
				}
				case SC_CloseFile:									//ClOSE: dong file
				{
					int openid = machine->ReadRegister(4); 						//Doc OpenFileID cua file tu thanh ghi so 4
					if (openid >= 0 && openid <= 9) 						//Chi xu li khi OpenFileID nam trong khoang [0; 9] vi openf co toi da 10 phan tu
					{
						if (fileSystem->openf[openid] != NULL) 					//Neu file dang duoc mo
						{
							delete fileSystem->openf[openid]; 				//Xoa vung nho cua file do
							fileSystem->openf[openid] = NULL; 				//Gan vung nho bang NULL
							machine->WriteRegister(2, 0);					//Ghi 0 vao thanh ghi so 2
							break;
						}
					}
					machine->WriteRegister(2, -1);							//Neu file dang dong thi ghi -1 vao thanh ghi so 2
					break;
				}
				case SC_ReadInt:									//READINT: doc so nguyen
				{
					char* buffer;								//Chuoi buffer dung de luu so duoc truyen vao duoi dang chuoi
					int MAX_BUFFER = 255;
					buffer = new char[MAX_BUFFER + 1];
					int numbytes = gSynchConsole->Read(buffer, MAX_BUFFER);			//Ham read cua gSynchoConsole yeu cau nhap 1 chuoi ki tu va gan vao bien buffer, tra ve so 															ki tu cua chuoi vua nhap
					bool isNegative = false; 						//Bien isNegative de xem so duoc truyen vao co phai so am hay khong
					int firstNumIndex = 0;							//Xac dinh ki tu dau tien cua chuoi so nguyen
					int lastNumIndex = 0;							//Xac dinh ki tu cuoi cung cua chuoi so nguyen (khong tinh sau dau '.')
					int number = 0;                  					//Luu ket qua tra ve
					   
					if(buffer[0] == '-')							//Kiem tra xem so truyen vao la so am hay duong
					{
						isNegative = true;
						firstNumIndex = 1;
						lastNumIndex = 1;                        			   		
					}
					    
					for(int i = firstNumIndex; i < numbytes; i++)					//Kiem tra buffer co phai so nguyen hay khong				
					{
						if(buffer[i] == '.') 							//Kiem tra sau dau '.' vi '.0' van la so nguyen
						{
							int j = i + 1;
					    		for(; j < numbytes; j++)
					    		{
					        		if(buffer[j] != '0')					//Sau dau '.' ton tai so khac 0 -> Khong phai so nguyen				
					        		{
									printf("'%s' khong phai la so nguyen.", buffer);
									machine->WriteRegister(2, 0);
									delete buffer;
									return;
					        		}
					    		}
						lastNumIndex = i - 1;							
						break;                           
						}
						else if(buffer[i] < '0' && buffer[i] > '9')				//Kiem tra gia tri cua tung ki tu co nam trong khoang [0; 9] hay khong
						{
							printf("'%s' khong phai la so nguyen.", buffer);
							machine->WriteRegister(2, 0);
							delete buffer;
							return;
						}
						lastNumIndex = i;    
					}			
					   
					for(int i = firstNumIndex; i<= lastNumIndex; i++)				//Thuc hien chuyen tu chuoi ve so nguyen
					{
						number = number * 10 + (int)(buffer[i] - 48); 
		                        }
					    
					if(isNegative)									//Neu la so am thi nhan '-1' vao ket qua
					{
						number = number * -1;
					}
					machine->WriteRegister(2, number);
					delete buffer;
					break;		
				}
				case SC_PrintInt:								//PRINTINT: xuat so nguyen
				{	
					int number = machine->ReadRegister(4);					//Doc gia tri cua so duoc truyen vao tu thanh ghi so 4 
					if(number == 0)								//Truong hop so truyen vao la so 0					
					{
						gSynchConsole->Write("0", 1); 
						break;    
					}
					  
					bool isNegative = false; 						//Bien isNegative de xem so duoc truyen vao co phai so am hay khong
					int numberOfNum = 0; 							//Bien numberofNum dung de luu so chu so cua number
					int firstNumIndex = 0;
					if(number < 0)								//Truong hop so truyen vao la so am thi doi sang so duong de chuyen thanh chuoi
					{
						isNegative = true;
						number = number * -1; 
						firstNumIndex = 1; 
					} 	
					 
					int tnumber = number; 							//Bien tam cua bien number
					while(tnumber)								//Tinh so luong chu so trong number
					{
						numberOfNum++;
						tnumber /= 10;
					}
					   
					char* buffer;
					int MAX_BUFFER = 255;
					buffer = new char[MAX_BUFFER + 1];
					for(int i = firstNumIndex + numberOfNum - 1; i >= firstNumIndex; i--)	//Thuc hien chuyen number tu int sang char* (dang chuoi)
					{
						buffer[i] = (char)((number % 10) + 48);
						number /= 10;
					}
					
					if(isNegative)								//Truong hop number la so am
					{
						buffer[0] = '-';
						buffer[numberOfNum + 1] = 0;
						gSynchConsole->Write(buffer, numberOfNum + 1);			//Ghi buffer(dang chuoi cua number) ra man hinh
						delete buffer;
						break;
					}
					buffer[numberOfNum] = 0;						//Truong hop number la so duong	
					gSynchConsole->Write(buffer, numberOfNum);
					delete buffer;
					break;        			
				}
				case SC_Read:									//READ: doc file
				{
					int virtAddr;
					int charcount;
					int id;
					int OldPos;
					int NewPos;
					char* buf;

					virtAddr = machine->ReadRegister(4); 					//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4 
					charcount = machine->ReadRegister(5); 					//Doc so ki tu toi da co the doc cua ham syscall Read tu thanh ghi so 5
					id = machine->ReadRegister(6); 						//Doc id cua file dang mo tu thanh ghi so 6

					if (id < 0 || id > 9)							//Kiem tra id cua file truyen vao co nam ngoai openf hay khong
					{
						printf("Khong the read vi id nam ngoai bang mo ta file.");
						machine->WriteRegister(2, -1);
						delete buf;
						break;
					}
					if (fileSystem->openf[id] == NULL)					//Kiem tra file co ton tai khong
					{
						printf("Khong the read vi file nay khong ton tai.");
						machine->WriteRegister(2, -1);
						delete buf;
						break;
					}
					if (fileSystem->openf[id]->type == 3) 					//Truong hop file duoc mo theo dang stdout (type 3)-> Khong the read file stdout
					{
						printf("Khong the read file stdout.");
						machine->WriteRegister(2, -1);
						delete buf;
						break;
					}
					
					OldPos = fileSystem->openf[id]->GetCurrentPos(); 			//Bien OldPos luu giu vi tri con tro hien tai cua file
					buf = User2System(virtAddr, charcount); 				//Bien buf duoc gan du lieu bang voi chuoi ki tu duoc truyen vao
										
					if (fileSystem->openf[id]->type == 2)					//Truong hop file duoc mo theo dang stdin (type 2)
					{
						int size;
						size = gSynchConsole->Read(buf, charcount); 			//Ham read cua gSynchConsole yeu cau nguoi dung nhap 1 chuoi ki tu va gan vao bien buf, tra 															ve so ki tu cua chuoi vua nhap
						if(size == 0)
						{
							printf("Chuoi vua nhap la chuoi rong. Khong the doc");
							break;
						}
			
						System2User(virtAddr, size, buf); 				//Gan vao o nho virtAddr chuoi buf voi so ki tu toi da la size va dua ve lai User Space
						machine->WriteRegister(2, size); 				//Ghi so ki tu doc duoc vao thanh ghi so 2
						delete buf;
						break;
					}

					if ((fileSystem->openf[id]->Read(buf, charcount)) > 0)			//Truong hop file duoc mo theo dang binh thuong (type = 0 || type = 1)
					{									//Thuc hien doc chuoi buf tu file neu file khong rong
						NewPos = fileSystem->openf[id]->GetCurrentPos();		//Bien NewPos luu giu vi tri con tro moi cua file
						System2User(virtAddr, NewPos - OldPos, buf); 			//Gan vao o nho virtAddr chuoi buf voi so ki tu toi da la (NewPos - OldPos) va dua ve lai 															User Space
						machine->WriteRegister(2, NewPos - OldPos);			//Ghi so ki tu doc duoc vao thanh ghi so 2
						delete buf;
						break;
					}
					else									//Truong hop file la file rong
					{
						printf("File rong. Khong co gi de doc.");
						machine->WriteRegister(2, -2);
						delete buf;
						break;
					}
				}
				case SC_Write:										//WRITE: doc file
				{
					int virtAddr;
					int charcount;
					int id;
					int OldPos;
					int NewPos;
					char* buf;
					int tempcharcount;
					tempcharcount = 0;

					virtAddr = machine->ReadRegister(4); 						//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4 
					charcount = machine->ReadRegister(5); 						//Doc so ki tu toi da co the ghi cua ham syscall Write tu thanh ghi so 5
					id = machine->ReadRegister(6); 							//Doc id cua file dang mo tu thanh ghi so 6				
					
					if (id < 0 || id > 9)								//Kiem tra id cua file truyen vao co nam ngoai openf hay khong
					{
						printf("Khong the write vi id nam ngoai bang mo ta file.");
						machine->WriteRegister(2, -1);
						delete buf;
						break;
					}
					if (fileSystem->openf[id] == NULL)						//Kiem tra file co ton tai khong
					{
						printf("Khong the write vi file nay khong ton tai.");
						machine->WriteRegister(2, -1);
						delete buf;
						break;
					}
					
					if (fileSystem->openf[id]->type == 1 || fileSystem->openf[id]->type == 2)	//Truong hop file duoc mo theo kieu read-only va kieu stdin (type 1 || type 2)
					{
						printf("Khong the write file stdin hoac file only read.");
						machine->WriteRegister(2, -1);
						delete buf;
						break;
					}

					OldPos = fileSystem->openf[id]->GetCurrentPos(); 				//Bien OldPos luu giu vi tri con tro hien tai cua file
					buf = User2System(virtAddr, charcount);  					//Bien buf duoc gan du lieu bang voi chuoi ki tu duoc truyen vao

					if (fileSystem->openf[id]->type == 0)						//Truong hop file duoc mo theo kieu read n write (type 0)
					{
						if ((fileSystem->openf[id]->Write(buf, charcount)) > 0)			//Thuc hien ghi chuoi buf vao file neu chuoi buf khong rong
						{
							NewPos = fileSystem->openf[id]->GetCurrentPos();
							machine->WriteRegister(2, NewPos - OldPos);
							delete buf;
							break;
						}
					}
					if (fileSystem->openf[id]->type == 3) 						//Truong hop file duoc mo theo kieu stdout (type 3)
					{
						int i;
						i = 0;
						while (buf[i] != 0) 							// Vong lap de write den khi gap ky tu '\n'
						{
							gSynchConsole->Write(buf + i, 1);
							i++;
						}
						machine->WriteRegister(2, i - 1); 					//Ghi vao thanh ghi so 2 so ki tu da ghi
						delete buf;
						break;
					}
				}
				case SC_Seek:										//SEEK: di chuyen vi tri con tro file
				{
					int pos = machine->ReadRegister(4); 						//Doc vi tri con tro se tro den tu thanh ghi so 4
					int id = machine->ReadRegister(5);						//Doc id cua file dang mo tu thanh ghi so 5
				
					if (id < 0 || id > 9)								//Kiem tra id cua file truyen vao co nam ngoai openf hay khong
					{
						printf("Khong the seek vi id nam ngoai bang mo ta file.");
						machine->WriteRegister(2, -1);
						break;
					}
					
					if (fileSystem->openf[id] == NULL)						//Kiem tra file co ton tai khong
					{
						printf("Khong the seek vi file nay khong ton tai.");
						machine->WriteRegister(2, -1);
						break;
					}
					
					if (id == 0 || id == 1)								//Truong hop file la file console
					{
						printf("Khong the seek tren file console.");
						machine->WriteRegister(2, -1);
						break;
					}
					
					if(pos == -1)									//Neu pos = -1 thi gan pos = Length 
						pos = fileSystem->openf[id]->Length();
					if (pos > fileSystem->openf[id]->Length() || pos < 0) 				//Truong hop vi tri seek khong phu hop
					{
						printf("Khong the seek file den vi tri nay.");
						machine->WriteRegister(2, -1);
						break;
					}
					else										//Truong hop seek thanh cong
					{
						fileSystem->openf[id]->Seek(pos);
						machine->WriteRegister(2, pos);						//Ghi vi tri con tro sau khi seek vao thanh ghi so 2
						break;
					}
				}
				case SC_Delete:										//DELETE: xoa file
				{
					int virtAddr;
					int i;
					char* filename;
					
					
					virtAddr = machine->ReadRegister(4);						//Doc o dia chi dau tien cua chuoi truyen vao tu thanh ghi so 4 
					filename = User2System(virtAddr, MaxFileLength + 1);				//Bien filename duoc gan du lieu bang voi ten file truyen vao

					for(i = 2; i < 10; i++)								//Kiem tra trong openf co ton tai file hay khong (file co dang mo hay khong)
					{
						if(fileSystem->openf[i] != NULL)
						{
							if(strcmp(filename, fileSystem->openf[i]->GetFileName())== 0)
							{
								printf("File dang duoc mo. Khong the xoa");
								machine->WriteRegister(2, -1);
								delete filename;
								break;
							}
						}
					}
					if (filename == NULL)								//Truong hop khong du bo nho de cap phat cho bien filename
					{
						printf("Bo nho khong du de tao filename.");
						machine->WriteRegister(2, -1);
						delete filename;
						break;
					}
					if(!fileSystem->Remove(filename))						//Truong hop khi goi ham remove that bai
					{
						printf("File khong ton tai");
						machine->WriteRegister(2,-1);
						delete filename;					
						break;
					}
					machine->WriteRegister(2,0);							//Truong hop khi goi ham remove thanh cong
					delete filename;
					return;
				}
			default:
				break;
			}
		IncreasePC();				
	}
}
