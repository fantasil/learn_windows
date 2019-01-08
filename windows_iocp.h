#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#include<thread>
#define DEBUG

namespace windows_iocp {

	/*
		�ⲿ�ֵ���������windows_unit_10,���ǹ���iocp(io��ɶ˿�)�����ݷǳ���Ҫ,���е�������

			����ԭ��:
				HANDLE CreateIoCompletionPort(HADNLE hFile,HANDLE hExistingCompletionPort,ULONG_PTR CompletionKey,DWORD dwNumberOfConcurrentThreads);
					//���������΢����,��Ϊ��������������:����һ��io�˿ڶ���,�����豸�����.Ϊ��������������ʶ,���ǰ��������ֹ��ֿܷ�����
				#1:����io�˿ڶ���:
					HANDLE CreateNewIoCompletionPort(DWORD dwNumberOfConcurrentThreads)
					{
						return CreateIoCompletionPort(nullptr,nullptr,0,dwNumberOfConcurrentThreads);
					}
						//@dwNumberOfConcurrentThreads:��ɶ˿���ͬһʱ������������е��߳����� 0��ʾĬ��ֵ��������0
						//�ú���ֻ�Ǵ�����һ��IO�˿ڶ���,û�����豸���й���;
				#2:�����豸:
					BOOL AssociateDeivceWithCompletionPort(HANDLE hDevice,HANDLE hExistingCompletionPort,ULONG_PTR CompletionKey)
					{
						HANDLE h=CreateIoCompletionPort(hDevice,hExistingCompletionPort,CompletionKey,0);
						return (h==hExistingCompletionPort);
					}
						//@hDevice:���й������豸
						//@hExistingCompletionPort:����io�˿ڶ���
						//@CompletionKey:��ɼ�,�ᱻ��¼���豸�б�ṹ��,����׷��IO�����Ƿ��Ѿ����.
			��IO��ɶ˿���ص�5�����ݽṹ
				#1:�豸�б�
					����:��ʾ��IO��ɶ˿���������豸�б�
						#1	���:������CreateIoCompletionPort�����豸��ʱ��,������豸(hFile)������ɼ�(CompletionKey);
						#2	�Ƴ�:���豸������رյ�ʱ��,���Ƴ��豸(hFile);
				#2:IO��ɶ���(�Ƚ��ȳ�)
					����:��ʾIO��������״̬,�����������,�ȴ�����
						#1	���:IO�������;����PostQueuedCompletionStatus����
						#2	����:�ȴ��̶߳��б�����,�����ͷ��̶߳���(������IO������ɺ�ĺ�������);
				#3:�ȴ��̶߳���(�����Գ�)
					����:��ʾ���ڵȴ���io��������ɵĴ���
						#1	���:����GetQueuedCompletionStatus����
						#2	����:IO��ɶ��в�Ϊ��,�������е��߳���С������߳���;
				#4:���ͷ��߳��б�
					����:����io������ɵĺ�����
						#1	���:�ȴ��̳߳���;����ͣ�б��еĹ����̱߳�����
						#2	����:����GetQueuedCompletionStatus�������̼߳���ȴ�����;�̹߳����������ͣ�б�
				#5:����ͣ�б�
					����:�߳̽������״̬:
						#1	����:���ͷ��߳̽��Լ�����
						#2	����:�ѹ�����̱߳�����

			������Ҫ����:
				BOOL GetQueuedCompletionStatus(HADNLE hCompletionPort,PDWORD pdwNumberOfBytesTransferred
												,PULONG_PTR pCompletionKey,OVERLAPPED** ppOverlapped,DWORD dwMilliseconds);
					//_IN_ @hCompletionPort:��ɶ˿ھ��
					//_OUT_ @pdwNumberOfBytesTransferred:�����ֽ���
					//_OUT_ @pCompletionKey:�������ɼ�
					//_OUT_ @ppOverlapped:�ص��ṹ
					//_IN_ @dwMilliseconds:�ȴ�ʱ��
					//����:�������߳���io��ɶ˿������,������˯��,��ӵ��ȵ��̶߳���.Ȼ���������״̬�����߳�״̬�л�

				BOOL PostQueuedCompletionStatus(HANDLE hCompletionPort,DWORD dwNumberOfBytesTransferred,
												ULONG_PTR CompletionKey,LPOVERLAPPED pOverlapped);
					//����:��ָ������ɶ˿ڵ�IO��ɶ������һ�� 
			
			��������
				����create*���������豸��IO��ɶ˿�------>�����첽io����------>����ϵͳʹ���Լ��߳����IO����--->IO������ɺ����IO��ɶ���
											|																		|
											|-->��ʼ���̳߳�,����Get*Status��������˯��,�ȴ�IO�������	------>(���߳���Ŀ<���������Ŀʱ),�����߳�,�������											
																					
					
*/
	//����������key_kill��ɼ�ʱ,֪ͨ��ȡ���߳̽����Լ�

