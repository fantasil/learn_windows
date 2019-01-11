#pragma once
#include<Windows.h>
#include<iostream>

#ifndef Windows_General_H
#define Windows_General_H





/*
	将一些常用的函数,变量,类放在这里
*/

namespace general {

	namespace const_val {
		const int buf_sz = 1024;
	}


	namespace general_func {

		//同步输出语句
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
		
		//thread传递的消息的简单包装,仅包含源线程和数据
		struct msg_for_thread {
			DWORD tid_send;
			const char* data;
		};
		
		//指明io操作,op_kill用来指示子线程清理自己
		enum class io_op {
			op_read,op_write,op_kill
		};

		//自定义overlapped子类,添加了指示io操作的数据结构
		struct my_overlapped :OVERLAPPED {
			io_op m_op;
			my_overlapped() :OVERLAPPED{ 0 }, m_op{ io_op::op_read } {}
			explicit my_overlapped(io_op op) :OVERLAPPED{ 0 }, m_op{ op } {}
			my_overlapped(OVERLAPPED overlapped, io_op op) :OVERLAPPED{ overlapped }, m_op{ op } {}
		};

	}



}

#endif // !Windows_General_H