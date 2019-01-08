#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#include<thread>
#define DEBUG

namespace windows_iocp {

	/*
		这部分的内容属于windows_unit_10,但是关于iocp(io完成端口)的内容非常重要,所有单独做讲

			函数原型:
				HANDLE CreateIoCompletionPort(HADNLE hFile,HANDLE hExistingCompletionPort,ULONG_PTR CompletionKey,DWORD dwNumberOfConcurrentThreads);
					//这个函数略微复杂,因为它做了两件事情:创建一个io端口对象,并和设备相关联.为了能有清晰的认识,我们把这两部分功能分开考虑
				#1:创建io端口对象:
					HANDLE CreateNewIoCompletionPort(DWORD dwNumberOfConcurrentThreads)
					{
						return CreateIoCompletionPort(nullptr,nullptr,0,dwNumberOfConcurrentThreads);
					}
						//@dwNumberOfConcurrentThreads:完成端口在同一时间最多允许运行的线程数量 0表示默认值而非数量0
						//该函数只是创建了一个IO端口对象,没有与设备进行关联;
				#2:关联设备:
					BOOL AssociateDeivceWithCompletionPort(HANDLE hDevice,HANDLE hExistingCompletionPort,ULONG_PTR CompletionKey)
					{
						HANDLE h=CreateIoCompletionPort(hDevice,hExistingCompletionPort,CompletionKey,0);
						return (h==hExistingCompletionPort);
					}
						//@hDevice:进行关联的设备
						//@hExistingCompletionPort:存活的io端口对象
						//@CompletionKey:完成键,会被记录在设备列表结构中,用来追踪IO请求是否已经完成.
			与IO完成端口相关的5个数据结构
				#1:设备列表
					功能:表示与IO完成端口相关联的设备列表
						#1	添加:当调用CreateIoCompletionPort关联设备的时候,会添加设备(hFile)及其完成键(CompletionKey);
						#2	移除:当设备句柄被关闭的时候,会移除设备(hFile);
				#2:IO完成队列(先进先出)
					功能:表示IO请求的完成状态,即请求已完成,等待处理
						#1	入队:IO请求完成;调用PostQueuedCompletionStatus函数
						#2	出队:等待线程队列被唤醒,进入释放线程队列(即处理IO请求完成后的后续工作);
				#3:等待线程队列(后入显出)
					功能:表示正在等待对io请求已完成的处理
						#1	入队:调用GetQueuedCompletionStatus函数
						#2	出队:IO完成队列不为空,正在运行的线程数小于最大线程数;
				#4:已释放线程列表
					功能:处理io请求完成的后序工作
						#1	入队:等待线程出队;已暂停列表中的挂起线程被唤醒
						#2	出队:调用GetQueuedCompletionStatus函数讲线程加入等待队列;线程挂起进入已暂停列表
				#5:已暂停列表
					功能:线程进入挂起状态:
						#1	进入:已释放线程将自己挂起
						#2	出队:已挂起的线程被唤醒

			两个重要函数:
				BOOL GetQueuedCompletionStatus(HADNLE hCompletionPort,PDWORD pdwNumberOfBytesTransferred
												,PULONG_PTR pCompletionKey,OVERLAPPED** ppOverlapped,DWORD dwMilliseconds);
					//_IN_ @hCompletionPort:完成端口句柄
					//_OUT_ @pdwNumberOfBytesTransferred:传输字节数
					//_OUT_ @pCompletionKey:请求的完成键
					//_OUT_ @ppOverlapped:重叠结构
					//_IN_ @dwMilliseconds:等待时间
					//功能:将调用线程与io完成端口相关联,并进入睡眠,添加到等到线程队列.然后根据数据状态进行线程状态切换

				BOOL PostQueuedCompletionStatus(HANDLE hCompletionPort,DWORD dwNumberOfBytesTransferred,
												ULONG_PTR CompletionKey,LPOVERLAPPED pOverlapped);
					//功能:向指定的完成端口的IO完成队列添加一项 
			
			基本流程
				调用create*函数关联设备和IO完成端口------>发出异步io请求------>操作系统使用自己线程完成IO请求--->IO请求完成后加入IO完成队列
											|																		|
											|-->初始化线程池,调用Get*Status函数进入睡眠,等待IO请求完成	------>(当线程数目<最大运行数目时),唤醒线程,操作结果											
																					
					
*/
	//当主动发送key_kill完成键时,通知获取的线程结束自己

	const DWORD key_kill = 0;




