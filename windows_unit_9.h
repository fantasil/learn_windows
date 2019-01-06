#pragma once
#include<Windows.h>
#include<iostream>

#define DEBUG

namespace windows_unit_9
{

	/*
		使用内核对象进行线程同步
		内核对象拥有{触发/非触发}两种状态.
	*/

	/*
		等待系列函数:
			线程资源进入等待状态,直到一个内核对象处于触发状态.

		DWORD WaitForSingleObject(HANDLE hObject,DWORD dwMillseconds);
			//指定等待对象hObject和等待时间dwMillseconds;
			//返回值{ --WAIT_OBJECT_0:对象处于触发状态;
					  --WAIT_TIMEOUT:等待超时;
					  --WAIT_FAILED:句柄无效}
		DWORD WaitForMultipleObkect(DWORD dwCount,CONST HANDLE* phObject,BOOL bWaitAll,DWORD dwMillseconds);
			//指定等待多个对象(dwCount指定对象数目,phObject是一个handle数组,bWaitAll指明是否等待全部对象触发,dwMillseconds指定等待时间)

		tips:
		等待函数的副作用:
			对于某些内核对象来说,等待函数成功调用之后会改变对象的状态(触发,非触发).如自动重置事件
	*/
	namespace wait_for_funcs {

		struct handle_wrapper {
			CONST HANDLE* phandle;
			DWORD dwCount;
		};

		long g_x{}, g_y{}, g_z{}, g_m{}, g_n{};
		DWORD WINAPI func1(PVOID pvParam)
		{
			g_y = g_x + 50;
			std::cout << "g_y=" << g_y << "\n";
			return 0;
		}

		//func2依赖于func1执行完毕
		DWORD WINAPI func2(PVOID pvParam)
		{
			HANDLE handle = (HANDLE)(pvParam);
			WaitForSingleObject(handle, INFINITE);
			g_z = g_y * 5;
			std::cout << "g_z=" << g_z << "\n";
			return 0;
		}

		//func3和func1无依赖,可以并行
		DWORD WINAPI func3(PVOID pvParam)
		{
			g_m = 200;
			std::cout << "g_m=" << g_m << "\n";
			return 0;
		}

		//func4依赖于fun2和func3执行完毕
		DWORD WINAPI func4(PVOID pvParam)
		{
			handle_wrapper* hw = (handle_wrapper*)pvParam;
			WaitForMultipleObjects(hw->dwCount, hw->phandle, true, INFINITE);
			g_n = g_z + g_m;
			std::cout << "g_n=" << g_n << "\n";
			return 0;
		}

		void _test()
		{
			DWORD id1, id2, id3, id4;
			auto hd1 = CreateThread(nullptr, 0, func1, nullptr, 0, &id1);
			
			auto hd2 = CreateThread(nullptr, 0, func2, hd1, 0, &id2);
			
			auto hd3 = CreateThread(nullptr, 0, func3, hd2, 0, &id3);

			HANDLE handle_arr[2] = { hd2,hd3 };
			handle_wrapper hw{ handle_arr,2 };

			auto hd4 = CreateThread(nullptr, 0, func4, &hw, 0, &id4);
			Sleep(200);

		}
	}


	/*
		事件内核对象:
			功能:事件完成时触发,唤醒等待线程.{自动重置事件:被触发时,只有一个线程变成可调度状态}
										   手动重置事件:被触发时,所有等待线程变成可调度状态
			常用作用:让一个线程执行初始化过程,然后再触发另外一个线程.
			创建:
				HANDLE CreateEvent(PSECURITY_ATTRIBUTES psa,BOOL bManualReset,BOOL bInitialState,PCTSTR pszName);
				HANDLE CreateEventEx(PSECURITY_ATTRIBUTES psa,PCTSTR pszName,DWORD dwFlags,DWORD dwDesiredAccess);
					//dwFlag{CREATE_EVENT_INITIAL_SET:初始状态为触发,同bInitialState参数作用} 
					        {CREATE_EVENT_MANUAL_RESET:设为手动重置事件,同bManualReset参数作用}\
					//dwDesiredAccess{指定对事件的访问权限}

			访问:
				HANDLE OpenEvent(DWORD dwDesiredAccess,BOOL bInherit,PCSTR pszName);
			设置状态:
				BOOL SetEvent(HANDLE hEvent);	//设置成触发状态
				BOOL ResetEvent(HANDLE hEvent);	//设置成未触发状态
			副作用:
				自动重置事件触发之后,在线程等待成功之后被调度之前,该事件会被操作系统自动设置为未触发状态.

	*/	

