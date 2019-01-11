#pragma once
#include<iostream>
#include<Windows.h>
#include<string>
#include"general.h"

namespace windows_unit_11
{
	/*
		这里主要介绍windows线程池:
			#1:使用异步方式调用函数
			#2:每隔一段时间调用函数
			#3:内核对象触发的时候调用函数
			#4:异步IO请求完成时调用函数
			#5:回调函数的终止操作
	*/


	/*
		#1:使用异步方式调用函数
			##1:定义一个回调函数:
				VOID NTAPI SimpleCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext);
					//@pInstance:描述在函数执行完毕返回之后,应该执行的操作.通常是对同步对象(临界区,互斥量,信号量,事件)的处理,也可以表示卸载dll
					//@pvContext:上下文参数,类似于pvParam,用于传递需要的信息
			##2:向线程池提交请求
				BOOL TrySumbitThreadpoolCallback(PTP_SIMPLE_CALLBACK pfnCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
					//@pfnCallback:自定义的回调函数指针
					//@pvContext:传递给函数的pvContext值
					//@pcbe:
			当使用trysubmit提交请求的时候,系统会在内部替我们分配一个工作项,然后提交,但,我们也可以显示的控制工作项.
		显示的控制工作项:
			在某些情况下(内存不足等),TrySumbitThreadPoolCallback函数调用可能会失败,当我们不能接收失败的情况时,我们可以显示的控制工作项;
			##1:定义回调函数:
				VOID CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_WORK Work);
					//@pInstance:同上,描述函数执行完毕返回后的操作.
					//@pvContext:同上
					//@work:
			##2:创建工作项:
				PTP_WORK CreateThreadpoolWork(PTP_WORK_CALLBACK pfnWorkHandler,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
					//@pfnWorkHandler:回调函数指针
					//@pvContext:传递给回调函数pvContext值
					//pcbe:
			##3:提交请求:
				VOID SumbitThreadpoolWork(PTP_WORK pWork);
					//由于这里返回值是void,不需要判别是否成功加入队列.我们可以默认为成功了
			##4:取消提交的工作项:
				VOID WaitForThreadpoolWorkCallbacks(PTP_WORK pwork,BOOL bCancelPendingCallbacks);
					//@pwork:指定对象
					//@bCancelPendingCallbacks:如果为true,若工作项仍在等待,则会被取消(仅仅向工作项添加取消标记);若正在执行,则在(当前工作项)执行完毕之后返回.
											   若为false,该函数的调用线程会被挂起,直到工作项已经执行完毕(等到所有提交的工作项执行完毕)
			##5:删除工作项:
				VOID CloseThreadpoolWork(PTP_WORK pwork);
					//@pwork:指定删除的work对象

			tips:
				1:显示请求的回调函数比匿名请求的回调函数多了一个PTP_WORK参数,其他步骤基本相同
				2:当多次提交工作项的时候(work),那么回调函数被调用的时候,传入的pvContext值是相同的,是在创建work过程中指定的值
				3:如果一个work对象提交了多个工作项,且调用WaitForThreadpoolWorkCallbacks时,传递参数为false,那么会等待所有的工作项执行完毕;若是ture,只等待当前工作项执行完毕
;	*/

	namespace use_thread_pool_by_async_call{

		HANDLE mutex_for_sync_print = CreateMutex(nullptr, false, nullptr);

		VOID NTAPI my_callback(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext)
		{
			using namespace general::general_func;
			DWORD id = GetCurrentThreadId();
			DWORD main_id = *(DWORD*)pvContext;
			sync_print(mutex_for_sync_print, "#", id, ": my_callback called and the main_id:",main_id);
			sync_print(mutex_for_sync_print, "#", id, ": exit");
		}

		VOID CALLBACK work_callback(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext, PTP_WORK work)
		{
			using namespace general::general_func;
			DWORD id = GetCurrentThreadId();
			DWORD main_id = *(DWORD*)pvContext;
			sync_print(mutex_for_sync_print, "#", id, ": work called and the main_id:", main_id);
			sync_print(mutex_for_sync_print, "#", id, ": exit");
		}

