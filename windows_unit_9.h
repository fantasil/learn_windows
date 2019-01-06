#pragma once
#include<Windows.h>
#include<iostream>

#define DEBUG

namespace windows_unit_9
{

	/*
		ʹ���ں˶�������߳�ͬ��
		�ں˶���ӵ��{����/�Ǵ���}����״̬.
	*/

	/*
		�ȴ�ϵ�к���:
			�߳���Դ����ȴ�״̬,ֱ��һ���ں˶����ڴ���״̬.

		DWORD WaitForSingleObject(HANDLE hObject,DWORD dwMillseconds);
			//ָ���ȴ�����hObject�͵ȴ�ʱ��dwMillseconds;
			//����ֵ{ --WAIT_OBJECT_0:�����ڴ���״̬;
					  --WAIT_TIMEOUT:�ȴ���ʱ;
					  --WAIT_FAILED:�����Ч}
		DWORD WaitForMultipleObkect(DWORD dwCount,CONST HANDLE* phObject,BOOL bWaitAll,DWORD dwMillseconds);
			//ָ���ȴ��������(dwCountָ��������Ŀ,phObject��һ��handle����,bWaitAllָ���Ƿ�ȴ�ȫ�����󴥷�,dwMillsecondsָ���ȴ�ʱ��)

		tips:
		�ȴ������ĸ�����:
			����ĳЩ�ں˶�����˵,�ȴ������ɹ�����֮���ı�����״̬(����,�Ǵ���).���Զ������¼�
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

		//func2������func1ִ�����
		DWORD WINAPI func2(PVOID pvParam)
		{
			HANDLE handle = (HANDLE)(pvParam);
			WaitForSingleObject(handle, INFINITE);
			g_z = g_y * 5;
			std::cout << "g_z=" << g_z << "\n";
			return 0;
		}

		//func3��func1������,���Բ���
		DWORD WINAPI func3(PVOID pvParam)
		{
			g_m = 200;
			std::cout << "g_m=" << g_m << "\n";
			return 0;
		}

		//func4������fun2��func3ִ�����
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
		�¼��ں˶���:
			����:�¼����ʱ����,���ѵȴ��߳�.{�Զ������¼�:������ʱ,ֻ��һ���̱߳�ɿɵ���״̬}
										   �ֶ������¼�:������ʱ,���еȴ��̱߳�ɿɵ���״̬
			��������:��һ���߳�ִ�г�ʼ������,Ȼ���ٴ�������һ���߳�.
			����:
				HANDLE CreateEvent(PSECURITY_ATTRIBUTES psa,BOOL bManualReset,BOOL bInitialState,PCTSTR pszName);
				HANDLE CreateEventEx(PSECURITY_ATTRIBUTES psa,PCTSTR pszName,DWORD dwFlags,DWORD dwDesiredAccess);
					//dwFlag{CREATE_EVENT_INITIAL_SET:��ʼ״̬Ϊ����,ͬbInitialState��������} 
					        {CREATE_EVENT_MANUAL_RESET:��Ϊ�ֶ������¼�,ͬbManualReset��������}\
					//dwDesiredAccess{ָ�����¼��ķ���Ȩ��}

			����:
				HANDLE OpenEvent(DWORD dwDesiredAccess,BOOL bInherit,PCSTR pszName);
			����״̬:
				BOOL SetEvent(HANDLE hEvent);	//���óɴ���״̬
				BOOL ResetEvent(HANDLE hEvent);	//���ó�δ����״̬
			������:
				�Զ������¼�����֮��,���̵߳ȴ��ɹ�֮�󱻵���֮ǰ,���¼��ᱻ����ϵͳ�Զ�����Ϊδ����״̬.

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
		�ɵȴ��ļ�ʱ���ں˶���:
			����:��ĳ��ָ�����¼�����,����ÿ��һ��ʱ��δ���
			����:
				HANDLE CreateWaitableTimer(PSECURITY_ATTRIBUTES psa,BOOL bManualReset,PCTSTR pszName);
			����:
				HANDLE OpenWaitableTimer(DWORD dwDesiredAccess,BOOL bInheritHandle,PCSTR pszName);
			����:
				BOOL SetWaitableTimer(HADNLE hTimer,const LARGE_INTEGER* pDueTime,
									  long lPeriod,PTIMERAPCROUTINE pfnCompletionRoutine,
									  PVOID pvArgToCompletionRoutine,BOOL bResumed);
				//@pDueTime:��һ�δ�����ʱ��,����UTCʱ��
				//@lPeriod:�������
				//@pfnCompletionRoutine:�첽���̵��ú���
				//@pvArgToCompletionRoutine:�첽���̺�������
				
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
		

		//�˺�������һ����ʱ������
		/*
			#1:SYSTEMTIME st;FILETIME ftLocal,ftUTC;	//st�Ǳ���ϵͳʱ��,ftLocal�Ǳ����ļ���ʽʱ��,ftUTC���ļ���ʽ��UTCʱ��
			#2:����stָ����Ӧʱ�䲢ת����ftLocal			//SystemTimeToFileTime(&st,&ftLocal);
			#3:��flLocalת���ɱ�׼UTCʱ��					//LocalFileTimeToFileTime(&ftLocal,&ftUTC);
			#4:��ftUTCת����LARGE_INTEGER��ʽ��ʾ			//liUTC.lowPart=ftUTC.dwLowDateTime;liUTC.highPart=ftUTCDateTime;
			#5:����ֱ�Ӹ�ֵ liUTC.QuadPart=xxxx;xxxx��һ������100ns����������ֵ,���xxxxΪ��,��ʾ����ʱ��;����Ϊ���ʱ��;
			   ��liUTC.QuadPart=-10*1000*1000*10;��ʾִ��10s֮���ʱ��.

			tips:
			   ��ȻLARGE_INTEGER��FILETIME�Ķ����Ƹ�ʽ��ͬ,���Ƕ��뷽ʽһ��(FILETIME32λ����,LARGE_INTEGER64λ����),
			   ���Բ���lowPart=lowDateTime��ʽ,�����ǽ���ǿ��ת��(liUTC=(LARGE_INTEGER)ftUTC;);

			#6 APC:�첽���̵���
				1:�첽���̵��ý��������ڵ���SetWaitableTimer�������̶߳�����,�ڴ���ʱ�и��̵߳���
				2:���̱߳����ǿ�����״̬,�������̵߳���SleepEx,WaitForSingleObjectEx,
													WaitForMultipleObjectsEx,MsgWaitForMultipleObjectsEx,
													SignalObjectAndWait������ĵȴ�״̬
				3:����ͬһ�߳��е���SetWaitableTimer����,��APC�������,���ȴ��ü�ʱ������ʱ.
					����ʱ�����󱻵���,�߳̾��˳��˿�����״̬,APC����������ִ��.
					//��Ҫ��ͬһ�߳��еȴ���ͬ���ں˶���
			
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
		�ź����ں˶���
			����:
				HANDLE CreateSemaphore(PSECURITY_ATTRIBUTES psa,LONG lInitialCount,Long lMaxinumCount,PCTSTR pszName);
					//@lInitialCount:��ǰ��Դ����
					//@lMaxinumCount:�����Դ����
				HANDLE CreateSemaphoreEx(PSECURITY_ATTRIBUTES pas,LONG lInitialCount,LONG lMaxinumCount,PCTSTR pszName,DWORD dwflags,DWORD dwDesireAccess);
					//@dwFlags:ϵͳ���� ʼ��Ϊ0
					//@dwDesireAccess:ָ������Ȩ��
			������ǰ��Դ����:
				BOOL ReleaseSemaphore(HADNLE hSemaphore,LONG lReleaseCount,PLONG plPreviousCount);
					//@lReleaseCount:����������
					//@plPreviousCount:����ǰ�ļ���
			tips:
				Ŀǰû�а취�ڲ��ı���Դ����ֵ������»�ȡ��Դ����ֵ

			��������:
				#1:������ָ�������Դ�����͵�ǰ������Դ����
				#2:���������Դ��������0,���ڴ���״̬,�ȴ��߳̿��Ի�õ���;
				#3:�߳��뿪�ȴ�״̬,������Դ����-1,�߳̿�ʼִ��.
				#4:ֱ��������ԴΪ0,�źŴ��ڷǴ���״̬,�߳̽���ȴ�
				#5:�߳̿���ͨ��ReleaseSemaphore�������ӿ�����Դ����.
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
		������:
			����:
				HANDLE CreateMutex(PSECURITY_ATTRIBUTES psa,BOOL bInitialOwner,PCTSTR pszName);
					//@bInitialOwner:���Ƴ�ʼ״̬,false��ʾ��������Ϊ�κ��߳���ռ��
				HANDLE CreateMutexEx(PSECURITY_ATTRIBUTES psa,PCTSTR pszName,DWORD dwFlags,DWORD dwDesiredAccess);
					//@dwFlags:�����bInitialOwner,0��ʾfalse,CREATE_MUTEX_INTIAL_OWNERΪtrue;
			����:
				HANDLE OpenMutex(DWORD dwDesiredAccess,BOOL bInheritHandle,PCTSTR pszName);
			�߳��ͷ�:(����������)
				BOOL ReleaseMutex(HANDLE hMutex);

			��������:
				�������������һ��ʹ�ü���,�߳�ID,�Լ��ݹ����.
				�߳�ID��ʾ��ǰ����ռ�û��������߳�,0��ʾû�б�ռ��,��ʱ���������ڴ���״̬;��0��ʾ��ĳһ���߳���ռ��.
				�߳�ռ�û�����ʱ,�Ὣ�ݹ������Ϊ1,��ͬһ���߳���ͼ�ȴ���ͬ�Ļ�����ʱ,�õݹ����������;
				���������ͷ�ֻ���õݹ������1,���Ի�ȡ����������εȴ����߳�,��Ҫ����ͷŻ�����.ֱ������Ϊ0,���������´��ڴ���״̬.
				��һ��ռ�û��������߳���δ�ͷŻ�����֮ǰ������(ExitThread��),��ô�û������ͱ�������,ֱ��ϵͳ�Զ��������û�������״̬.

			��������:
				�����ڹؼ��ε�,���м�����ͬ.p256��9-2;


	*/