	namespace event_handle {

		class event_wrapper {
		public:
			event_wrapper(LPCSTR pscName, DWORD dwFlags) { m_event = CreateEventEx(nullptr, pscName, dwFlags, EVENT_MODIFY_STATE); }
			~event_wrapper() { CloseHandle(m_event); }
			bool set_event() { return SetEvent(m_event); }
			bool reset_event() { return ResetEvent(m_event); }
			HANDLE native_handle() { return m_event; }
		private:
			HANDLE m_event;
		};

		event_wrapper ew{ "bigTree",0 };
		int arr[10];

		DWORD WINAPI func1(PVOID pvParam)
		{
			std::cout << "init int array\n";
			for (int i = 0; i < 10; ++i)
			{
				arr[i] = i * 2;
			}
			ew.set_event();
			return 0;

		}

		DWORD WINAPI func2(PVOID pvParam)
		{
			WaitForSingleObject(ew.native_handle(), INFINITE);
			ew.reset_event();
			std::cout << "read from array\n";
			for (int i = 0; i < 10; ++i)
			{
				std::cout << arr[i];
			}
			return 0;
		}

		void _test()
		{
			DWORD id1, id2;
			CreateThread(nullptr, 0, func1, nullptr, 0, &id1);
			CreateThread(nullptr, 0, func2, nullptr, 0, &id2);

		}

	}



	/*
		可等待的计时器内核对象:
			功能:在某个指定的事件触发,或者每隔一个时间段触发
			创建:
				HANDLE CreateWaitableTimer(PSECURITY_ATTRIBUTES psa,BOOL bManualReset,PCTSTR pszName);
			访问:
				HANDLE OpenWaitableTimer(DWORD dwDesiredAccess,BOOL bInheritHandle,PCSTR pszName);
			设置:
				BOOL SetWaitableTimer(HADNLE hTimer,const LARGE_INTEGER* pDueTime,
									  long lPeriod,PTIMERAPCROUTINE pfnCompletionRoutine,
									  PVOID pvArgToCompletionRoutine,BOOL bResumed);
				//@pDueTime:第一次触发的时间,基于UTC时间
				//@lPeriod:触发间隔
				//@pfnCompletionRoutine:异步过程调用函数
				//@pvArgToCompletionRoutine:异步过程函数参数
				
	*/

	namespace waitable_timer {

		class timer_wrapper {
		public:
			timer_wrapper() { m_timer = CreateWaitableTimer(nullptr, false, nullptr); }
			~timer_wrapper() { CloseHandle(m_timer); }
			HANDLE native_handle() { return m_timer; }
		private:
			HANDLE m_timer;
		};

		std::ostream& operator<<(std::ostream& os, SYSTEMTIME& st)
		{
			os << st.wYear << "-";
			if (st.wMonth < 10)
				os << "0" << st.wMonth<<"-";
			else
				os << st.wMonth<<"-";
			if (st.wDay < 10)
				os << "0" << st.wDay<<" ";
			else
				os << st.wDay<<" ";
			if (st.wHour < 10)
				os << "0" << st.wHour << "::";
			else
				os << st.wHour << "::";
			if (st.wMinute < 10)
				os << "0" << st.wMinute << "::";
			else
				os << st.wMinute << "::";
			if (st.wSecond < 10)
				os << "0" << st.wSecond << "\n";
			else
				os << st.wSecond << "\n";
			return os;
		}

		VOID APIENTRY APC_Route(PVOID pvArgs, DWORD dwTimerLow, DWORD dwTimerHigh)
		{
			SYSTEMTIME st;
			FILETIME ftUTC, ftLocal;
			ftUTC.dwHighDateTime = dwTimerHigh;
			ftUTC.dwLowDateTime = dwTimerLow;
			FileTimeToLocalFileTime(&ftUTC, &ftLocal);
			FileTimeToSystemTime(&ftLocal, &st);
			std::cout << "ARP_ROUTE called at:" << st << std::endl;
		}