		void _test()
		{
			using namespace general::general_func;
			DWORD id = GetCurrentThreadId();
			general::general_func::sync_print(mutex_for_sync_print, "#", id, ":main thread init");
			bool flag = TrySubmitThreadpoolCallback(my_callback, &id, nullptr);
			if (!flag)
			{
				sync_print(mutex_for_sync_print, "#", id, ":sumbit failed by error code ", GetLastError());
			}

			PTP_WORK work = CreateThreadpoolWork(work_callback, &id, nullptr);
			SubmitThreadpoolWork(work);
			CloseThreadpoolWork(work);
			CloseHandle(mutex_for_sync_print);
			sync_print(mutex_for_sync_print, "#", id, ":exit");
			
		}
	}


	/*
		#2:每隔一段时间调用一个函数
			##1:定义一个回调函数:
				VOID CALLBACK TimeoutCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_TIMER pTimer);
			##2:创建PTP_TIMER对象
				PTP_TIMER CreateThreadpoolTimer(PTP_TIMER_CALLBACK pfnTimerCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
			##3:向线程池注册
				VOID SetThreadpoolTimer(PTP_TIMER pTimer,PFILETIME pftDueTime,DWORD msPeriod,DWORD msWindowLength);
					//@pTimer:
					//@pftDueTimer:第一次触发的时间
					//@msPeriod:第一次触发之后再次触发的事件间隔
					//@msWindowLength:随机延长时间,用于将触发间隔时间延长至[msPeriod,msPeriod+msWindowLength]时间段中

			tips:这段代码的流程与上述的工作项十分相似.
			learn:
				#1:在回调函数在间隔时间不断执行的时候,进程空间必须保留.即在某一时刻,必须有一个存活线程,使得进程有效.这里主线程通过Sleep挂起了自己

	*/


	namespace use_thread_pool_by_timer {

		using namespace std;
		using namespace general::general_func;
		

		struct my_msg {
			DWORD thread_id;
			string data;
		};



		using pmy_msg = my_msg * ;

		HANDLE mutex_for_sync_print = CreateMutex(nullptr, false, nullptr);
		

		VOID CALLBACK my_timer_callback(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext, PTP_TIMER pTimer)
		{
			DWORD id=GetCurrentThreadId();
			my_msg msg = *(pmy_msg)(pvContext);
			sync_print(mutex_for_sync_print, "#", id, " called and recv data :", msg.data, " from thread:#", msg.thread_id);
			sync_print(mutex_for_sync_print, "#", id, " exit");
		}

		void _test()
		{
			auto thread_id = GetCurrentThreadId();
			my_msg msg{ thread_id," hello first thread_pool_timer" };
			PTP_TIMER ptp_timer = CreateThreadpoolTimer(my_timer_callback, &msg, nullptr);

			//SYSTEMTIME sys_ti;
			FILETIME fl_ti;
		/*	sys_ti.wMonth = 2019;
			sys_ti.wMonth = 1;
			sys_ti.wDay = 11;
			sys_ti.wHour = 17;
			sys_ti.wMinute = 2;
			sys_ti.wSecond = 0;
			sys_ti.wMilliseconds = 0;
			sys_ti.wDayOfWeek = 0;
			SystemTimeToFileTime(&sys_ti, &fl_ti);*/

			LARGE_INTEGER li_ti;
			li_ti.QuadPart = -1;
			fl_ti.dwLowDateTime = li_ti.LowPart;
			fl_ti.dwHighDateTime = li_ti.HighPart;

			DWORD period_s = 5 * 1000;

			SetThreadpoolTimer(ptp_timer, &fl_ti, period_s, 0);
			Sleep(60 * 1000);
			sync_print(mutex_for_sync_print, "#", thread_id, ":close ptp_timer");
			CloseThreadpoolTimer(ptp_timer);
			sync_print(mutex_for_sync_print, "#", thread_id, ":exit");
			CloseHandle(mutex_for_sync_print);
		}


	}

	/*
		#3:在内核对象触发的时候调用函数
			##1:回调函数原型:
				VOID  CALLBACK WaitCallBack(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_WAIT wait,TP_WAIT_RESULT WaitResult);
					//@WaitResult:用于标识callback被用的原因 {WAIT_OBJECT_0,WAIT_TIME_OUT,WAIT_ABANDONED_0},本身是DWORD类型
			##2:创建线程池等待对象:
				PTP_WAIT CreateThreadpoolWait(PTP_WAIT_CALLBACK pfnWaitCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
			##3:注册
				VOID SetThreadpoolWait(PTP_WAIT pWaitItem,HANDLE hObject,PFILETIME pftTimeOut);
					//@pWaitItem:##2中创建的等待对象
					//@hObject:标识等待的内核对象
					//@pftTimeOut:超时时间,nullptr表示无限长
	*/

