#pragma once
#include<Windows.h>
#include<iostream>
namespace windows_unit_8 {



	//�����ʼ���������ǻ��׳�"����û�д洢���˵����"����,��������ʹ���˰�װ��

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

	//��������init��������һ��"����û�д洢���˵�����Ĵ���",��������ʹ���˰�װ��.
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
		windows_unit_8:�û�ģʽ�µ��߳�ͬ��
	*/


	/*
		ԭ�ӷ���:InterLockedϵ�к���
			ԭ������:
				LONG InterLockedExchangeAdd(PLONG volatile plAddend,Long lIncrement)
				LONGLONG InterLockedExchangeAdd64(PLONGLONG volatile plAddend,LONGLONG lIncrement);
					//����plAddend����,����lIncrement,lIncrement��Ϊ����
				LONG InterLockedIncrement(PLONG  plAddend);
				LONG InterLockedDecrement(PLONG  plAddend);
					//����/�Լ�
			ԭ���滻: 
				LONG InterLockedExchange(PLONG volatile plTarget,LONG lValue);
				LONGLONG InterLockedExchange64(PLONGLONG volatile plTarget,LONGLONG lValue);
					//ʹ��lValue�滻plTarget
				PVOID InterlockedExchangePointer(PVOID* volatile ppvTarget,PVOID pvValue);
					//ʹ��pvValue�滻ppvTarget
			ԭ�ӽ���: 
				PLONG InterlockedCompareExchange(PLONG plDestination,LONG lExchange,Long lCompare);
					//�Ƚ�plDes��lCompare,����ͬ��ʹ��lEx�滻plDes
				LONG InterlockedCompareExchangePointer(PVOID* ppvDestination,PVOID pvExchange,PVOID pvComparand);
					//�Ƚ�ppvDes��pvCom,����ͬ��ʹ��pvEx�滻
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

		//other:һ���򵥵���ת��
		//tips:��ת�������������cpuʱ�����æ��,������̵߳����ȼ��ϸ�,��ô�����ȼ����߳̾ͻ�ʧȥ�����ȵĻ���.
		bool g_useable = false;
		void spin_lock()
		{
			//��g_useable���ture,���鿴��ֵ,��Ϊfalse������Դδ��ʹ��,��Ϊtrue���ʾ��Դ���ڱ�ʹ��.
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
		�ٽ���:CRITICAL_SECTION
			����:
				CRiTICAL_SECTION g_cs;
			��ʼ��:
				InitializeCriticalSection(&g_cs);
				InitializeCriticalSectionAndSpinCount(&g_cs,times); //��ʼ��������һ����ת��,�ڻ�ȡ��ԴȨ��ʱ,���ѭ��times��,�Բ��ɹ�,���߳̽���ȴ�״̬.
			�����ٽ���:
				EnterCriticalSection(&g_cs);
				TryEnterCriticalSection(&g_cs); //����ʧ��(����false)�������߳̽���ȴ�״̬. �ɹ�������Ѿ���ȡ��Դ����Ȩ(true)
			�뿪�ٽ���
				LeaveCriticalSection(&g_cs);
			ɾ��:
				DeleteCriticalSection(&g_cs);
			����:
				�ı���ת���Ĵ���
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
			cout << g_x << endl;	//g_X��ֵʼ��Ϊ30
		}
	}


	/*
		SRWLock��д��
	*/

	/*
		����:
			SRWLOCK lock;
		��ʼ��:
			InitializeSRWlock(&lock);
		��ȡ��ռ��:
			AcquireSRWLockExclusive(&lock);
		�����ռ��:
			ReleaseSRWlockExclusive(&lock);
		��ȡ������:
			AcquireSWRLockShared(&lock);
		���������:
			ReleaseSWRLockShared(&lock);

		tips:
			1:�����Ի�ȡ����ʱ��,�����Ѿ���ռ��,��ô�����������߳�
			2:���ܵݹ�Ļ�ȡ��
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
		��������:condition_variable

			CRITICAL_SECTION cs;
			SRWLOCK lock;
			CONDITION_VARIABLE cv;
			InitializeCriticalSection(&cs);
			InitializeSRWLOCK(&lock);
		�ͷ�����Դ,�����߳�,�ȴ���������:
			SleepConditionVariableCS(&cv,&cs,200);
			SleepConditionVariableSRW(&cv,&lcok,200,0);
		����:
			WakeConditionVariable(&cv);
			WakeAllConditionVariable(&cv);
	*/

	//һ���򵥵�ͬ������
	//������ʾ��,���Բ�ʹ�õ��������豸,��ֻ���int���͵Ĵ洢ֵ



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