	//一些全局对象
	const int buf_sz = 1024;
	char buff[buf_sz];
	const char* file_name = "first.txt";
	HANDLE hFile{ INVALID_HANDLE_VALUE };
	HANDLE iocp{ INVALID_HANDLE_VALUE };
	HANDLE hMutex{ INVALID_HANDLE_VALUE };

	//用于同步输出语句,使用内核对象mutex;
	template<class Arg1,class... Args>
	void async_print(Arg1 arg1,Args... args)
	{
		using namespace std;
		WaitForSingleObject(hMutex, INFINITE);
		cout << arg1;
		async_print(args...);
		ReleaseMutex(hMutex);
		
	}

	template<class Arg>
	void async_print(Arg arg)
	{
		using namespace std;
		WaitForSingleObject(hMutex, INFINITE);
		cout << arg << endl;
		ReleaseMutex(hMutex);
	}

	//这两个自定义的类用于传送额外的数据,在处理函数中做一些有用的辨别处理
	enum class io_oper {
		op_read, op_write, op_kill
	};

	struct my_overlapped :OVERLAPPED {
		io_oper oper;
		my_overlapped() :OVERLAPPED{ 0 }, oper{ io_oper::op_read } {}
		explicit my_overlapped(io_oper op) :OVERLAPPED{ 0 }, oper{ op } {}
	};

	

	void open_file()
	{
		hFile = CreateFile(file_name, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, nullptr);
	}

	DWORD get_current_thread_id()
	{
		HANDLE h=GetCurrentThread();
		return GetThreadId(h);
	}
	


	//io_read_request
	DWORD WINAPI ioreq_read(PVOID pvParam)
	{
		using namespace std;
		DWORD local_id = get_current_thread_id();
		//OVERLAPPED o_read{ 0 };
		my_overlapped o_read{ io_oper::op_read };
		bool flag = ReadFile(hFile, buff, 256, nullptr, &o_read);
		DWORD dw = GetLastError();
		if (!flag&&dw != ERROR_IO_PENDING)
		{
#ifdef DEBUG
			async_print("#", local_id, "(read_req):ReadFile failed:",dw);
#endif // DEBUG
			return -1;
		}
#ifdef DEBUG
		async_print("#", local_id, "(read_req):exit");
#endif // DEBUG
		return 0;
	}

//	//io_read_resolve
//	DWORD WINAPI iores_read(PVOID pvParam)
//	{
//		using namespace std;
//		DWORD local_id = get_current_thread_id();
//		DWORD cp_key{};
//		DWORD bytes_read{};
//		OVERLAPPED o_read{ 0 };
//		LPOVERLAPPED lpo_read = &o_read;
//
//		while (true) {
//			bool flag = GetQueuedCompletionStatus(iocp, &bytes_read, &cp_key, &lpo_read, INFINITE);
//			if (flag)
//			{
//				if (cp_key != key_kill) {
//					async_print("#", local_id , "(read_res):\nbytes_read:", bytes_read, "\ncompletion_key:", cp_key,"\n", buff);
//				}
//				else {
//					break;
//				}
//			}
//			else {
//				DWORD dw = GetLastError();
//				async_print("#", local_id, "(read_res):io failed:", dw);
//				return -1;
//			}
//		}
//		
//#ifdef DEBUG
//		async_print("#", local_id, "(read_res):exit");
//#endif // DEBUG
//		return 0;
//
//	}

	DWORD WINAPI ioreq_write(PVOID pvParam)
	{
		using namespace std;
		DWORD local_id = get_current_thread_id();
		//OVERLAPPED o_write{0};
		my_overlapped o_write{ io_oper::op_write };
		string data{ "to be or not to be,this is a question." };
		bool flag = WriteFile(hFile, data.c_str(), data.size(), nullptr, &o_write);
		DWORD dw = GetLastError();
		if (!flag&&dw != ERROR_IO_PENDING)
		{
#ifdef DEBUG
			async_print("#", local_id, "(write_req):write_file failed ", dw);
#endif // DEBUG
			return -1;
		}
#ifdef DEBUG
		async_print("#", local_id, "(write_req):exit.");
#endif // DEBUG

		return 0;
	}

