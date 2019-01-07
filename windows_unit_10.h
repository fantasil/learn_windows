#pragma once
#include<Windows.h>
#include<iostream>
#include<string>
#define DEBUG

namespace windows_unit_10 {

	/*
		ͬ���豸IO���첽�豸IO
			ͬ��:�̷߳���IO�����,�̱߳�����,ֱ��IO������ɻ��߷�������,��������;
			�첽:�̷߳���IO�����,����ֱ�ӷ���,��IO������ɻ��������,�̵߳õ�֪ͨ;
	*/

	/*
		�򿪺͹ر��豸:

		msdn_guide:https://docs.microsoft.com/zh-cn/windows/desktop/FileIO/creating-and-opening-files

			�豸:�κ��ܹ���֮ͨ�ŵĶ���
			Createϵ�к���:
				CreateFile(...):�ɴ��ļ�,Ŀ¼,�߼�/�������������,����,����,�ʼ��ۿͻ���;
				CreateMailslot(...):���ʼ��۷����
				CreateNamedPipe(...):�������ܵ�
				CreatePipe(...):�������ܵ�
				Socket():�׽���
				CreateConsoleScreenBuffer()/GetStdHandle():����̨
			��ȡ�豸����:
				DWORD GetFileType(HANDLE hDevice); 
					//FILE_TYPE_UNKNOW
					//FILE_TYPE_DISK
					//FILE_TYPE_CHAR
					//FILE_TYPE_PIPE
			CreateFile����:
				HANDLE CreateFile(PCTSTR,DWORD,DWORD,PSECURITY_ATTRIBUTES,DWORD,DWORD,HANDLE);
					//@pszName:�ȿ��Ա�ʾ�豸����,Ҳ���Ա�ʾ�豸��ĳ��ʵ��
					//@dwDesiredAccess:����Ȩ��
						$0:�Ƕ���д,ֻ�Ǹı��豸������
						$GENERIC_READ:ֻ��
						$GENERIC_WRITE:ֻд
						$GENERIC_READ|GENERIC_WRITE:��д
					//@dwShareMode:ָ���豸������Ȩ
						$0:��ռ����,���ѱ���ռ,��CreateFile��ʧ��
						$FILE_SHARE_READ:�����Ȩ��,�����̷߳��ʿɶ�,������д
						$FILE_SHARE_WRITE:����дȨ��,�����̷߳��ʿ�д,�����ɶ�
						$FILE_SHARE_READ|FILE_SHARE_WRITE:�����дȨ��
						$FILE_SHARE_DELETE:�������ļ��Ƿ��߼�ɾ�����ƶ�	 "?"
					//@psa:ָ����ȫ��Ϣ�ͼ̳�Ȩ
					//@dwCreationDisposition:�򿪷�ʽ
						$CREATE_NEW:�������ļ�,��ͬ���ļ��Ѵ���,��CreateFile����ʧ��
						$CREATE_ALWAYS:�������ļ�,�����ͬ���ļ�,����֮
						$OPEN_EXISTING:��һ�������ļ����Ɑ,���������,CreateFile����ʧ��
						$OPEN_ALWAYS:�������ļ�,��������,�򴴽��µ��ļ�
						$TRUNCATE_EXISTING:�������ļ����ض�Ϊ0�ֽ�,����ļ�������,�����ʧ��
					//dwFlagAndAttributes:΢�����豸֮���ͨ��;�����һ���ļ�,���ܹ������ļ�������
						$FILE_FLAG_*:������Ӧ�ı�־,�������ò�ͬ�Ļ��淽ʽ,����ģʽ����������
						$FILE_ATTRIBUTE_*:������Ӧ������
					//dwFileTemplate:һ���Ѵ򿪵���Ч���,���ļ���̳и����Բ�����dwFlagAndAttributes����
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
		ʹ���ļ��豸:
			��ȡ�ļ���С:
				BOOL GetFileSizeEx(HANDLE hFile,PLARGE_INTEGER pliFileSize);
					//@hFile:�ļ����
					//@pliFileSize:�ļ���С�ı���λ��,64λ
				DWORD GetCompressedFileSize(PCTSTR pszFileName,PDWORD pdwFileSizeHigh);
					//pszFileName:�ļ�����
					//pdwFileSizeHigh:�ļ���С�ĸ�32λ
					//����ֵ:�ļ���С�ĵ�32λ;
				tips:
					GetFileSizeEx�����ļ����߼���С,��GetCompressedFileSize�����ļ��������С;
			�����ļ�ָ��λ��(���ݶ�ȡ����ʼλ��):
				BOOL SetFilePointerEx(HADNLE hFile,LARGE_INTEGER liDistanceToMove,PLARGE_INTEGER pliNewFilePointer,DWORD dwMoveMethod);
					//@hFile:...
					//@liDistanceToMove:�ļ�ָ���ƶ��ľ���,����methodӵ�в�ͬ����;
					//@ploNewFilePointer:����ָ�����ֵ;
					//@dwMoveMethod:ָ�����ʹ��liDistanceToMove
						$FILE_BEGIN:�ļ�ָ���趨ΪliDistanceToMove�趨�Ĳ���ֵ,������ͳ��޷���64λֵ
						$FILE_CURRENT:�ļ�ָ��=��ǰָ��ֵ+liDistanceToMove,�з���64λֵ
						$FILE_END:�ļ�ָ��=fileSize+liDistanceToMove,�з���64λֵ
			�����ļ�β:���õ�ǰ�ļ�ָ���λ��Ϊ�ļ�β;
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
		ִ���豸IO
			��:
				BOOL ReadFile(HANDLE,PVOID,DWORD,DWORD,OVERLAPPED*);
					//@hFile:ʹ�õ��豸���
					//@pvBuffer:���ݻ�����
					//@dwNumBytesToRead:ϣ����ȡ���ֽ���
					//@pdwNumBytes:ʵ�ʶ�ȡ���ֽ���
					//@pOverlapped:�ص��ṹ,�����첽��ȡ
			д:
				BOOL WriteFile(HANDLE,CONST VOID*,DWORD,PDWORD,OVERLAPPED*);
					//ͬ��

			ˢ�����豸:����������ֱ��д�뵽�豸
				BOOL FlushFileBuffers(HANDLE hFile);

			ͬ��IO��ȡ��:��ĳ���߳�����ͬ��IO���������ʱ,����ʹ�ø÷�ʽ�����߳�.�����߳��ڴ���ʱ��Ҫ��THREAD_TERMINATEȨ��.
				BOOL CancelSynchronousIo(HANDLE hTrhead);
			
			tips:
				ʹ��GENERIC_READ��־�򿪵��豸����ʹ��ReadFile;
				ʹ��GENERIC_WRITE��־�򿪵��豸����ʹ��WriteFile,FlushFileBuffers;
				�첽����read/writeʱ,���ǻ᷵��false;
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
		�첽IO
			#1:ָ��FILE_FLAG_OVERLAPPED���豸
			#2:���ö�/д������������뵽�豸�Ķ���


		typedef struct _OVERLAPPED{
			DWORD Internal;
			DWORD InternalHigh;
			DWORD Offset;
			DWORD OffsetHigh;
			HANDLE hEvent;
		}OVERLAPPED,*LPOVERLAPPED;
			@Offset:��32λֵ,��offsetHigh��ָͬ���ļ�ƫ����(��ʵ��ȡ���ݵ�λ��)
			@OffsetHigh:��32λֵ,��offset��ָͬ��ƫ����
			@Internal:������
			@InternalHigh:�����ֽ���Ŀ
			@hEvent:�����¼���ɵ�֪ͨ.Ҳ�����ڱ�������

		�첽IOע������:
			#1:�豸�������򲻱��������ȳ��ķ�ʽ��������е�IO����;
			#2:����ȷ�ķ�ʽ������
			#3:һ�������ƶ���������IO����ʱʹ�õ����ݻ����ַ��OVERLAPPED�ṹ

		ȡ�������е��豸IO����:
			#1:����CancelIo,�������߳̾��,�ر��ɸ��߳���ӵ�����IO����;BOOL CancelIo(HADNLE,hFile);
			#2:�ر��豸���,ȡ�����е�����IO����;
			#3:�߳���ֹ,��ȡ�����̵߳�IO����;
			#4:BOOL CancelIoEx(HANDLE hFile,LPOVERLAPPED pOverlapped); ָ�����������ָ��IO����;
			//��ȡ����IO����᷵�ش�����ERROR_OPERATION_ABORTED;

		����IO���֪ͨ:
			#1:�����豸�ں˶���;
			#2:�����¼��ں˶���;
			#3:ʹ�ÿ�����IO;
			#4:ʹ��IO��ɶ˿�;

			##�����豸�ں˶���:
				ԭ��:�ڽ��첽IO����������ʱ,���豸�ں˶�����Ϊδ����״̬,����ϵͳִ�����IO��,�ٽ��豸�ں���Ϊ�ɴ���״̬,ĳ���ȴ��߳̽�������ִ��;
				tips:��ʹ���豸�ں˶��󴥷���ʱ��,���������IO�����,ĳ��IO�������֮���豸�ں˱�����,�������޷���֪,"��һ��"���󱻴�����!!
			##�����¼��ں˶���:
				ԭ��:����ԭ��ʹ����豸�ں˶�����ͬ,���ǽ���˲��ֱܷ�IO����ľ���;
				tips:�����ǲ���IO�����ʱ��,read/write��һ��overlapped����,��������ָ��һ���¼��ں˶���,\
					 ���ǿ��Ը����¼��ں˶������ж����ĸ����������,������Ҫ���Ǵ�����ͬ���¼�����
			##������IO:
				ԭ��:ϵͳ�����̵߳�ʱ��,��ͬʱ����һ���߳���صĶ���,�̲߳���io���󲢼��뵽�豸����,�豸���io����֮��Ὣ֪ͨ���뵽�����̵߳�APC������,\
					�����̴߳��ڿ�����״̬,��ô�ͻ���ô�����.
				tips:
					������Ҫʹ��ReadFIleEx/WriteFileEx����������io����,��Ϊ��Ҫ���ݻص�����
					�ص�����ԭ��:
						VOID WINAPI CompletionRoutine(DWORD dwError,DWORD dwNumBytes,OVERLAPPED* po);
					APC�������ܲ�������ִ��,Ϊ��ִ��APC����,�߳���Ҫ�ǿ����ѵ�,ͨ��һ��6����������:
						#1:DWORD SleepEx(DWORD,BOOL);
						#2:DWORD WaitForSingleObjectEx(HANDLE,DWORD,BOOL);
						#3:DWORD WaitForMultipleObjectsEx(...);
						#4:BOOL SignalObjectAndWait(...);
						#5:BOOL GetQueueCompletionStatusEx(...);
						#6:DWORD MsgWaitForMultipleObjectsEx(...);
					�����Ѻ���������:
					 bad:
						�ص�����:
							�ص�������Ҫ��������������Ϣ,��Ҫ����Щ��Ϣ����ȫ�ֱ�������;
						�߳�����:
							����������߳�,���������ʱ���д���.(����/������ͬһ���߳�)
					 good:
						���ǿ����ֶ��Ľ�һ����ӵ��̵߳�APC����֮��
							DWORD QueueUserAPC(PAPCFUNC pfnFun,HANDLE hThread,ULONG_PTR dwData);
								//@pfnFun:APC���ú���
								//@hThread:ָ�����߳�
								//@dwData:�������ò���
							����ԭ��:VOID WINAPI APCFunc(ULONG_PTR dwParam);
							tips:pfnFunc��hThreadӦ�ô���ͬһ�����̿ռ���;
						QueueUserAPC�������Խ��зǳ���Ч���̼߳�ͨ��;
	*/


