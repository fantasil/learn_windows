#pragma once
#include<Windows.h>
#include<iostream>
namespace windows_unit_8 {



	//这里初始化函数总是会抛出"此类没有存储类或说明符"错误,所以这里使用了包装类

	class critical_section_wrapper {
	public:
		critical_section_wrapper() { InitializeCriticalSection(&m_cs); }
		critical_section_wrapper(DWORD times) { InitializeCriticalSectionAndSpinCount(&m_cs, times); }
		~critical_section_wrapper() { DeleteCriticalSection(&m_cs); }
		void begin() { EnterCriticalSection(&m_cs); }
		void end() { LeaveCriticalSection(&m_cs); }
		void count(DWORD times) { SetCriticalSectionSpinCount(&m_cs, times); }
	private:
		CRITICAL_SECTION m_cs;
	};

	//单独调用init函数会有一个"此类没有存储类或说明符的错误",所以这里使用了包装类.
	class srwlock_wrapper {
	public:
		srwlock_wrapper() { InitializeSRWLock(&m_lock); }
		~srwlock_wrapper() {}
		void acquire_lock_exclusive() { AcquireSRWLockExclusive(&m_lock); }
		void acquire_lock_shared() { AcquireSRWLockShared(&m_lock); }
		void release_lock_exclusive() { ReleaseSRWLockExclusive(&m_lock); }
		void release_lock_shared() { ReleaseSRWLockShared(&m_lock); }

	private:
		SRWLOCK m_lock;
	};

	/*
		windows_unit_8:用户模式下的线程同步
	*/


	/*
		原子访问:InterLocked系列函数
			原子增减:
				LONG InterLockedExchangeAdd(PLONG volatile plAddend,Long lIncrement)
				LONGLONG InterLockedExchangeAdd64(PLONGLONG volatile plAddend,LONGLONG lIncrement);
					//更新plAddend对象,增加lIncrement,lIncrement可为负数
				LONG InterLockedIncrement(PLONG  plAddend);
				LONG InterLockedDecrement(PLONG  plAddend);
					//自增/自减
			原子替换: 
				LONG InterLockedExchange(PLONG volatile plTarget,LONG lValue);
				LONGLONG InterLockedExchange64(PLONGLONG volatile plTarget,LONGLONG lValue);
					//使用lValue替换plTarget
				PVOID InterlockedExchangePointer(PVOID* volatile ppvTarget,PVOID pvValue);
					//使用pvValue替换ppvTarget
			原子交换: 
				PLONG InterlockedCompareExchange(PLONG plDestination,LONG lExchange,Long lCompare);
					//比较plDes和lCompare,若相同则使用lEx替换plDes
				LONG InterlockedCompareExchangePointer(PVOID* ppvDestination,PVOID pvExchange,PVOID pvComparand);
					//比较ppvDes和pvCom,若相同则使用pvEx替换
				LONGLONG InterlockedCompareExchange64(LONGLONG pllDestination,LONGLONG llExchange,LONGLONG llComprand);
	*/

	namespace atomic_access {

		long g_x{};

		DWORD WINAPI func1(PVOID pvParam)
		{
			InterlockedExchangeAdd(&g_x, 1);
			InterlockedCompareExchange(&g_x, 10, 3);
			return 0;
		}

		DWORD WINAPI func2(PVOID pvParam)
		{
			InterlockedExchangeAdd(&g_x, 2);
			InterlockedCompareExchange(&g_x, 10, 3);
			return 0;
		}

		void _test()
		{
			
			using namespace std;
			DWORD func1_id, exit_code_1;
			DWORD func2_id, exit_code_2;
			auto hd1=CreateThread(nullptr, 0, func1, nullptr, 0, &func1_id);
			auto hd2=CreateThread(nullptr, 0, func2, nullptr, 0, &func2_id);
			Sleep(200);
			GetExitCodeThread(hd1, &exit_code_1);
			GetExitCodeThread(hd2, &exit_code_2);
			cout << g_x << endl;
		}