		timer_wrapper tw{ };
		

		//此函数设置一个计时器对象
		/*
			#1:SYSTEMTIME st;FILETIME ftLocal,ftUTC;	//st是本地系统时间,ftLocal是本地文件格式时间,ftUTC是文件格式的UTC时间
			#2:设置st指定响应时间并转化成ftLocal			//SystemTimeToFileTime(&st,&ftLocal);
			#3:将flLocal转化成标准UTC时间					//LocalFileTimeToFileTime(&ftLocal,&ftUTC);
			#4:将ftUTC转化成LARGE_INTEGER格式表示			//liUTC.lowPart=ftUTC.dwLowDateTime;liUTC.highPart=ftUTCDateTime;
			#5:可以直接赋值 liUTC.QuadPart=xxxx;xxxx是一个基于100ns整数倍的数值,如果xxxx为正,表示绝对时间;否则为相对时间;
			   即liUTC.QuadPart=-10*1000*1000*10;表示执行10s之后的时间.

			tips:
			   虽然LARGE_INTEGER和FILETIME的二进制格式相同,但是对齐方式一样(FILETIME32位对齐,LARGE_INTEGER64位对齐),
			   所以采用lowPart=lowDateTime方式,而不是进行强制转换(liUTC=(LARGE_INTEGER)ftUTC;);

			#6 APC:异步过程调用
				1:异步过程调用将函数放在调用SetWaitableTimer函数的线程队列中,在触发时有该线程调用
				2:该线程必须是可提醒状态,这是由线程调用SleepEx,WaitForSingleObjectEx,
													WaitForMultipleObjectsEx,MsgWaitForMultipleObjectsEx,
													SignalObjectAndWait而进入的等待状态
				3:当在同一线程中调用SetWaitableTimer函数,讲APC加入队列,并等待该计时器对象时.
					当计时器对象被调用,线程就退出了可提醒状态,APC函数并不会执行.
					//不要再同一线程中等待相同的内核对象
			
		*/
		void time_set(HANDLE& handle)
		{
			
		
			SYSTEMTIME st;
			FILETIME ftLocal, ftUTC;
			LARGE_INTEGER liUTC;

			st.wYear = 2019;
			st.wMonth = 1;
			st.wDay = 5;
			st.wHour = 17;
			st.wMinute = 6;
			st.wSecond = 0;
			st.wDayOfWeek = 0;
			st.wMilliseconds = 0;
			bool flag = false;
			flag=SystemTimeToFileTime(&st, &ftLocal);
			if (!flag)
				std::cout << "system_time to file_time failed\n";
			flag=LocalFileTimeToFileTime(&ftLocal, &ftUTC);
			if (!flag)
				std::cout << "local_file_tile to file_tile failed\n";
			liUTC.LowPart = ftUTC.dwLowDateTime;
			liUTC.HighPart = ftUTC.dwHighDateTime;
			SYSTEMTIME st_utc;
			FileTimeToSystemTime(&ftUTC, &st_utc);
			std::cout << st_utc << std::endl;
			SetWaitableTimer(handle, &liUTC, 0, APC_Route, nullptr, false);
		}

		

		DWORD WINAPI func1(PVOID pvParam)
		{
			HANDLE handle = (HANDLE)pvParam;
			
			
			auto rtc=WaitForSingleObject(handle, INFINITE);
			if (rtc == WAIT_OBJECT_0)
			{
				std::cout << "wait success\n";
				std::cout << "timer is set\n";
			}
			else {
				std::cout << "wait failed\n";
				DWORD err=GetLastError();
				std::cout << err << std::endl;
			}
			
			
			return 0;
		}

		

		void _test()
		{
			auto timer = tw.native_handle();
			time_set(timer);
			DWORD id;
			auto hd=CreateThread(nullptr, 0, func1, timer, 0, &id);
			//WaitForSingleObject(hd, INFINITE);
			SleepEx(INFINITE, true);
		}
	}


