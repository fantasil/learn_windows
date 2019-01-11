#pragma once
#include<Windows.h>
#include<iostream>

#ifndef Windows_General_H
#define Windows_General_H





/*
	��һЩ���õĺ���,����,���������
*/

namespace general {

	namespace const_val {
		const int buf_sz = 1024;
	}


	namespace general_func {

		//ͬ��������
		template<class windows_mutex,class Arg1,class... Args>
		void sync_print(windows_mutex& wm, Arg1 arg1, Args... args)
		{
			using namespace std;
			WaitForSingleObject(wm, INFINITE);
			cout << arg1;
			sync_print(wm, args...);
			ReleaseMutex(wm);
		}

		template<class windows_mutex,class Arg>
		void sync_print(windows_mutex& wm, Arg arg)
		{
			using namespace std;
			WaitForSingleObject(wm, INFINITE);
			cout << arg << endl;
			ReleaseMutex(wm);
		}

		

	}

	namespace general_class {
		
		//thread���ݵ���Ϣ�ļ򵥰�װ,������Դ�̺߳�����
		struct msg_for_thread {
			DWORD tid_send;
			const char* data;
		};
		
		//ָ��io����,op_kill����ָʾ���߳������Լ�
		enum class io_op {
			op_read,op_write,op_kill
		};

		//�Զ���overlapped����,�����ָʾio���������ݽṹ
		struct my_overlapped :OVERLAPPED {
			io_op m_op;
			my_overlapped() :OVERLAPPED{ 0 }, m_op{ io_op::op_read } {}
			explicit my_overlapped(io_op op) :OVERLAPPED{ 0 }, m_op{ op } {}
			my_overlapped(OVERLAPPED overlapped, io_op op) :OVERLAPPED{ overlapped }, m_op{ op } {}
		};

	}



}

#endif // !Windows_General_H