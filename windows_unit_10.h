#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#define DEBUG

namespace windows_unit_10 {

	/*
		同步设备IO和异步设备IO
			同步:线程发出IO请求后,线程被挂起,直到IO请求完成或者发生错误,函数返回;
			异步:线程发出IO请求后,函数直接返回,当IO请求完成或发生错误后,线程得到通知;
	*/

	/*
		打开和关闭设备:

		msdn_guide:https://docs.microsoft.com/zh-cn/windows/desktop/FileIO/creating-and-opening-files

			设备:任何能够与之通信的东西
			Create系列函数:
				CreateFile(...):可打开文件,目录,逻辑/物理磁盘驱动器,串口,并口,邮件槽客户端;
				CreateMailslot(...):打开邮件槽服务端
				CreateNamedPipe(...):打开命名管道
				CreatePipe(...):打开匿名管道
				Socket():套接字
				CreateConsoleScreenBuffer()/GetStdHandle():控制台
			获取设备类型:
				DWORD GetFileType(HANDLE hDevice); 
					//FILE_TYPE_UNKNOW
					//FILE_TYPE_DISK
					//FILE_TYPE_CHAR
					//FILE_TYPE_PIPE
			CreateFile函数:
				HANDLE CreateFile(PCTSTR,DWORD,DWORD,PSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
					//@pszName:既可以表示设备类型,也可以表示设备的某个实例
					//@dwDesiredAccess:访问权限
						$0:非读非写,只是改变设备的配置
						$GENERIC_READ:只读
						$GENERIC_WRITE:只写
						$GENERIC_READ|GENERIC_WRITE:读写
					//@dwShareMode:指定设备共享特权
						$0:独占访问,若已被独占,则CreateFile会失败
						$FILE_SHARE_READ:共享读权限,其他线程访问可读,但不可写
						$FILE_SHARE_WRITE:共享写权限,其他线程访问可写,但不可读
						$FILE_SHARE_READ|FILE_SHARE_WRITE:共享读写权限
						$FILE_SHARE_DELETE:不关心文件是否被逻辑删除或移动	 "?"
					//@psa:指定安全信息和继承权
					//@dwCreationDisposition:打开方式
						$CREATE_NEW:创建新文件,若同名文件已存在,则CreateFile调用失败
						$CREATE_ALWAYS:创建新文件,如果有同名文件,覆盖之
						$OPEN_EXISTING:打开一个已有文件或这杯,如果不存在,CreateFile调用失败
						$OPEN_ALWAYS:打开已有文件,若不存在,则创建新的文件
						$TRUNCATE_EXISTING:打开已有文件并截断为0字节,如果文件不存在,则调用失败
					//dwFlagAndAttributes:微调与设备之间的通信;如果是一个文件,还能够设置文件的属性
						$FILE_FLAG_*:设置相应的标志,用于设置不同的缓存方式,访问模式和其他作用
						$FILE_ATTRIBUTE_*:设置相应的属性
					//dwFileTemplate:一个已打开的有效句柄,打开文件会继承该属性并忽略dwFlagAndAttributes参数
	*/

	namespace create_and_open_file {


		void _test()
		{
			PCTSTR file_name = "first.txt";
			HANDLE hFile = CreateFile(file_name, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, 0);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				std::cout << "create file " << file_name << " failed.\n";
			}
			else {
				std::cout << "create file " << file_name << " success.\n";
			}