	namespace async_file_read_and_write {

		char buff[1024];	//��������
		PCTSTR file_name = "hi.txt";	//���ļ�
		HANDLE hFile{ INVALID_HANDLE_VALUE };	//�ļ����

		

	


		

		void open_file()
		{
			hFile = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_FLAG_OVERLAPPED | FILE_ATTRIBUTE_NORMAL, nullptr);
		}

		//��ȡ�̷߳���IO��ȡ����
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

		//д���̷߳���IOд������
		DWORD WINAPI io_write(PVOID pvParam)
		{
			using namespace std;
			LPOVERLAPPED overlap_to_write = (LPOVERLAPPED)(pvParam);
			
			string data{ "\nthis is test data.\n" };	//д������
			LARGE_INTEGER li{ 0 }, pos{ 0 };
			GetFileSizeEx(hFile, &li);
			overlap_to_write->Offset = li.LowPart;
			overlap_to_write->OffsetHigh = li.HighPart;
			//bool flag_a=SetFilePointerEx(hFile, li, &pos, FILE_BEGIN); �첽д����ļ�ָ��λ����Ҫ��overlap�ṹ��ָ��
			bool flag = WriteFile(hFile, data.c_str(), data.size(), nullptr, overlap_to_write);
			DWORD dw = GetLastError();
			if (!flag&&dw != ERROR_IO_PENDING)
			{
				cout << "return write_file:" << dw << "\n";
				return -1;
			}
			return 0;
		}