	namespace use_thread_pool_by_wait_item {
		using namespace std;
		using namespace general::general_class;
		using namespace general::general_func;

		using pmft = msg_for_thread * ;

		HANDLE hEvent = CreateEvent(nullptr, false, false, nullptr);
		HANDLE hMutex = CreateMutex(nullptr, false, nullptr);

		//简单判断等待事件是否触发,并输出来自主线程的问候数据
		VOID CALLBACK my_wait_item_cb(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext, PTP_WAIT wait, TP_WAIT_RESULT WaitResult)
		{
			auto id = GetCurrentThreadId();
			switch (WaitResult) {
			case WAIT_TIMEOUT:
				sync_print(hMutex, "#", id, " start:time_out");
				break;
			case WAIT_ABANDONED_0:
				sync_print(hMutex, "#", id, " start:mutex had been abandoned");
				break;
			default:
				sync_print(hMutex, "#", id, "start:event set");
				msg_for_thread mft = *(pmft)(pvContext);
				sync_print(hMutex, "#", id, ":\n recv data:", mft.data, "\n from thread:#", mft.tid_send);
				break;
			}

			sync_print(hMutex, "#", id, ":exit");

			CloseHandle(hEvent);
			Sleep(1000);	//确保主线程正确退出并清理互斥量
			CloseHandle(hMutex);

		}

		void _test()
		{
			auto id = GetCurrentThreadId();
			msg_for_thread mft{ id,"hello first wait_item" };
			PTP_WAIT ptp_wait = CreateThreadpoolWait(my_wait_item_cb, &mft, nullptr);
			SetThreadpoolWait(ptp_wait,hEvent,nullptr);
			sync_print(hMutex, "#", id, ":sleep for 30sec and set the event");
			Sleep(30 * 1000);
			SetEvent(hEvent);
			sync_print(hMutex, "#", id, ":waked and set the event");
			sync_print(hMutex, "#", id, ":exit");

		}
	}

	/*
		#4:在异步IO请求完成时调用一个函数
			##1:回调函数原型
				VOID CALLBACK OverlappedCompletionRoutine(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,
														  PVOID pOverlapped,ULONG ioResult,ULONG_PTR numberOfByteTransfered,PTP_IO pio)
					//@ioResult:io结果 NO_ERROR表示成功
					//@pid:线程池IO对象
			##2:创建线程池IO对象:
				PTP_IO CreateThreadpoolIo(HANDLE hDevice,PTP_WIN32_IO_CALLBACK pfnIoCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
			##3:注册:
				VOID StartThreadpoolIo(PTP_IO pio);
			##4:取消调用:
				VOID CancelThreadpoolIo(PTP_IO pio);
			##5:释放:
				VOID CloseThreadpoolIo(PTP_IO pio);

			tips:
				#1:在读取和写入操作之前(比如说ReadFile/WriteFile),必须先调用StartThreadpoolIo函数,否则回调不会执行
				#2:
	*/

	namespace use_thread_pool_by_io {

		using namespace std;
		using namespace general::const_val;
		using namespace general::general_class;
		using namespace general::general_func;
		using pmft = msg_for_thread * ;
		using pmy_overlapped = my_overlapped * ;

		HANDLE hMutex = CreateMutex(nullptr, false, nullptr);
		const char* file_name = "first.txt";
		char buf[buf_sz];


		VOID CALLBACK my_pio_cb(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PVOID pOverlapped,ULONG IoResult,ULONG_PTR ByteTransfered,PTP_IO pio) 
		{
			auto id = GetCurrentThreadId();
			msg_for_thread mft = *(pmft)(pvContext);
			sync_print(hMutex, "#", id, ":\nrecv data:", mft.data, "\nfrom thread:#", mft.tid_send);
			if (IoResult != NO_ERROR)
			{
				DWORD dw = GetLastError();
				sync_print(hMutex, "#", id, ":io_operaion failed by", dw," ...exit!");
				CloseHandle(hMutex);
				return;
			}

			my_overlapped mo = *(pmy_overlapped)(pOverlapped);
			switch (mo.m_op) {
			case io_op::op_read:
				sync_print(hMutex, "#", id, ":read ", ByteTransfered, " done\n", buf);
				break;
			case io_op::op_write:
				sync_print(hMutex, "#", id, ":write ", ByteTransfered, " done");
				break;
			default:
				sync_print(hMutex, "#", id, ":recv kill cmd...exit");
				CloseHandle(hMutex);
				return;
			}

			sync_print(hMutex, "#", id, ":exit...normal");
			CloseHandle(hMutex);
			

		}