	namespace mutex_unit {

		//һ������mutex&semaphore�ļ�ͬ������
		//tips:����Ŀɶ���д����Ҫ�ı���е�״̬.
		/*
			learn:
				#1:����Ҫͬʱ��ȡ����ں˶����ʱ��,ʹ��WaitForMultipleObjects�����Ƿֱ�ʹ��WaitForSingleObject,���߻��������
					//��ʼʱ,���ɶ��߳��Ȼ�ȡ��mutex,�����ܻ�ȡ�ɶ��ź�(��ʼ���ɶ�,����д),����ȴ�;��д�̻߳�ȡ����mutex�����ȴ�.
				#2:�ں˶����ͷŵ�˳���������̲߳���Ӱ��.
					//���ڿ�ʼ���ʱ�ɶ��ź�����ͷ�,����ʱ��д�ź�����ͷ�,��������һֱִ�����,Ȼ���ִ�г��Ӳ���.
				#3:������û�ͬ��������ͬ�ĵط�
					//�û�ģʽ��������п�,��ɶ��ȴ�;�����ǻ�ȡ�ɶ��ź�(�ɽ���ȴ�),������ɺ���ִ�ж��źŵĵ���.
				#4:���ǹ��Ƿ�ʹ��bool��ʾ�ɶ���д.
					//���ǲ�,��Ϊ������Ҫ�������������ʱ��ʹ�߳̽���ȴ�״̬,bool����������,��Ҫʹ���ں˶���
		*/
		class sync_queue {
		public:
			sync_queue(int capacity) :m_capacity{ capacity }, m_front{ 0 }, m_back{ 0 }
			{
				m_element = new int[m_capacity];
				m_mtx = CreateMutex(nullptr, false, nullptr);
				//�ɶ��ź�,������Ϊ1,��ǰ����Ϊ0,δ����,û�����ݿɶ�;Ϊ1��ʾ�����ݿɶ�;��ʼ���ɶ�
				m_semaphore_for_read = CreateSemaphore(nullptr, 0, 1, nullptr);
				//��д�ź�,������Ϊ1,��ǰ����Ϊ1,�ɴ���,��ǰ��д;Ϊ0��ʾû���ڴ�λ�ÿ�д����;��ʼ��д
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
				//���ʱ,��Ҫͬʱ��ȡ�������Ϳ�д�ź�
				const HANDLE handle_arr[2] = { m_mtx,m_semaphore_for_write };
				WaitForMultipleObjects(2, handle_arr, true, INFINITE);
				//WaitForSingleObject(m_mtx, INFINITE);
				//WaitForSingleObject(m_semaphore_for_write, INFINITE);
#ifdef DEBUG
				std::cout << "in:" << val << "\n";
#endif // DEBUG

				m_element[m_back] = val;
				m_back = (++m_back) % m_capacity;
				ReleaseSemaphore(m_semaphore_for_read, 1, nullptr);	//���ݲ���֮��,�����ɶ��ź�
				if (!full())
				{
					ReleaseSemaphore(m_semaphore_for_write, 1,nullptr);	//�������δ��,���´�����д�ź�
				}
				ReleaseMutex(m_mtx);
				
			}