	/*
		信号量内核对象
			创建:
				HANDLE CreateSemaphore(PSECURITY_ATTRIBUTES psa,LONG lInitialCount,Long lMaxinumCount,PCTSTR pszName);
					//@lInitialCount:当前资源计数
					//@lMaxinumCount:最大资源计数
				HANDLE CreateSemaphoreEx(PSECURITY_ATTRIBUTES pas,LONG lInitialCount,LONG lMaxinumCount,PCTSTR pszName,DWORD dwflags,DWORD dwDesireAccess);
					//@dwFlags:系统保留 始终为0
					//@dwDesireAccess:指定访问权限
			递增当前资源计数:
				BOOL ReleaseSemaphore(HADNLE hSemaphore,LONG lReleaseCount,PLONG plPreviousCount);
					//@lReleaseCount:递增的数量
					//@plPreviousCount:递增前的计数
			tips:
				目前没有办法在不改变资源计数值的情况下获取资源计数值

			基本流程:
				#1:创建并指定最大资源计数和当前可用资源计数
				#2:如果可用资源计数大于0,则处于触发状态,等待线程可以获得调度;
				#3:线程离开等待状态,可用资源计数-1,线程开始执行.
				#4:直到可用资源为0,信号处于非触发状态,线程进入等待
				#5:线程可以通过ReleaseSemaphore函数增加可用资源计数.
	*/

	namespace semaphore_unit {

		class semaphore_wrapper {
		public:
			semaphore_wrapper(long cur, long max) { m_semaphore = CreateSemaphore(nullptr, cur, max, nullptr); }
			void release_count(long add_count, long* previous) { ReleaseSemaphore(m_semaphore, add_count, previous); }
			HANDLE native_handle() { return m_semaphore; }
			~semaphore_wrapper() { CloseHandle(m_semaphore); }
		private:
			HANDLE m_semaphore;
		};


		semaphore_wrapper sw{ 0,2 };


		DWORD WINAPI release_count(PVOID pvParam)
		{
			std::cout << "release_count\n";
			sw.release_count(2, nullptr);
			return 0;
		}

		DWORD WINAPI func1(PVOID pvParam)
		{
			WaitForSingleObject(sw.native_handle(), INFINITE);
			std::cout << "func1 start to sleep 10s\n";
			Sleep(10 * 1000);
			std::cout << "func1 end\n";
			return 0;
		}

		DWORD WINAPI func2(PVOID pvParam)
		{
			WaitForSingleObject(sw.native_handle(), INFINITE);
			std::cout << "func2 start to sleep 5s\n";
			std::cout << "func2 release 1\n";
			sw.release_count(1, nullptr);
			Sleep(5 * 1000);
			std::cout << "func2 end\n";
			return 0;
		}

		DWORD WINAPI func3(PVOID pvParam)
		{
			WaitForSingleObject(sw.native_handle(), INFINITE);
			std::cout << "func3 start to sleep 3s\n";
			Sleep(3 * 1000);
			std::cout << "func3 end\n";
			return 0;
		}

		void _test()
		{

			DWORD id1, id2, id3, id4;
			CreateThread(nullptr, 0, func1, nullptr, 0, &id1);
			CreateThread(nullptr, 0, func2, nullptr, 0, &id2);
			CreateThread(nullptr, 0, func3, nullptr, 0, &id3);
			CreateThread(nullptr, 0, release_count, nullptr, 0, &id4);

			Sleep(15 * 1000);

		}



	}


	/*
		互斥量:
			创建:
				HANDLE CreateMutex(PSECURITY_ATTRIBUTES psa,BOOL bInitialOwner,PCTSTR pszName);
					//@bInitialOwner:控制初始状态,false表示互斥量不为任何线程所占有
				HANDLE CreateMutexEx(PSECURITY_ATTRIBUTES psa,PCTSTR pszName,DWORD dwFlags,DWORD dwDesiredAccess);
					//@dwFlags:替代了bInitialOwner,0表示false,CREATE_MUTEX_INTIAL_OWNER为true;
			访问:
				HANDLE OpenMutex(DWORD dwDesiredAccess,BOOL bInheritHandle,PCTSTR pszName);
			线程释放:(非析构对象)
				BOOL ReleaseMutex(HANDLE hMutex);

			工作概念:
				互斥量对象包含一个使用计数,线程ID,以及递归计数.
				线程ID表示当前正在占用互斥量的线程,0表示没有被占用,此时互斥量处于触发状态;非0表示被某一个线程所占用.
				线程占用互斥量时,会将递归计数记为1,当同一个线程试图等待相同的互斥量时,该递归计数会增加;
				互斥量的释放只是让递归计数减1,所以获取互斥量并多次等待的线程,需要多次释放互斥量.直到计数为0,互斥量重新处于触发状态.
				若一个占用互斥量的线程在未释放互斥量之前结束了(ExitThread等),那么该互斥量就被遗弃了,直到系统自动重新设置互斥量的状态.

			工作流程:
				类似于关键段的,但有几处不同.p256表9-2;


	*/


