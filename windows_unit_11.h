#pragma once
#include<iostream>
#include<Windows.h>
#include<string>
#include"general.h"

namespace windows_unit_11
{
	/*
		������Ҫ����windows�̳߳�:
			#1:ʹ���첽��ʽ���ú���
			#2:ÿ��һ��ʱ����ú���
			#3:�ں˶��󴥷���ʱ����ú���
			#4:�첽IO�������ʱ���ú���
			#5:�ص���������ֹ����
	*/


	/*
		#1:ʹ���첽��ʽ���ú���
			##1:����һ���ص�����:
				VOID NTAPI SimpleCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext);
					//@pInstance:�����ں���ִ����Ϸ���֮��,Ӧ��ִ�еĲ���.ͨ���Ƕ�ͬ������(�ٽ���,������,�ź���,�¼�)�Ĵ���,Ҳ���Ա�ʾж��dll
					//@pvContext:�����Ĳ���,������pvParam,���ڴ�����Ҫ����Ϣ
			##2:���̳߳��ύ����
				BOOL TrySumbitThreadpoolCallback(PTP_SIMPLE_CALLBACK pfnCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
					//@pfnCallback:�Զ���Ļص�����ָ��
					//@pvContext:���ݸ�������pvContextֵ
					//@pcbe:
			��ʹ��trysubmit�ύ�����ʱ��,ϵͳ�����ڲ������Ƿ���һ��������,Ȼ���ύ,��,����Ҳ������ʾ�Ŀ��ƹ�����.
		��ʾ�Ŀ��ƹ�����:
			��ĳЩ�����(�ڴ治���),TrySumbitThreadPoolCallback�������ÿ��ܻ�ʧ��,�����ǲ��ܽ���ʧ�ܵ����ʱ,���ǿ�����ʾ�Ŀ��ƹ�����;
			##1:����ص�����:
				VOID CALLBACK WorkCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_WORK Work);
					//@pInstance:ͬ��,��������ִ����Ϸ��غ�Ĳ���.
					//@pvContext:ͬ��
					//@work:
			##2:����������:
				PTP_WORK CreateThreadpoolWork(PTP_WORK_CALLBACK pfnWorkHandler,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
					//@pfnWorkHandler:�ص�����ָ��
					//@pvContext:���ݸ��ص�����pvContextֵ
					//pcbe:
			##3:�ύ����:
				VOID SumbitThreadpoolWork(PTP_WORK pWork);
					//�������ﷵ��ֵ��void,����Ҫ�б��Ƿ�ɹ��������.���ǿ���Ĭ��Ϊ�ɹ���
			##4:ȡ���ύ�Ĺ�����:
				VOID WaitForThreadpoolWorkCallbacks(PTP_WORK pwork,BOOL bCancelPendingCallbacks);
					//@pwork:ָ������
					//@bCancelPendingCallbacks:���Ϊtrue,�����������ڵȴ�,��ᱻȡ��(�������������ȡ�����);������ִ��,����(��ǰ������)ִ�����֮�󷵻�.
											   ��Ϊfalse,�ú����ĵ����̻߳ᱻ����,ֱ���������Ѿ�ִ�����(�ȵ������ύ�Ĺ�����ִ�����)
			##5:ɾ��������:
				VOID CloseThreadpoolWork(PTP_WORK pwork);
					//@pwork:ָ��ɾ����work����

			tips:
				1:��ʾ����Ļص���������������Ļص���������һ��PTP_WORK����,�������������ͬ
				2:������ύ�������ʱ��(work),��ô�ص����������õ�ʱ��,�����pvContextֵ����ͬ��,���ڴ���work������ָ����ֵ
				3:���һ��work�����ύ�˶��������,�ҵ���WaitForThreadpoolWorkCallbacksʱ,���ݲ���Ϊfalse,��ô��ȴ����еĹ�����ִ�����;����ture,ֻ�ȴ���ǰ������ִ�����
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
		#2:ÿ��һ��ʱ�����һ������
			##1:����һ���ص�����:
				VOID CALLBACK TimeoutCallback(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_TIMER pTimer);
			##2:����PTP_TIMER����
				PTP_TIMER CreateThreadpoolTimer(PTP_TIMER_CALLBACK pfnTimerCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
			##3:���̳߳�ע��
				VOID SetThreadpoolTimer(PTP_TIMER pTimer,PFILETIME pftDueTime,DWORD msPeriod,DWORD msWindowLength);
					//@pTimer:
					//@pftDueTimer:��һ�δ�����ʱ��
					//@msPeriod:��һ�δ���֮���ٴδ������¼����
					//@msWindowLength:����ӳ�ʱ��,���ڽ��������ʱ���ӳ���[msPeriod,msPeriod+msWindowLength]ʱ�����

			tips:��δ���������������Ĺ�����ʮ������.
			learn:
				#1:�ڻص������ڼ��ʱ�䲻��ִ�е�ʱ��,���̿ռ���뱣��.����ĳһʱ��,������һ������߳�,ʹ�ý�����Ч.�������߳�ͨ��Sleep�������Լ�

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
		#3:���ں˶��󴥷���ʱ����ú���
			##1:�ص�����ԭ��:
				VOID  CALLBACK WaitCallBack(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,PTP_WAIT wait,TP_WAIT_RESULT WaitResult);
					//@WaitResult:���ڱ�ʶcallback���õ�ԭ�� {WAIT_OBJECT_0,WAIT_TIME_OUT,WAIT_ABANDONED_0},������DWORD����
			##2:�����̳߳صȴ�����:
				PTP_WAIT CreateThreadpoolWait(PTP_WAIT_CALLBACK pfnWaitCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
			##3:ע��
				VOID SetThreadpoolWait(PTP_WAIT pWaitItem,HANDLE hObject,PFILETIME pftTimeOut);
					//@pWaitItem:##2�д����ĵȴ�����
					//@hObject:��ʶ�ȴ����ں˶���
					//@pftTimeOut:��ʱʱ��,nullptr��ʾ���޳�
	*/