		void _test()
		{
			auto id = GetCurrentThreadId();
			HANDLE hFile = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				sync_print(hMutex, "#", id, ":open file(", file_name, ") failed...exit");
				CloseHandle(hMutex);
				return;
			}
			msg_for_thread mft{ id,"hello first thread_pool_io" };
			PTP_IO ptp_io = CreateThreadpoolIo(hFile, my_pio_cb, &mft, nullptr);
			StartThreadpoolIo(ptp_io);

			DWORD bytes_to_read{256};
			my_overlapped mo{ io_op::op_read };
			bool flag=ReadFile(hFile, buf, bytes_to_read, nullptr, &mo);
			DWORD error_code = GetLastError();
			if (!flag&&error_code != ERROR_IO_PENDING)
			{
				sync_print(hMutex, "#", id, ":read_file failed by error_code ", error_code,"...exit");
				return;
			}
			sync_print(hMutex, "#", id, ":exit...normal");
		}
	}

	/*
		#5:定制线程组
			目前为止我们使用的都是默认的线程池,其生命周期与进程一致,操作系统会自动的进行资源管理操作.但我们也可以使用自定义的线程池
				#1:创建新的线程池:
					PTP_POOL CreateThreadpool(PVOID reserved);
				#2:指定线程池的最大最小线程数量(默认最小为1,最大为500)
					BOOL SetThreadpoolThreadMininum(PTP_POOL pThreadPool,DWORD cthrdMin);
					BOOL SetThreadpoolThreadMaxinum(PTP_POOL pThreadPool,DWORD cthrdMax);
				#3:销毁线程池:
					VOID CloseThreadpool(PTP_POOL pThreadPool);
				#4:定制回调环境,即上边的_TP_CALLBACK_ENVIRON参数
					##1:初始化:
						VOID InitializeThreadpoolEnvironment(PTP_CALLBACK_ENVIRON pcbe);
					##2:设置线程池回调环境
						VOID SetThreadpoolCallbackPool(PTP_CALLBACK_ENVIRON pcbe,PTP_POOL pThreadPool);
					##3:若预测线程需要长时间的处理时间,则设置回调环境使用更公平的调度
						VOID SetThreadpoolCallbackRunsLong(PTP_CALLBACK_ENVIRON pcbe);
					##4:若线程池仍有待处理项,设置特定的dll在进程的地址空间中
						VOID SetThreadCallbackLibrary(PTP_CALLBACK_ENVIRON pcbe,PVOID mod);
					##5:清理:
						VOID DestroyThreadpoolEnvironment(PTP_CALLBACK_ENVIRON pcbe);
				#5:使用清理组来销毁线程池:
					此节只适用于自定义的线程池.
					##1:创建销毁组:
						PTP_CLEANUP_GROUP CreateThreadpoolCleanupGroup();
					##2:当清理组被取消时调用的回调函数原型:	
						VOID CALLBACK CleanupGroupCancelCallback(PVOID pvObjectContext,PVOID pvCleanupContext);
					##3:关联pcbe回调环境结构:
						VOID SetThreadpoolCallbackCleanupGroup(PTP_CALLBACK_ENVIRON pcbe,PTP_CLEANUP_GROUP ptpcg,PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfng);
							//@ptpcg:清理组对象,用于设置回调环境中的CleanupGroup字段
							//@pfng:##2中的回调函数,在取消清理组的时候被调用,用于设置回调函数中的CleanupGroupCancelCallback字段
					##4:销毁线程池:
						VOID CloseThreadpoolCleanupGroupMembers(PTP_CLEANUP_GROUP ptpcg,BOOL bCancelPendingCallbacks,PVOID pvCleanupContext);
							//机制十分类似于WaitForThreadpoolWork()函数
					##5:销毁清理组:
						VOID WINAPI CloseThreadpoolCleanupGroup(PTP_CLEANUP_GROUP ptpcg);

	*/


	/*
		example:
			主线程启动后进行30s倒计时;
			使用ptp_timer对象调用倒计时函数(time_count_down)每秒打印倒计时语句;
			倒计时完毕,主线程释放ptp_timer对象,并使用ptp_io进行io操作,完成后调用(io_down)输出io事件(这里是读取事件,输出读取内容);
			io_down完毕后会触发事件,导致close_global_val函数被调用,进行全局变量的清理之后退出

		learn:
			#1:不同的ptp_*对象,得可以在回调函数中得到释放(is_this_right?);
	*/


	namespace unit_11_example {

		using namespace std;
		using namespace general::general_class;
		using namespace general::general_func;
		using namespace general::const_val;

		long g_left{ 30 };
		char buf[buf_sz];
		const char* file_name = "first.txt";
		HANDLE mutex_for_sync_print = CreateMutex(nullptr, false, nullptr);
		HANDLE hFile = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, nullptr);
		HANDLE hEvent = CreateEvent(nullptr, false, false, nullptr);

		//用于打印倒计时
		VOID CALLBACK time_count_down(PTP_CALLBACK_INSTANCE pInstance, PVOID pvContext,PTP_TIMER ptp_timer)
		{
			auto id = GetCurrentThreadId();
			if (g_left < 10) {
				sync_print(mutex_for_sync_print, "#", id, ":0", g_left, " sec left");
			}
			else {
				sync_print(mutex_for_sync_print, "#", id, ":", g_left, " sec left");
			}
			//InterlockedExchangeAdd(&g_left, -1);
			--g_left;
		}

		//手动关闭全局资源
		VOID CALLBACK close_global_val(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_WAIT ptp_wait,TP_WAIT_RESULT wait_result)
		{
			auto id = GetCurrentThreadId();
			sync_print(mutex_for_sync_print, "#", id, ":start to cleanup global val");
			CloseHandle(hFile);
			CloseHandle(hEvent);
			CloseHandle(mutex_for_sync_print);
			CloseThreadpoolWait(ptp_wait);
			cout << "#" << id << ":exit"; 
		}

		//当异步io完成时调用函数
		VOID CALLBACK io_done(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PVOID pOverlapped,ULONG IoResult,ULONG_PTR  bytes_transferred,PTP_IO pio)
		{
			auto id = GetCurrentThreadId();

			if (IoResult != NO_ERROR)
			{
				DWORD dw = GetLastError();
				sync_print(mutex_for_sync_print, "#", id, ":io failed by error code ", dw);
				return;
			}

			my_overlapped mo = *(my_overlapped*)(pOverlapped);
			switch (mo.m_op)
			{
			case io_op::op_read:
				sync_print(mutex_for_sync_print, "#", id, ":read ", bytes_transferred, " done\n", buf);
				break;
			case io_op::op_write:
				sync_print(mutex_for_sync_print, "#", id, ":write ", bytes_transferred, " done");
				break;
			default:
				sync_print(mutex_for_sync_print, "#", id, ":exit by recv kill cmd");
				return;
			}
			CloseThreadpoolIo(pio);
			SetEvent(hEvent);
			sync_print(mutex_for_sync_print, "#", id, ":exit.");
		}

		void _test()
		{
			auto id = GetCurrentThreadId();
			PTP_TIMER ptp_ti = CreateThreadpoolTimer(time_count_down, nullptr, nullptr);
			LARGE_INTEGER li_ti;
			li_ti.QuadPart = -1;
			FILETIME fi_ti;
			fi_ti.dwHighDateTime = li_ti.HighPart;
			fi_ti.dwLowDateTime = li_ti.LowPart;
			DWORD period_s = 1 * 1000;
			SetThreadpoolTimer(ptp_ti, &fi_ti, period_s, 0);
			Sleep(30*1000);
			CloseThreadpoolTimer(ptp_ti);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				sync_print(mutex_for_sync_print, "#", id, ":open file failed by error code ",GetLastError());
				return;
			}
			DWORD bytes_to_read = 256;
			my_overlapped mo{ io_op::op_read };
			PTP_IO ptp_io = CreateThreadpoolIo(hFile, io_done, nullptr, nullptr);
			StartThreadpoolIo(ptp_io);
			bool flag=ReadFile(hFile, buf, bytes_to_read, nullptr, &mo);
			DWORD dw = GetLastError();
			if (!flag&&dw != ERROR_IO_PENDING)
			{
				sync_print(mutex_for_sync_print, "#", id, ":read failed by error code", dw);
				return;
			}

			PTP_WAIT ptp_wait = CreateThreadpoolWait(close_global_val, nullptr, nullptr);
			SetThreadpoolWait(ptp_wait, hEvent, nullptr);
			sync_print(mutex_for_sync_print, "#", id, ":exit");
		}
	}

}