	namespace mutex_unit {

		//一个基于mutex&semaphore的简单同步队列
		//tips:这里的可读可写都需要改变队列的状态.
		/*
			learn:
				#1:当需要同时获取多个内核对象的时候,使用WaitForMultipleObjects而不是分别使用WaitForSingleObject,后者会产生死锁
					//初始时,当可读线程先获取了mutex,但不能获取可读信号(初始不可读,但可写),进入等待;可写线程获取不了mutex亦进入等待.
				#2:内核对象释放的顺序会对其他线程产生影响.
					//由于开始入队时可读信号最后释放,出队时可写信号最后释放,导致首先一直执行入队,然后才执行出队操作.
				#3:这里和用户同步有稍许不同的地方
					//用户模式下如果队列空,则可读等待;这里是获取可读信号(可进入等待),操作完成后再执行对信号的调整.
				#4:考虑过是否使用bool表示可读可写.
					//答案是不,因为我们需要在条件不满足的时候使线程进入等待状态,bool变量做不到,需要使用内核对象
		*/
		class sync_queue {
		public:
			sync_queue(int capacity) :m_capacity{ capacity }, m_front{ 0 }, m_back{ 0 }
			{
				m_element = new int[m_capacity];
				m_mtx = CreateMutex(nullptr, false, nullptr);
				//可读信号,最大计数为1,当前计数为0,未触发,没有数据可读;为1表示有数据可读;初始不可读
				m_semaphore_for_read = CreateSemaphore(nullptr, 0, 1, nullptr);
				//可写信号,最大计数为1,当前计数为1,可触发,当前可写;为0表示没有内存位置可写数据;初始可写
				m_semaphore_for_write = CreateSemaphore(nullptr, 1, 1, nullptr);
				
			}

			~sync_queue()
			{
				CloseHandle(m_mtx);
				CloseHandle(m_semaphore_for_read);
				CloseHandle(m_semaphore_for_write);
			}

			inline bool empty() const { return m_front == m_back; }
			inline bool full() const 
			{
				int back_next = m_back + 1;
				if (back_next%m_capacity == m_front)
					return true;
				return false;
			}
			void enqueue(const int& val)
			{
				//入队时,需要同时获取互斥量和可写信号
				const HANDLE handle_arr[2] = { m_mtx,m_semaphore_for_write };
				WaitForMultipleObjects(2, handle_arr, true, INFINITE);
				//WaitForSingleObject(m_mtx, INFINITE);
				//WaitForSingleObject(m_semaphore_for_write, INFINITE);
#ifdef DEBUG
				std::cout << "in:" << val << "\n";
#endif // DEBUG

				m_element[m_back] = val;
				m_back = (++m_back) % m_capacity;
				ReleaseSemaphore(m_semaphore_for_read, 1, nullptr);	//数据插入之后,触发可读信号
				if (!full())
				{
					ReleaseSemaphore(m_semaphore_for_write, 1,nullptr);	//如果队列未满,重新触发可写信号
				}
				ReleaseMutex(m_mtx);
				
			}