	namespace use_thread_pool_by_wait_item {
		using namespace std;
		using namespace general::general_class;
		using namespace general::general_func;

		using pmft = msg_for_thread * ;

		HANDLE hEvent = CreateEvent(nullptr, false, false, nullptr);
		HANDLE hMutex = CreateMutex(nullptr, false, nullptr);

		//���жϵȴ��¼��Ƿ񴥷�,������������̵߳��ʺ�����
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
			Sleep(1000);	//ȷ�����߳���ȷ�˳�����������
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
		#4:���첽IO�������ʱ����һ������
			##1:�ص�����ԭ��
				VOID CALLBACK OverlappedCompletionRoutine(PTP_CALLBACK_INSTANCE pInstance,PVOID pvContext,
														  PVOID pOverlapped,ULONG ioResult,ULONG_PTR numberOfByteTransfered,PTP_IO pio)
					//@ioResult:io��� NO_ERROR��ʾ�ɹ�
					//@pid:�̳߳�IO����
			##2:�����̳߳�IO����:
				PTP_IO CreateThreadpoolIo(HANDLE hDevice,PTP_WIN32_IO_CALLBACK pfnIoCallback,PVOID pvContext,PTP_CALLBACK_ENVIRON pcbe);
			##3:ע��:
				VOID StartThreadpoolIo(PTP_IO pio);
			##4:ȡ������:
				VOID CancelThreadpoolIo(PTP_IO pio);
			##5:�ͷ�:
				VOID CloseThreadpoolIo(PTP_IO pio);