	const DWORD key_kill = 0;




	//һЩȫ�ֶ���
	const int buf_sz = 1024;
	char buff[buf_sz];
	const char* file_name = "first.txt";
	HANDLE hFile{ INVALID_HANDLE_VALUE };
	HANDLE iocp{ INVALID_HANDLE_VALUE };
	HANDLE hMutex{ INVALID_HANDLE_VALUE };

	//����ͬ��������,ʹ���ں˶���mutex;
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

	//�������Զ���������ڴ��Ͷ��������,�ڴ���������һЩ���õı����
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

	//io��ɴ�����
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
				my_overlapped* mo = (my_overlapped*)lpoverlapped;	//ת�����Զ���Ľṹ
				switch (mo->oper)
				{
				case io_oper::op_read:	//һ����ȡ���������
					async_print("#", local_id, "(io_resolve):\nbytes_read:", bytes_transfered, "\ncompletion_key:", cp_key, "\n", buff);
					break;
				case io_oper::op_write:	//һ��д����������
					async_print("#", local_id, "(io_resolve):write ", bytes_transfered, " bytes");
					break;
				case io_oper::op_kill:	//һ��"kill"��������Լ�
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
		//step1:��ʼ���豸
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
		//step2:����io��ɶ˿�,�����豸�����
		DWORD ck_read = 1;
		auto max_threads = std::thread::hardware_concurrency();
		iocp=CreateIoCompletionPort(hFile, nullptr, ck_read, max_threads);

		//step3:�������߳�,����IO������
		DWORD id_read_req, id_read_res;
		DWORD id_write_req, id_write_res;
		CreateThread(nullptr, 0, ioreq_read, nullptr, 0, &id_read_req);
		CreateThread(nullptr, 0, io_resolve, nullptr, 0, &id_write_res);
		//CreateThread(nullptr, 0, iores_read, nullptr, 0, &id_read_res);
		//����IOд���߳�
		CreateThread(nullptr, 0, io_resolve, nullptr, 0, &id_read_res);
		CreateThread(nullptr, 0, ioreq_write, nullptr, 0, &id_write_req);
		//CreateThread(nullptr, 0, io_resolve, nullptr, 0, &id_write_res);
		//step4:�ȴ�һ��ʱ��,������ָֹ��
		Sleep(5 * 1000);
		my_overlapped o_kill{ io_oper::op_kill };
		for (unsigned int i = 0; i < max_threads; ++i) {
			PostQueuedCompletionStatus(iocp, 0, key_kill, &o_kill);
		}
		//step5:������Դ
		CloseHandle(hFile);
		CloseHandle(iocp);
		CloseHandle(hMutex);
		//�߳̽�����ʾ
#ifdef DEBUG
		async_print("#", get_current_thread_id(), "(main_thread):exit!");
#endif // DEBUG

	}

}


/*
	
	learn:
		#1:io�����߳�,������GetQueuedCompletionStatus�������߳�,��������ȡ�õ�����ʱ��ȡ����д������,�����ʼд��iores_read/iores_write����������,�����ǲ���Ҫ��.
			ֻ��Ҫ�ڴ������ڲ�,���ݴ��͵�overlapped�ṹ,��ȡ�Զ�������ݽ����жϴ�������� (�����Զ������߳�(ioreq_read)֮����������д������(iores_write)֮��,�����������)
		#2:ͬ���������������Ҫ���ڲ��ݹ��ȡĳ������,��������ѡȡ�˻�����(mutex)�����ǹؼ���(critical_section)
		#3:�ʼ��ϣ���ڴ����߳���ͨ��completion_key�����в�ͬ�Ĵ���,,���Ƿ��ַ���������̲߳����ܴ���cp_key,ֻ��overlapped�ṹ,
			����ֻ��ʹ�ü̳�overlapped�ṹ���Զ����������ݿ��ƶ���
		
*/