	//io完成处理函数
	DWORD WINAPI io_resolve(PVOID pvParam)
	{
		using namespace std;
		DWORD local_id = get_current_thread_id();
		DWORD bytes_transfered{};
		DWORD cp_key{};
		OVERLAPPED overlapped{0};
		//my_overlapped overlapped{ 0 };
		//LPOVERLAPPED lpoverlapped = &overlapped;
		LPOVERLAPPED lpoverlapped = &overlapped;
		while (true)
		{
			bool flag=GetQueuedCompletionStatus(iocp, &bytes_transfered, &cp_key, &lpoverlapped, INFINITE);
			if (flag)
			{
				my_overlapped* mo = (my_overlapped*)lpoverlapped;	//转化成自定义的结构
				switch (mo->oper)
				{
				case io_oper::op_read:	//一个读取操作完成了
					async_print("#", local_id, "(io_resolve):\nbytes_read:", bytes_transfered, "\ncompletion_key:", cp_key, "\n", buff);
					break;
				case io_oper::op_write:	//一个写入操作完成了
					async_print("#", local_id, "(io_resolve):write ", bytes_transfered, " bytes");
					break;
				case io_oper::op_kill:	//一个"kill"请求结束自己
					async_print("#", local_id, "(io_resolve):exit by receive a kill singal.");
					return 0;
				default:	
					break;
				}
			}
			else {
				DWORD dw = GetLastError();
				if (dw == ERROR_ABANDONED_WAIT_0)
				{
#ifdef DEBUG
					async_print("#", local_id, " (io_resolve):exit by error \"iocp is closed\"");
#endif // DEBUG
					break;
				}
				else {
#ifdef DEBUG
					async_print("#", local_id, "(io_resolve):exit by error ", dw);
#endif // DEBUG
					break;
				}
			}
		}
#ifdef DEBUG
		async_print("#", local_id, "(write_res):exit.");
#endif // DEBUG

		return 0;
	}

	void _test()
	{
		using namespace std;
		//step1:初始化设备
		if (hFile == INVALID_HANDLE_VALUE)
		{
			open_file();
			if (hFile == INVALID_HANDLE_VALUE)
			{
#ifdef DEBUG
				async_print("open file ", file_name, " failed");
				//cout << "open file " << file_name << " failed!\n";
#endif // DEBUG
				return ;
			}
		}

		hMutex = CreateMutex(nullptr, false, nullptr);
		//step2:创建io完成端口,并与设备相关联
		DWORD ck_read = 1;
		auto max_threads = std::thread::hardware_concurrency();
		iocp=CreateIoCompletionPort(hFile, nullptr, ck_read, max_threads);

		//step3:创建子线程,发出IO读请求
		DWORD id_read_req, id_read_res;
		DWORD id_write_req, id_write_res;
		CreateThread(nullptr, 0, ioreq_read, nullptr, 0, &id_read_req);
		CreateThread(nullptr, 0, io_resolve, nullptr, 0, &id_write_res);
		//CreateThread(nullptr, 0, iores_read, nullptr, 0, &id_read_res);
		//创建IO写子线程
		CreateThread(nullptr, 0, io_resolve, nullptr, 0, &id_read_res);
		CreateThread(nullptr, 0, ioreq_write, nullptr, 0, &id_write_req);
		//CreateThread(nullptr, 0, io_resolve, nullptr, 0, &id_write_res);
		//step4:等待一段时间,发出终止指令
		Sleep(5 * 1000);
		my_overlapped o_kill{ io_oper::op_kill };
		for (unsigned int i = 0; i < max_threads; ++i) {
			PostQueuedCompletionStatus(iocp, 0, key_kill, &o_kill);
		}
		//step5:清理资源
		CloseHandle(hFile);
		CloseHandle(iocp);
		CloseHandle(hMutex);
		//线程结束提示
#ifdef DEBUG
		async_print("#", get_current_thread_id(), "(main_thread):exit!");
#endif // DEBUG

	}

}


/*
	
	learn:
		#1:io处理线程,即调用GetQueuedCompletionStatus函数的线程,并不明白取得的请求时读取还是写入请求,所以最开始写了iores_read/iores_write两个处理函数,但这是不必要的.
			只需要在处理函数内部,根据传送的overlapped结构,获取自定义的数据进行判断处理就行了 (当测试读请求线程(ioreq_read)之后立即调用写处理函数(iores_write)之后,发现这个问题)
		#2:同步输出函数由于需要在内部递归获取某个对象,所以这里选取了互斥量(mutex)而不是关键区(critical_section)
		#3:最开始是希望在处理线程中通过completion_key来进行不同的处理,,但是发现发起请求的线程并不能传递cp_key,只有overlapped结构,
			所以只能使用继承overlapped结构的自定义类来传递控制对象
		
*/