			tips:
				#1:�ڶ�ȡ��д�����֮ǰ(����˵ReadFile/WriteFile),�����ȵ���StartThreadpoolIo����,����ص�����ִ��
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
		#5:�����߳���
			ĿǰΪֹ����ʹ�õĶ���Ĭ�ϵ��̳߳�,���������������һ��,����ϵͳ���Զ��Ľ�����Դ�������.������Ҳ����ʹ���Զ�����̳߳�
				#1:�����µ��̳߳�:
					PTP_POOL CreateThreadpool(PVOID reserved);
				#2:ָ���̳߳ص������С�߳�����(Ĭ����СΪ1,���Ϊ500)
					BOOL SetThreadpoolThreadMininum(PTP_POOL pThreadPool,DWORD cthrdMin);
					BOOL SetThreadpoolThreadMaxinum(PTP_POOL pThreadPool,DWORD cthrdMax);
				#3:�����̳߳�:
					VOID CloseThreadpool(PTP_POOL pThreadPool);
				#4:���ƻص�����,���ϱߵ�_TP_CALLBACK_ENVIRON����
					##1:��ʼ��:
						VOID InitializeThreadpoolEnvironment(PTP_CALLBACK_ENVIRON pcbe);
					##2:�����̳߳ػص�����
						VOID SetThreadpoolCallbackPool(PTP_CALLBACK_ENVIRON pcbe,PTP_POOL pThreadPool);
					##3:��Ԥ���߳���Ҫ��ʱ��Ĵ���ʱ��,�����ûص�����ʹ�ø���ƽ�ĵ���
						VOID SetThreadpoolCallbackRunsLong(PTP_CALLBACK_ENVIRON pcbe);
					##4:���̳߳����д�������,�����ض���dll�ڽ��̵ĵ�ַ�ռ���
						VOID SetThreadCallbackLibrary(PTP_CALLBACK_ENVIRON pcbe,PVOID mod);
					##5:����:
						VOID DestroyThreadpoolEnvironment(PTP_CALLBACK_ENVIRON pcbe);
				#5:ʹ���������������̳߳�:
					�˽�ֻ�������Զ�����̳߳�.
					##1:����������:
						PTP_CLEANUP_GROUP CreateThreadpoolCleanupGroup();
					##2:�������鱻ȡ��ʱ���õĻص�����ԭ��:	
						VOID CALLBACK CleanupGroupCancelCallback(PVOID pvObjectContext,PVOID pvCleanupContext);
					##3:����pcbe�ص������ṹ:
						VOID SetThreadpoolCallbackCleanupGroup(PTP_CALLBACK_ENVIRON pcbe,PTP_CLEANUP_GROUP ptpcg,PTP_CLEANUP_GROUP_CANCEL_CALLBACK pfng);
							//@ptpcg:���������,�������ûص������е�CleanupGroup�ֶ�
							//@pfng:##2�еĻص�����,��ȡ���������ʱ�򱻵���,�������ûص������е�CleanupGroupCancelCallback�ֶ�
					##4:�����̳߳�:
						VOID CloseThreadpoolCleanupGroupMembers(PTP_CLEANUP_GROUP ptpcg,BOOL bCancelPendingCallbacks,PVOID pvCleanupContext);
							//����ʮ��������WaitForThreadpoolWork()����
					##5:����������:
						VOID WINAPI CloseThreadpoolCleanupGroup(PTP_CLEANUP_GROUP ptpcg);

	*/


	/*
		example:
			���߳����������30s����ʱ;
			ʹ��ptp_timer������õ���ʱ����(time_count_down)ÿ���ӡ����ʱ���;
			����ʱ���,���߳��ͷ�ptp_timer����,��ʹ��ptp_io����io����,��ɺ����(io_down)���io�¼�(�����Ƕ�ȡ�¼�,�����ȡ����);
			io_down��Ϻ�ᴥ���¼�,����close_global_val����������,����ȫ�ֱ���������֮���˳�

		learn:
			#1:��ͬ��ptp_*����,�ÿ����ڻص������еõ��ͷ�(is_this_right?);
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

		//���ڴ�ӡ����ʱ
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

		//�ֶ��ر�ȫ����Դ
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

		//���첽io���ʱ���ú���
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