			void dequeue(int& front)
			{
				//����ʱ,��Ҫͬʱ��ȡ�������Ϳɶ��ź�
				const HANDLE handle_arr[2] = { m_mtx,m_semaphore_for_read };
				WaitForMultipleObjects(2, handle_arr, true, INFINITE);
				///WaitForSingleObject(m_mtx, INFINITE);
				//WaitForSingleObject(m_semaphore_for_read, INFINITE);
				//��ȡ��read�źű�ʾ�ɶ�
				front = m_element[m_front];
				m_front = (++m_front) % m_capacity;
#ifdef DEBUG
				std::cout << "out:" << front << "\n";
#endif // DEBUG
				ReleaseSemaphore(m_semaphore_for_write, 1, nullptr);
				if (!empty())
				{
					ReleaseSemaphore(m_semaphore_for_read, 1, nullptr);	//������в�Ϊ��,���´����ɶ��ź�
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
			//������������ֻ�����CRITICAL_SECTION��SRWLock,�������ﲻ��ʹ��
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
		�����߳�ͬ������
			#1:�첽�豸IO:�߳̿���ִ�ж�ȡ/д�����,���ǲ��صȴ��������;���ڼ����ִ����������.  ?����ϵͳ���������߳�ִ�����������?
			#2:BOOL WaitForInputIdle(HANDLE hProcess,DWORD dwMilliseconds):�ȴ�hProcess����,ֱ���ý��̵����߳�û�д����������Ϊֹ
			#3:MsgWaitForMultipleObjects(Ex)����:������WaitForMultipleObjects,����ĳ��������Ϣ�����͵��ɸ��̴߳����Ĵ���ʱ,Ҳ���Ϊ�ɵ���״̬.
			#4:WaitForDebugEvent����:�ȴ������¼�
			#5:SignalObjectAndWait����:����һ���ں˶��󲢵ȴ�����һ���ں˶���;�������ı�����{������,�ź���,�¼�),�ɵȴ�������ͨ�ں˶���;

			DWORD SignalObjectAndWait(HANDLE hObjectToSignal,HANDLE hObjectToWaitOn,DWORD dwMilliseconds,BOOL bAlertable);

			��������Ҫ����һ������,���ҵȴ�����һ�������ʱ��,�п��ܻ�������µĴ���:
				SetEvent(hEventA);
				WaitForSingleObject(hEventB,INIFNITE);
			��������:
				��A������ʱ��,�߳̿���ʧȥ��ʱ��Ƭ,����δ��ִ�еȴ�B�¼��Ĵ���.��ʱ,��������һ���̴߳���B�¼���ʱ��,���̲߳����ܵõ�����.��set/wait����������,������ԭ�Ӳ���;
			����취:
				SignalObjectAndWait(hEventA,hEventB,INFINITE,false);
	*/



	

}