			void dequeue(int& front)
			{
				//出队时,需要同时获取互斥量和可读信号
				const HANDLE handle_arr[2] = { m_mtx,m_semaphore_for_read };
				WaitForMultipleObjects(2, handle_arr, true, INFINITE);
				///WaitForSingleObject(m_mtx, INFINITE);
				//WaitForSingleObject(m_semaphore_for_read, INFINITE);
				//获取了read信号表示可读
				front = m_element[m_front];
				m_front = (++m_front) % m_capacity;
#ifdef DEBUG
				std::cout << "out:" << front << "\n";
#endif // DEBUG
				ReleaseSemaphore(m_semaphore_for_write, 1, nullptr);
				if (!empty())
				{
					ReleaseSemaphore(m_semaphore_for_read, 1, nullptr);	//如果队列不为空,重新触发可读信号
				}
				ReleaseMutex(m_mtx);
				
			}

		private:
			int * m_element;
			int m_front, m_back, m_capacity;
			HANDLE m_mtx;
			HANDLE m_semaphore_for_read;
			HANDLE m_semaphore_for_write;
			//CONDITION_VARIABLE m_cv_for_read;
			//CONDITION_VARIABLE m_cv_for_write;
			//由于条件变量只能配合CRITICAL_SECTION和SRWLock,所以这里不能使用
		};

	/*	class srwlock_wrapper {
		public:
			srwlock_wrapper() { InitializeSRWLock(&m_lock); }
			~srwlock_wrapper() {}
			void acquire_lock_exclusive() { AcquireSRWLockExclusive(&m_lock); }
			void acquire_lock_shared() { AcquireSRWLockShared(&m_lock); }
			void release_lock_exclusive() { ReleaseSRWLockExclusive(&m_lock); }
			void release_lock_shread() { ReleaseSRWLockShared(&m_lock); }
		private:
			SRWLOCK m_lock;
		};*/

		sync_queue sq{ 32 };


		DWORD WINAPI read(PVOID pvParam)
		{
			int val;
			for (int i = 0; i < 20; ++i)
			{
				sq.dequeue(val);
			}
			return 0;
		}

		DWORD WINAPI read_2(PVOID pvParam)
		{
			int val;
			for (int i = 0; i < 20; ++i)
			{
				sq.dequeue(val);
			}
			return 0;
		}

		DWORD WINAPI write(PVOID pvParam)
		{
			for (int i = 0; i < 20; i+=2)
			{
				sq.enqueue(i);
				
			}
			return 0;
		}

		DWORD WINAPI write_2(PVOID pvParam)
		{
			for (int i = 1; i < 20; i+=2)
			{
				sq.enqueue(i );
			}
			return 0;
		}

		void _test()
		{
			DWORD id1, id2, id3, id4;
			CreateThread(nullptr, 0, read, nullptr, 0, &id1);
			CreateThread(nullptr, 0, write, nullptr, 0, &id2);
			CreateThread(nullptr, 0, read_2, nullptr, 0, &id3);
			CreateThread(nullptr, 0, write_2, nullptr, 0, &id4);

			Sleep(2000);
		}


	}

	/*
		其他线程同步函数
			#1:异步设备IO:线程可以执行读取/写入操作,但是不必等待操作完成;此期间可以执行其他操作.  ?操作系统会启用新线程执行这个操作吗?
			#2:BOOL WaitForInputIdle(HANDLE hProcess,DWORD dwMilliseconds):等待hProcess进程,直到该进程的主线程没有待处理的输入为止
			#3:MsgWaitForMultipleObjects(Ex)函数:类似于WaitForMultipleObjects,但当某个窗口消息被派送到由该线程创建的窗口时,也会成为可调度状态.
			#4:WaitForDebugEvent函数:等待调试事件
			#5:SignalObjectAndWait函数:触发一个内核对象并等待另外一个内核对象;被触发的必须是{互斥量,信号量,事件),可等待的是普通内核对象;

			DWORD SignalObjectAndWait(HANDLE hObjectToSignal,HANDLE hObjectToWaitOn,DWORD dwMilliseconds,BOOL bAlertable);

			当我们需要触发一个对象,并且等待另外一个对象的时候,有可能会采用如下的代码:
				SetEvent(hEventA);
				WaitForSingleObject(hEventB,INIFNITE);
			问题在于:
				当A触发的时候,线程可能失去了时间片,所以未能执行等待B事件的代码.此时,当有另外一个线程触发B事件的时候,该线程并不能得到调度.即set/wait是两步操作,而不是原子操作;
			解决办法:
				SignalObjectAndWait(hEventA,hEventB,INFINITE,false);
	*/



	

}