		//other:一个简单的旋转锁
		//tips:旋转锁会大量的消耗cpu时间进行忙等,如果该线程的优先级较高,那么低优先级的线程就会失去被调度的机会.
		bool g_useable = false;
		void spin_lock()
		{
			//将g_useable设成ture,并查看旧值,若为false代表资源未被使用,若为true则表示资源正在被使用.
			while ((bool)InterlockedExchange((PLONG)&g_useable, true) == true)
			{
				Sleep(0);
			}

			//access the resource;
			//...
			InterlockedExchange((LONG*)&g_useable, false);
		}


		
	}

	/*
		临界区:CRITICAL_SECTION
			声明:
				CRiTICAL_SECTION g_cs;
			初始化:
				InitializeCriticalSection(&g_cs);
				InitializeCriticalSectionAndSpinCount(&g_cs,times); //初始化并关联一个旋转锁,在获取资源权限时,最多循环times次,仍不成功,则线程进入等待状态.
			进入临界区:
				EnterCriticalSection(&g_cs);
				TryEnterCriticalSection(&g_cs); //调用失败(返回false)不会是线程进入等待状态. 成功则表明已经获取资源访问权(true)
			离开临界区
				LeaveCriticalSection(&g_cs);
			删除:
				DeleteCriticalSection(&g_cs);
			其他:
				改变旋转锁的次数
					SetCriticalSectionSpinCount(&g_cs,500);
	*/


	namespace critical_section {


		int g_x{};
		critical_section_wrapper w{1};
		
		DWORD WINAPI func1(PVOID pvParam)
		{
			w.begin();
			for (int i = 0; i < 10; ++i)
			{
				++g_x;
			}
			w.end();
			return 0;
		}

		DWORD WINAPI func2(PVOID pvParam)
		{
			w.begin();
			for (int i = 0; i < 20; ++i)
			{
				++g_x;
			}
			w.end();
			return 0;
		}

		void _test()
		{
			using namespace std;
			DWORD id;
			DWORD id2;
			CreateThread(nullptr, 0, func1, nullptr, 0, &id);
			CreateThread(nullptr, 0, func2, nullptr, 0, &id2);
			cout << g_x << endl;	//g_X的值始终为30
		}
	}


	/*
		SRWLock读写锁
	*/

	/*
		声明:
			SRWLOCK lock;
		初始化:
			InitializeSRWlock(&lock);
		获取独占锁:
			AcquireSRWLockExclusive(&lock);
		解除独占锁:
			ReleaseSRWlockExclusive(&lock);
		获取共享锁:
			AcquireSWRLockShared(&lock);
		解除共享锁:
			ReleaseSWRLockShared(&lock);

		tips:
			1:当尝试获取锁的时候,若锁已经被占用,那么会阻塞调用线程
			2:不能递归的获取锁
	*/
	namespace srwlock {
		
		


		int g_x{};
		srwlock_wrapper sw{};
		DWORD WINAPI func1(PVOID pvParam)
		{
			sw.acquire_lock_exclusive();
			for (int i = 0; i < 10; ++i)
			{
				++g_x;
			}
			sw.release_lock_exclusive();
			return 0;
		}

		DWORD WINAPI func2(PVOID pvParam)
		{
			sw.acquire_lock_exclusive();
			for (int i = 0; i < 10; ++i)
			{
				g_x += 2;
			}
			sw.release_lock_exclusive();
			return 0;
		}

		DWORD WINAPI func3(PVOID pvParam)
		{
			sw.acquire_lock_shared();
			for (int i = 0; i < 10; ++i)
			{
				std::cout << g_x << std::endl;
				Sleep(2);
			}
			sw.release_lock_shared();
			return 0;
		}