		//ʹ���豸�ں˶�����д���
		void device_obj_test()
		{
			using namespace std;
			DWORD id_read, id_write;
			OVERLAPPED o_read{ 0 }, o_write{ 0 };
			//��ʼ��hFile
			if (hFile == INVALID_HANDLE_VALUE)
			{
				open_file();
				if (hFile == INVALID_HANDLE_VALUE) {
					cout << "open file " << file_name << " failed\n";
					return;
				}
			}
			//ʹ�����̷߳���IO����
			auto hd_read = CreateThread(nullptr, 0, io_read, &o_read, 0, &id_read);
			//���̵߳ȴ�IO�������
			DWORD dw=WaitForSingleObject(hFile, INFINITE);
			//���ؽ�����
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

		//ʹ���¼��ں˶�����д���
		void event_obj_test()
		{
			using namespace std;
			//��ʼ��hFile
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
			//���̷߳����дIO����
			HANDLE hd_read = CreateThread(nullptr, 0, io_read, &o_read, 0, &id_read);
			HANDLE hd_write = CreateThread(nullptr, 0, io_write, &o_write, 0, &id_write);

			//��������
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

		
		//��ɺ���
		VOID WINAPI read_done(DWORD dwErrir,DWORD dwNumBytes,OVERLAPPED* po)
		{
			using namespace std;
			cout << "read event done\n";
			cout << "read " << dwNumBytes << " bytes\n";
			cout << buff << endl;
		}

		//ͨ���̵߳Ŀ�����״̬
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