			CloseHandle(hFile);
		}
	}


	/*
		使用文件设备:
			获取文件大小:
				BOOL GetFileSizeEx(HANDLE hFile,PLARGE_INTEGER pliFileSize);
					//@hFile:文件句柄
					//@pliFileSize:文件大小的保存位置,64位
				DWORD GetCompressedFileSize(PCTSTR pszFileName,PDWORD pdwFileSizeHigh);
					//pszFileName:文件名称
					//pdwFileSizeHigh:文件大小的高32位
					//返回值:文件大小的低32位;
				tips:
					GetFileSizeEx返回文件的逻辑大小,而GetCompressedFileSize返回文件的物理大小;
			设置文件指针位置(数据读取的起始位置):
				BOOL SetFilePointerEx(HADNLE hFile,LARGE_INTEGER liDistanceToMove,PLARGE_INTEGER pliNewFilePointer,DWORD dwMoveMethod);
					//@hFile:...
					//@liDistanceToMove:文件指针移动的距离,根据method拥有不同定义;
					//@ploNewFilePointer:保存指针的新值;
					//@dwMoveMethod:指明如何使用liDistanceToMove
						$FILE_BEGIN:文件指针设定为liDistanceToMove设定的参数值,这里解释成无符号64位值
						$FILE_CURRENT:文件指针=当前指针值+liDistanceToMove,有符号64位值
						$FILE_END:文件指针=fileSize+liDistanceToMove,有符号64位值
			设置文件尾:设置当前文件指针的位置为文件尾;
				BOOL SetEndOfFile(HANDLE hFile);
					
				
	*/

	namespace use_file {

		void _test()
		{
			PCTSTR file_name = "first.txt";
			HANDLE hFile = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, nullptr);
			std::cout << "create file " << file_name;
			if (hFile == INVALID_HANDLE_VALUE)
				std::cout << " failed\n";
			else
				std::cout << " success\n";

			LARGE_INTEGER li{ 2048 }, li_{ 0 };

			bool flag = SetFilePointerEx(hFile, li, &li_, FILE_BEGIN);
			if (flag)
			{
				std::cout << "set the file point at" << li_.QuadPart << std::endl;
				flag=SetEndOfFile(hFile);
				if (flag)
				{
					std::cout << "set the file end success\n";
				}
				else {
					std::cout << "set the file end failed ";
					DWORD f = GetLastError();
					std::cout << " error code:" << f << std::endl;
				}
				
			}

			LARGE_INTEGER file_size{};
			GetFileSizeEx(hFile, &file_size);
			std::cout << "file_size:" << file_size.QuadPart<< std::endl;
			CloseHandle(hFile);
		}
	}


	/*
		执行设备IO
			读:
				BOOL ReadFile(HANDLE,PVOID,DWORD,DWORD,OVERLAPPED*);
					//@hFile:使用的设备句柄
					//@pvBuffer:数据缓冲区
					//@dwNumBytesToRead:希望读取的字节数
					//@pdwNumBytes:实际读取的字节数
					//@pOverlapped:重叠结构,用于异步读取
			写:
				BOOL WriteFile(HANDLE,CONST VOID*,DWORD,PDWORD,OVERLAPPED*);
					//同上

			刷新至设备:将缓存数据直接写入到设备
				BOOL FlushFileBuffers(HANDLE hFile);

			同步IO的取消:当某个线程由于同步IO请求而挂起时,可以使用该方式唤醒线程.但该线程在创建时需要有THREAD_TERMINATE权限.
				BOOL CancelSynchronousIo(HANDLE hTrhead);
			
			tips:
				使用GENERIC_READ标志打开的设备可以使用ReadFile;
				使用GENERIC_WRITE标志打开的设备可以使用WriteFile,FlushFileBuffers;
				异步调用read/write时,总是会返回false;
	*/

	namespace file_read_and_write {

		void _test()
		{
			PCTSTR file_name = "first.txt";
			HANDLE hFile = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL, nullptr);
			std::cout << "create file " << file_name;
			if (hFile == INVALID_HANDLE_VALUE)
			{
				std::cout << " failed\n";
				return;
			}
			else 
				std::cout << " success\n";

			std::string data{ "write_file test data." };
			const char* data_to_write = data.c_str();
			
			DWORD write_num{};
			bool flag=WriteFile(hFile, data_to_write, data.size(), &write_num, nullptr);
			if (flag)
			{
				std::cout << "write " << write_num << " bytes data to " << file_name << "\n";
			}

			CloseHandle(hFile);
				
		}

	}
	

	/*
		异步IO
			#1:指定FILE_FLAG_OVERLAPPED打开设备
			#2:调用读/写函数将请求加入到设备的队列


		typedef struct _OVERLAPPED{
			DWORD Internal;
			DWORD InternalHigh;
			DWORD Offset;
			DWORD OffsetHigh;
			HANDLE hEvent;
		}OVERLAPPED,*LPOVERLAPPED;
			@Offset:低32位值,与offsetHigh共同指定文件偏移量(其实读取数据的位置)
			@OffsetHigh:高32位值,与offset共同指定偏移量
			@Internal:错误码
			@InternalHigh:传输字节数目
			@hEvent:接收事件完成的通知.也可用于保存数据

		异步IO注意事项:
			#1:设备驱动程序不必以先入先出的方式处理队列中的IO请求;
			#2:用正确的方式检查错误
			#3:一定不能移动或者销毁IO请求时使用的数据缓存地址和OVERLAPPED结构

		取消队列中的设备IO请求:
			#1:调用CancelIo,并传入线程句柄,关闭由该线程添加的所有IO请求;BOOL CancelIo(HADNLE,hFile);
			#2:关闭设备句柄,取消队列的所有IO请求;
			#3:线程终止,会取消该线程的IO请求;
			#4:BOOL CancelIoEx(HANDLE hFile,LPOVERLAPPED pOverlapped); 指定给定句柄的指定IO请求;
			//被取消的IO请求会返回错误码ERROR_OPERATION_ABORTED;

		接收IO完成通知:
			#1:触发设备内核对象;
			#2:触发事件内核对象;
			#3:使用可提醒IO;
			#4:使用IO完成端口;

			##触发设备内核对象:
				原理:在将异步IO请求加入队列时,将设备内核对象置为未触发状态,操作系统执行完毕IO后,再将设备内核置为可触发状态,某个等待线程将被唤醒执行;
				tips:当使用设备内核对象触发的时候,当发出多个IO请求后,某个IO请求完成之后设备内核被触发,但我们无法得知,"哪一个"请求被触发了!!
			##触发事件内核对象:
				原理:大致原理和触发设备内核对象相同,但是解决了不能分辨IO请求的窘境;
				tips:当我们产生IO请求的时候,read/write有一个overlapped参数,允许我们指定一个事件内核对象,\
					 我们可以根据事件内核对象来判断是哪个请求完成了,但这需要我们创建不同的事件对象
			##可提醒IO:
				原理:系统创建线程的时候,会同时创建一个线程相关的队列,线程产生io请求并加入到设备队列,设备完成io请求之后会将通知加入到请求线程的APC队列中,\
					若该线程处于可提醒状态,那么就会调用处理函数.
				tips:
					我们需要使用ReadFIleEx/WriteFileEx函数来产生io请求,因为需要传递回调函数
					回调函数原型:
						VOID WINAPI CompletionRoutine(DWORD dwError,DWORD dwNumBytes,OVERLAPPED* po);
					APC函数可能不会立即执行,为了执行APC函数,线程需要是可提醒的,通过一下6个函数设置:
						#1:DWORD SleepEx(DWORD,BOOL);
						#2:DWORD WaitForSingleObjectEx(HANDLE,DWORD,BOOL);
						#3:DWORD WaitForMultipleObjectsEx(...);
						#4:BOOL SignalObjectAndWait(...);
						#5:BOOL GetQueueCompletionStatusEx(...);
						#6:DWORD MsgWaitForMultipleObjectsEx(...);
					可提醒函数的优劣:
					 bad:
						回调函数:
							回调函数需要大量的上下文信息,需要将这些信息放在全局变量共享;
						线程问题:
							发出请求的线程,在请求完成时进行处理.(请求/处理处于同一个线程)
					 good:
						我们可以手动的将一项添加到线程的APC队列之中
							DWORD QueueUserAPC(PAPCFUNC pfnFun,HANDLE hThread,ULONG_PTR dwData);
								//@pfnFun:APC调用函数
								//@hThread:指定的线程
								//@dwData:函数调用参数
							函数原型:VOID WINAPI APCFunc(ULONG_PTR dwParam);
							tips:pfnFunc和hThread应该处于同一个进程空间中;
						QueueUserAPC函数可以进行非常高效的线程间通信;
	*/


	namespace async_file_read_and_write {

		char buff[1024];	//读缓冲区
		PCTSTR file_name = "hi.txt";	//打开文件
		HANDLE hFile{ INVALID_HANDLE_VALUE };	//文件句柄

		

	


		

		void open_file()
		{
			hFile = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, nullptr);
		}

		//读取线程发起IO读取请求
		DWORD WINAPI io_read(PVOID pvParam)
		{

			using namespace std;
			LPOVERLAPPED overlap_to_read =(LPOVERLAPPED)(pvParam);
			open_file();
			if (hFile == INVALID_HANDLE_VALUE)
			{
				cout << "open file " << file_name << " failed\n";
				return -1;
			}

			bool flag = ReadFile(hFile, buff, 256, nullptr, overlap_to_read);
			DWORD dw = GetLastError();
			if (!flag&&dw != ERROR_IO_PENDING)
			{
				cout << "read file failed:" << dw << "\n";
				return -1;
			}

			//cout << "child thread:" << overlap_to_read << "\n";
			//cout << " child thread:" << overlap_to_read->InternalHigh << "\n";
			
			return 0;

		}

		//写入线程发起IO写入请求
		DWORD WINAPI io_write(PVOID pvParam)
		{
			using namespace std;
			LPOVERLAPPED overlap_to_write = (LPOVERLAPPED)(pvParam);
			
			string data{ "\nthis is test data.\n" };	//写入数据
			LARGE_INTEGER li{ 0 }, pos{ 0 };
			GetFileSizeEx(hFile, &li);
			overlap_to_write->Offset = li.LowPart;
			overlap_to_write->OffsetHigh = li.HighPart;
			//bool flag_a=SetFilePointerEx(hFile, li, &pos, FILE_BEGIN); 异步写入的文件指针位置需要在overlap结构中指定
			bool flag = WriteFile(hFile, data.c_str(), data.size(), nullptr, overlap_to_write);
			DWORD dw = GetLastError();
			if (!flag&&dw != ERROR_IO_PENDING)
			{
				cout << "return write_file:" << dw << "\n";
				return -1;
			}
			return 0;
		}

		//使用设备内核对象进行触发
		void device_obj_test()
		{
			using namespace std;
			DWORD id_read, id_write;
			OVERLAPPED o_read{ 0 }, o_write{ 0 };
			//初始化hFile
			if (hFile == INVALID_HANDLE_VALUE)
			{
				open_file();
				if (hFile == INVALID_HANDLE_VALUE) {
					cout << "open file " << file_name << " failed\n";
					return;
				}
			}
			//使用子线程发起IO请求
			auto hd_read = CreateThread(nullptr, 0, io_read, &o_read, 0, &id_read);
			//主线程等待IO请求完成
			DWORD dw=WaitForSingleObject(hFile, INFINITE);
			//返回结果检查
			if (dw == WAIT_OBJECT_0)
			{
				cout << "read event done:" << o_read.InternalHigh << " bytes\n";
				cout << buff << endl;
			}
			else if (dw == WAIT_TIMEOUT)
			{
				cout << " time out\n";
			}
			else {
				DWORD err_code = GetLastError();
				cout << "wait_single_obj failed :" << err_code << "\n";
			}

		}

		//使用事件内核对象进行触发
		void event_obj_test()
		{
			using namespace std;
			//初始化hFile
			if (hFile == INVALID_HANDLE_VALUE)
			{
				open_file();
				if (hFile == INVALID_HANDLE_VALUE) {
					cout << "open file " << file_name << " failed\n";
					return;
				}
			}
			DWORD id_read, id_write;
			OVERLAPPED o_read{0}, o_write{0};
			HANDLE read_event = CreateEvent(nullptr, false, false, nullptr);
			HANDLE write_event = CreateEvent(nullptr, false, false, nullptr);
			o_read.hEvent = read_event;
			o_write.hEvent = write_event;
			//子线程发起读写IO请求
			HANDLE hd_read = CreateThread(nullptr, 0, io_read, &o_read, 0, &id_read);
			HANDLE hd_write = CreateThread(nullptr, 0, io_write, &o_write, 0, &id_write);

			//处理请求
			HANDLE h[2] = { read_event,write_event };
			int count = 2;
			while (count) {
				DWORD dw = WaitForMultipleObjects(2, h, false, INFINITE);
				switch (dw - WAIT_OBJECT_0)
				{
				case 0:
					cout << "read event happened\n";
					cout << "read " << o_read.InternalHigh << " bytes\n";
					cout << buff << endl;
					break;
				default:
					cout << "write event happened\n";
					cout << "write " << o_write.InternalHigh << " bytes\n";
					break;
				}
				--count;
			}
		}

		
		//完成函数
		VOID WINAPI read_done(DWORD dwErrir,DWORD dwNumBytes,OVERLAPPED* po)
		{
			using namespace std;
			cout << "read event done\n";
			cout << "read " << dwNumBytes << " bytes\n";
			cout << buff << endl;
		}

		//通过线程的可提醒状态
		void alertable_test()
		{
			using namespace std;
			open_file();
			if (hFile == INVALID_HANDLE_VALUE)
			{
				open_file();
				if (hFile == INVALID_HANDLE_VALUE)
				{
					cout << "open file " << file_name << " failed\n";
					return;
				}
			}

			OVERLAPPED o_read{ 0 };
			bool flag = ReadFileEx(hFile, buff, 256, &o_read, read_done);
			
			DWORD dw = GetLastError();
			if (!flag&&dw!= ERROR_IO_PENDING)
			{
				cout << "ReadFileEx failed:" << dw << "\n";
			}
			else {
				SleepEx(INFINITE, true);
			}
			
		}

	

		

		void _test()
		{
			device_obj_test();
			event_obj_test();
			alertable_test();
			CloseHandle(hFile);

		}
	}



	



}