		void _test()
		{
			DWORD func1_id, func2_id, func3_id;
			CreateThread(nullptr, 0, func1, nullptr, 0, &func1_id);
			CreateThread(nullptr, 0, func2, nullptr, 0, &func2_id);
			CreateThread(nullptr, 0, func3, nullptr, 0, &func3_id);
			Sleep(200);
		}
	}



	/*
		条件变量:condition_variable

			CRITICAL_SECTION cs;
			SRWLOCK lock;
			CONDITION_VARIABLE cv;
			InitializeCriticalSection(&cs);
			InitializeSRWLOCK(&lock);
		释放锁资源,阻塞线程,等待条件变量:
			SleepConditionVariableCS(&cv,&cs,200);
			SleepConditionVariableSRW(&cv,&lcok,200,0);
		唤醒:
			WakeConditionVariable(&cv);
			WakeAllConditionVariable(&cv);
	*/

	//一个简单的同步队列
	//由于是示例,所以不使用迭代器等设备,且只针对int类型的存储值



	class sync_queue {
	public:
		sync_queue(int capacity) :m_capacity{ capacity }, m_front{ 0 }, m_back{ 0 } 
		{
			element = new int[m_capacity]; 
			InitializeSRWLock(&m_lock); 
			InitializeConditionVariable(&cv_for_read);
			InitializeConditionVariable(&cv_for_write);
		
		};
	public:
		inline bool full() const {
			auto back_next = (m_back + 1) % m_capacity;
			if (back_next == m_front)
				return true;
			else
				return false;
		}
		inline bool empty() const { return m_back == m_front; }

		void enqueue(int val) {
			AcquireSRWLockExclusive(&m_lock);
			if (full())
			{
				SleepConditionVariableSRW(&cv_for_write, &m_lock, INFINITE, 0);
			}
			
				element[m_back] = val;
				m_back = (++m_back) % m_capacity;
			
			ReleaseSRWLockExclusive(&m_lock);
			WakeConditionVariable(&cv_for_read);
		};
		void dequeue(int& val)
		{
			AcquireSRWLockExclusive(&m_lock);
			if (empty()) {
				SleepConditionVariableSRW(&cv_for_read, &m_lock, INFINITE, 0);
			}
				val = element[m_front];
				m_front = (++m_front) % m_capacity;
			
			ReleaseSRWLockExclusive(&m_lock);
			WakeConditionVariable(&cv_for_write);
		}

		void front(int & val)
		{
			AcquireSRWLockShared(&m_lock);
			if (empty())
				return;
			else {
				val = element[m_front];
			}
			ReleaseSRWLockShared(&m_lock);
		}
	private:
		int * element;
		int m_front;
		int m_back;
		int m_capacity;
		SRWLOCK m_lock;
		CONDITION_VARIABLE cv_for_read;
		CONDITION_VARIABLE cv_for_write;
	};



	srwlock_wrapper sw{};

	DWORD WINAPI read(PVOID pvParam)
	{
		sync_queue* psq = (sync_queue*)pvParam;
		
		int val{-1};
		for (int i = 0; i < 100; ++i)
		{
			psq->dequeue(val);
			sw.acquire_lock_exclusive();
			std::cout << "read " << val << "from queue\n";
			sw.release_lock_exclusive();
			Sleep(500);
		}
		return 0;
	}

	DWORD WINAPI write(PVOID pvParam)
	{
		sync_queue* psq = (sync_queue*)pvParam;
		
		for (int i = 0; i < 100; ++i)
		{
			psq->enqueue(i);
			sw.acquire_lock_exclusive();
			std::cout << "write " << i << " into queue" << "\n";
			sw.release_lock_exclusive();
			Sleep(1000);

		}
		return 0;
	}

	sync_queue sq{ 32 };
	void _test()
	{
		
		DWORD thread_1, thread_2;
		CreateThread(nullptr, 0, read, &sq, 0, &thread_1);
		CreateThread(nullptr, 0, write, &sq, 0, &thread_2);
		

		Sleep(120 * 1000);
		
	}



}