#pragma once
#include<Windows.h>
#include<iostream>
#include<iomanip>
#include<exception>
#include<chrono>
#ifndef Windows_Memory_Explore_H
#define Windows_Memory_Explore_H

/*
	本节主要探讨windows内存管理的知识,由于许多都是概念性的知识,所以这里把unit13-unit18综合来表述

	重要的概念:
		1:页帧(frame):将物理内存分成等大小(取2的幂次方)的空间,每一份空间称为页帧(目前32位机器常用页帧为4KB,64位为8KB);
		2:页面(page):将虚拟内存分成等大小(取2的幂次方)的空间,每一份空间称为页面,一般与页帧大小相同
		3:虚拟内存地址:虚拟内存地址是操作系统的抽象,通常包含实际的物理内存和页面交换文件,使得我们可用的"内储存空间"超过了实际物理内存大小
		4:预定(reserve):进程告诉操作系统希望预定地址空间中的某一块地址,但预定操作此时并不分配任何实际的物理内存;(VirtualAlloc)
		5:调拨(commit):操作系统分配实际的物理内存给调用进程,将虚拟地址空间中的预定区域映射到实际的物理地址(VirtualAlloc)
		6:颠簸(thrash):物理内存和页交换文件之间的频率.频率越高,系统颠簸越厉害,运行的也越慢.
		7:撤销调拨/释放(decommit/free):回收释放该区域,该操作以页面(page)为基本单位,回收包含(addr+dwsize)区域的所有页面(VirtualFree)

	进程所见的内存空间:
		1:地址:进程所见的地址都是虚拟内存地址,将虚拟地址映射成物理地址是操作系统的工作;
		2:用户空间:虚拟地址被分为不同的功能区域(空指针区,用户区,64K禁入区,内核区),在程序员的内存模型中,使用的几乎都是用户区,堆段(通常位于高地址)所分配的内存不会超过用户区的高地址顶部,低地址相同
	
	虚拟地址的意义:
		1:假设A,B是两个程序,那么A访问0x12345678的数据和B访问0x12345678的数据是不会冲突的,他们的虚拟地址将由操作系统翻译成相应的物理地址并从该处取得相应数据,
		而A所见的区域内,只有A和操作系统,是察觉不到B的存在.B也相同.所以A,B若没有提供相应的交流手段,互相不可见.(隔离性)
		2:由于程序A只能察觉自己和操作系统的存在,且对于内核空间的内容,A是没有权限进行更改的(会违规),提供了一定的安全保障.
		3:虚拟地址突破了物理地址的桎梏,当使用页交换技术的时候,能使自己看起来比实际内存容量更大.相当于虚拟内存容量=物理内存容量+页面交换文件大小;
		4:当页交换文件有效的被使用的时候,即系统颠簸发生的越少,系统的吞吐率能得到提高;但若预测不当,那么系统运行也会被阻碍(大量的置换页面的开销);

	虚拟地址的预定/调拨/释放等:
		1:仅预定(reserve):VirtualAlloc(pvAddress,dwSize,MEM_RESERVE,fdwProtect);
		2:仅调拨(commit):VirtualAlloc(pvAddress,dwSize,MEM_COMMIT,fdwProtect);
		3:预定并且调拨(reserve&&commit):VirtualAlloc(pvAddress,dwSize,MEM_RESERVE|MEM_COMMIT,fdwProtect);
		4:撤销调拨或释放(decommit||free):VirtualFree(pvAddress,dwSize,MEM_DECOMMIT)||VirtualFree(pvAddress,0,MEM_RELEASE);
		5:改变页面保护属性:VirtualProtect(pvAddress,dwSize,flNewProtect,pflOldProtect);

	线程栈:
		1:线程栈是一块虚拟内存区域,从栈顶到带有保护属性标志(PAGE_GUARD)的已调拨页面PG之间是已调拨页面,从PG到栈底是已预定但未调拨的页面;
		2:调拨新页面的时候,OS擦去当前PG的保护属性,将指针指向下一块页面,为其调拨页面并写上保护属性;
		3:在调拨页面的时候,设当前保护页面为页面2(已调拨),下一块页面为页面1(仅预定),再下一块页面为页面0(仅预定且为栈底).此时OS将擦去页面2的保护属性,同时调拨页面给页面1,并抛出栈溢出异常,但不会给页面1写上保护属性
		4:步骤3是为了防止栈越界.当程序收到异常仍然继续访问页面0时,由于未分配物理内存会发生访问违规,此时OS一般会退出当前进程(而不是线程).

	内存文件映射:
	创建流程:
		1:创建或打开一个内核对象,标识需要映射的磁盘文件(CreateFile函数)
		2:创建文件映射内核对象,指出文件大小及其文件访问方式(CreateFileMapping函数)
		3:将文件数据映射到进程的地址空间(MapViewOfFile函数)
		清理流程:
		1:撤销地址空间的映射(UnmapViewOfFile函数)
		2:关闭文件映射内核对象及内核对象(CloseHandle)

		在之前的学习中,有一组标准IO函数用于文件的读取和写入(ReadFile/WriteFile),它的执行流程大约如下:
		ReadFile/WriteFile-->陷入操作系统代码,进行系统调用-->等待IO事件完成,数据写入操作系统空间-->将数据从系统区复制到用户区,函数返回(同步情况下);
		//这里有两个比较明显的开销:系统调用和内存复制
		而若是使用内存文件映射IO,大致流程如下:
		CreateFileMapping创建映射文件-->MapViewOfFile将文件数据映射到进程的地址空间-->使用数据
		//这里的开销是将文件数据一部分装载到用户内存空间,由于文件映射也是基于页面的,使用文件映射也会发生"缺页"和"颠簸"现象,情况和虚拟技术类似

	堆:
		1:堆适合分配大量的小型数据
		2:进程创建时,会生成一个默认堆,1MB大小
		3:获得默认堆的句柄:GetProcessHeap()
		4:创建堆:HeapCreate();
		5:从堆中分配内存块:HeapAlloc();
		6:调整已分配内存块大小:HeapReAlloc();
		7:获得内存块的大小:HeapSize();
		8:释放:HeapFree();
		9:销毁:HeapDestroy();

	几个重要对象:(具体释义请浏览msdn)
		typedef struct _SYSTEM_INFO{
			union{
				struct {
					WORD wProcessorArchitecture;	//处理器的体系结构
					WORD wReserved;					//保留
				};
			};
			DWORD dwPageSize;						//页面大小
			LPVOID lpMininumApplicationAddress;		//进程可用最小内存地址
			LPVOID lpMaxinumApplicationAddress;		//进程可用最大内存地址
			DWORD_PTR dwActiveProcessorMask;		//表示处于活动状态的CPU掩码
			DWORD dwNumberOfProcessors;				//CPU核心数量
			DWORD dwProcessorType;					//已作废
			DWORD dwAlloctionGranularity;			//分配粒度
			DWORD wProcessorLevel;					//细分处理器的体系结构
			DWORD wProcessorRevision;				//level的进一步细分
		}SYSTEM_INFO,* LPSYSTEM_INFO

		typedef struct _MEMORYSTATUS{
			DWORD deLength;							//结构的大小,必须初始化,通常为sizeof(MEMORYSTATUS);
			DWORD dwMemoryLoad;						//内存载入率,使用率,是一个大致的估值
			SIZE_T dwTotalPhys;						//物理内存总量
			SIZE_T dwAvailPhys;						//可用物理内存总量
			SIZE_T dwTotalPageFile;					//页交换文件的总量
			SIZE_T dwAvailPageFile;					//可用页交换文件的大小
			SIZE_T dwTotalVirtual;					//各进程私有的字节数
			SIZE_T dwAvailVirtual;					//进程可用私有内存量
		}MEMORYSTATUS,*LPMEMORYSTATUS;

		typedef struct _MEMORY_BASIC_INFOMATION{
			PVOID BaseAddress;						//起始地址空间
			PVOID AllocationBase;					//区域的基地址
			DWORD AllocationProtect;				//区域的保护属性
			SIZE_T RegionSize;						//区域大小
			DWORD State;							//区域中页面的状态
			DWORD Protect;							//相邻页面的保护属性
			DWORD Type;								//区域页面的类型
		}MEMORY_BASIC_INFOMATION,*PMEMORY_BASIC_INFOMATION;

	一些介绍的函数:
		

*/


namespace window_memory_explore {
	using namespace std;

	const int attr_len = 15;
	const int val_len = 20;

	const int mbi_column[7] = {10,15,20,15,10,10,10};	 //列表输出mbi结构时,列的大小;

	ostream& operator<<(ostream& os, const SYSTEM_INFO& si)
	{
		os << "page size:" << si.dwPageSize << "\n";
		os << "min addr:" << si.lpMinimumApplicationAddress << "\n";
		os << "max addr:" << si.lpMaximumApplicationAddress << "\n";
		os << "alloc_granularity:" << si.dwAllocationGranularity << "\n";
		return os;
	}

	void show_system_info()
	{
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		cout << si << endl;
	}


	ostream& operator<<(ostream& os, const MEMORYSTATUSEX& ms)
	{
		os << "memory usage:" << ms.dwMemoryLoad << "\n";
		os << "total phys:" << ms.ullTotalPhys << "\n";
		os << "avail phys:" << ms.ullAvailPhys << "\n";
		os << "total page file:" << ms.ullTotalPageFile << "\n";
		os << "avail page file:" << ms.ullAvailPageFile << "\n";
		os << "total virtual:" << ms.ullTotalVirtual << "\n";
		os << "avail virtual:" << ms.ullAvailVirtual << "\n";
		return os;
	}

	void show_memory_status()
	{
		MEMORYSTATUSEX ms;
		ms.dwLength = sizeof(MEMORYSTATUSEX);
		auto rtn=GlobalMemoryStatusEx(&ms);
		if (rtn)
		{
			cout << ms << endl;
		}
	}

	
	


	ostream& operator<<(ostream& os, const MEMORY_BASIC_INFORMATION& mbi)
	{
		os << setw(mbi_column[0]) << left << mbi.BaseAddress ;
		os << setw(mbi_column[4]) << left;
		switch (mbi.State)
		{
		case MEM_RESERVE:
			os << "RESERVE";
			break;
		case MEM_COMMIT:
			os << "COMMIT";
			break;
		default:
			os << "FREE";
			break;
		}
		os << setw(mbi_column[3]) << left << hex << showbase << mbi.RegionSize;
		if (mbi.State != MEM_FREE)
		{
			os << setw(mbi_column[1]) << left << hex << showbase << mbi.BaseAddress;
			os << setw(mbi_column[2]) << left;
			switch (mbi.AllocationProtect)
			{
			case PAGE_EXECUTE:
				os << "--E-";
				break;
			case PAGE_EXECUTE_READ:
				os << "R-E-";
				break;
			case PAGE_EXECUTE_READWRITE:
				os << "RWE-";
				break;
			case PAGE_EXECUTE_WRITECOPY:
				os << "-WEC";
				break;
			case PAGE_NOACCESS:
				os << "----";
				break;
			case PAGE_READONLY:
				os << "R---";
				break;
			case PAGE_READWRITE:
				os << "RW--";
				break;
			case PAGE_WRITECOPY:
				os << "-W-C";
				break;
			default:
				os << "UNKNOWN";
				break;
			}
			
		}

		switch (mbi.Type)
		{
		case MEM_IMAGE:
			os << "IMAGE";
			break;
		case MEM_MAPPED:
			os << "MAPPED";
			break;
		default:
			os << "PRIVATE";
			break;
		}
	
		os << "\n";
		
		return os;
	}

	void show_title_bar()
	{
		cout << setw(mbi_column[0]) << left << "base_addr";
		cout << setw(mbi_column[4]) << left << "state";
		cout << setw(mbi_column[3]) << left << "region_size";
		cout << setw(mbi_column[1]) << left << "alloc_base";
		cout << setw(mbi_column[2]) << left << "alloc_protect";
		cout << setw(mbi_column[6]) << left << "type";
		cout << endl;
		
	}


	//打印当前进程的内存视图的快照
	void show_process_memory_status()
	{
		void* st_addr = nullptr;
		MEMORY_BASIC_INFORMATION mbi;
		DWORD len = sizeof(mbi);
		show_title_bar();
		while (VirtualQuery(st_addr,&mbi,len))
		{
			cout << mbi << endl;
			st_addr = (void*)((char*)mbi.BaseAddress + mbi.RegionSize);
		}
	}

	void show_memory_basic()
	{
		MEMORY_BASIC_INFORMATION mbi;
		DWORD len = sizeof(mbi);
		VirtualQuery(nullptr, &mbi, len);
		cout << mbi << endl;
	}

	//虚拟地址分配的包装类
	class virtual_alloctor {
	public:
		/*
		预定地址空间区域,此时返回的地址不可访问,未分配物理内存
		分配的大小向上取整到页面大小的整数倍
		*/
		void* alloc_reserve(size_t size, bool top_down = false, DWORD protect_attr = PAGE_READWRITE)
		{
			DWORD alloc_type = MEM_RESERVE;
			alloc_type = top_down ? (alloc_type | MEM_TOP_DOWN) : alloc_type;
			return VirtualAlloc(nullptr, size, alloc_type, protect_attr);
		}

		/*
		调拨物理存储器内存
		*/
		void* alloc_commit(void* addr, size_t size, DWORD protect_attr = PAGE_READWRITE)
		{
			return VirtualAlloc(addr, size, MEM_COMMIT, protect_attr);
		}

		//预定并调拨物理存储器内存
		void* alloc(size_t size, DWORD protect_attr = PAGE_READWRITE)
		{
			return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, protect_attr);
		}

		//改变区域的保护属性
		bool protect_attr_reset(void* addr, size_t size, DWORD protect_attr_new, PDWORD protect_attr_old)
		{
			return VirtualProtect(addr, size, protect_attr_new, protect_attr_old);
		}

		//释放部分物理存储器
		bool decommit(void* addr, size_t size)
		{
			return VirtualFree(addr, size, MEM_DECOMMIT);
		}

		//释放整块区域
		bool free(void* addr)
		{
			return VirtualFree(addr, 0, MEM_FREE);
		}

	};

	//文件映射的包装类
	class memory_mapped {
	public:
		memory_mapped(HANDLE hFile, PSECURITY_ATTRIBUTES psa, DWORD fdwProtect, DWORD dwMaxSizeHigh, DWORD dwMaxSizeLow, PCTSTR pszName)
		{
			hFileMapping = CreateFileMapping(hFile, psa, fdwProtect, dwMaxSizeHigh, dwMaxSizeLow, pszName);
			if (hFileMapping == INVALID_HANDLE_VALUE)
			{
				throw exception{ "CreateFileMapping Failed" };
			}
		}
		memory_mapped(const memory_mapped& other) = delete;
		memory_mapped(memory_mapped&& other)
		{
			hFileMapping = other.hFileMapping;
			other.hFileMapping = INVALID_HANDLE_VALUE;
		}
		memory_mapped& operator=(const memory_mapped& other) = delete;
		memory_mapped& operator=(memory_mapped&& other) 
		{
			hFileMapping = other.hFileMapping;
			other.hFileMapping = INVALID_HANDLE_VALUE;
		}
		void* map_view_of_file(DWORD dwDesireAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap)
		{
			return MapViewOfFile(hFileMapping, dwDesireAccess, dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap);
		}
		BOOL unmap_view_of_file(void* base_addr)
		{
			return UnmapViewOfFile(base_addr);
		}
		~memory_mapped()
		{
			if (hFileMapping != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFileMapping);
			}
		}
	private:
		HANDLE hFileMapping;
	};

	
	//比较标准io和文件映射的读取速度
	class read_comp {
	public:
		read_comp(const char* filename)
		{
			bool flag = false;
			DWORD file_flag = flag ? FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING : FILE_ATTRIBUTE_NORMAL;
			hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, file_flag, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				throw exception{ "create file failed" };
			}
		}

		read_comp(const read_comp& other) = delete;
		read_comp& operator=(const read_comp& other) = delete;
		read_comp(read_comp&& other) = delete;
		read_comp& operator=(read_comp&& other) = delete;
		~read_comp()
		{
			if (hFile != INVALID_HANDLE_VALUE)
				CloseHandle(hFile);
			if (hFileMapping != INVALID_HANDLE_VALUE)
				CloseHandle(hFileMapping);
		}

		LARGE_INTEGER file_size()
		{
			LARGE_INTEGER fs{};
			GetFileSizeEx(hFile, &fs);
			return fs;
		}




		void read_by_std_io()
		{
			cout << "read by std io 64:\n";
			LARGE_INTEGER file_sz=file_size();
			LARGE_INTEGER buf_sz{ 4096 };
			LARGE_INTEGER offset{ 0 };
			LARGE_INTEGER byte_read_total{ 0 };
			DWORD byte_read{ 0 };
			cout << "file size:" << file_sz.QuadPart << "\n";
			SetFilePointerEx(hFile, offset, nullptr, FILE_BEGIN);	//将指针移动到文件首部
			while (byte_read_total.QuadPart<file_sz.QuadPart)
			{
				if (ReadFile(hFile, buf, buf_sz.QuadPart, &byte_read, nullptr) == TRUE)
				{
					byte_read_total.QuadPart += byte_read;
				}
				else {
					cout << "read file failed:" << GetLastError() << endl;
					break;
				}
			}
			cout << "read_total:" << byte_read_total.QuadPart << endl;
		}

		void read_by_std_io_32()
		{
			cout << "read by std io 32:\n";
			LARGE_INTEGER li = file_size();
			DWORD fs = li.LowPart;
			DWORD byte_read{};
			const DWORD byte_to_read{4096};
			DWORD byte_read_total{};
			cout << "file size:" << fs << endl;
			li.QuadPart = 0;
			SetFilePointerEx(hFile, li, nullptr, FILE_BEGIN);
			while (byte_read_total < fs)
			{
				if (ReadFile(hFile, buf, byte_to_read, &byte_read, nullptr)==TRUE)
				{
					byte_read_total += byte_read;
				}
				else {
					cout << "read file failed:" << GetLastError() << endl;
					break;
				}
				
			}
			cout << "bytes read total:" << byte_read_total << endl;
		}

		void read_by_file_map()
		{
			cout << "read by fila mapped:\n";
			HANDLE file_mapped = CreateFileMapping(hFile, nullptr, PAGE_READONLY, 0, 4096, nullptr);
			if (file_mapped == INVALID_HANDLE_VALUE)
				throw exception{ "file map fialed" };
			LARGE_INTEGER fs = file_size();
			LARGE_INTEGER bs{ 4096 };
			LARGE_INTEGER offset{ 0 };
			cout << "file_size:" << fs.QuadPart << endl;
			/*
				这里有一个疑问:如果需要读取文件的下个部分,需要不断的map/unmap吗?
				answer: For files that are larger than the address space, you can only map a small portion of the file data at one time. When the first view is complete, you can unmap it and map a new view.
				url:https://docs.microsoft.com/zh-cn/windows/desktop/api/memoryapi/nf-memoryapi-mapviewoffile
			*/
			while (offset.QuadPart<fs.QuadPart)
			{
				void* addr = MapViewOfFile(hFileMapping, FILE_MAP_READ, offset.HighPart, offset.LowPart, bs.QuadPart);
				offset.QuadPart += bs.QuadPart;
				UnmapViewOfFile(addr);
			}
			cout << "offset:" << offset.QuadPart << endl;

		}
	private:
		HANDLE hFile;
		HANDLE hFileMapping;
		char buf[4096];
	};

	
	//用于统计read_comp的成员函数的耗时
	void time_statistics(read_comp* prc,void(read_comp::*func)())
	{
		using namespace std::chrono;
		auto t1 = steady_clock::now();
		(prc->*func)();
		auto t2 = steady_clock::now();
		auto used_time = t2 - t1;
		cout << dec << duration_cast<chrono::milliseconds>(used_time).count() << "ms" << endl;
	}

	//读取比较测试
	void read_compare_test()
	{
		const char* filename = "//use your file";
		read_comp* rc = new read_comp{ filename };
		using rc_func = void(read_comp::*)();
		rc_func rio_32 = &read_comp::read_by_std_io_32;
		rc_func rio = &read_comp::read_by_std_io;
		rc_func rmp = &read_comp::read_by_file_map;
		for (int i = 0; i < 10; ++i)
		{
			time_statistics(rc, rio_32);
			time_statistics(rc, rio);
			time_statistics(rc, rmp);
		}
	}

	//windows堆的包装类
	class windows_heap {
	public:
		windows_heap(DWORD fdwOptions, SIZE_T dwInitialSize, SIZE_T dwMaximumSize) 
		{
			m_heap = HeapCreate(fdwOptions, dwInitialSize, dwMaximumSize);
		}
		windows_heap(const windows_heap& other) = delete;
		windows_heap& operator=(const windows_heap other) = delete;
		windows_heap(windows_heap&& other) { m_heap = other.m_heap; other.m_heap = INVALID_HANDLE_VALUE; }
		windows_heap& operator=(windows_heap&& other){ m_heap = other.m_heap; other.m_heap = INVALID_HANDLE_VALUE; }
		~windows_heap()
		{
			HeapDestroy(m_heap);
		}

		void* alloc(DWORD dwFlags, SIZE_T dwBytes) { return HeapAlloc(m_heap, dwFlags, dwBytes); }
		void* realloc(DWORD dwFlags, void* pvMem, SIZE_T dwBytes) { return HeapReAlloc(m_heap, dwFlags, pvMem, dwBytes); }
		SIZE_T mem_size(DWORD dwFlags, void* pvMem) { return HeapSize(m_heap, dwFlags, pvMem); }
		BOOL mem_free(DWORD dwFlags, void* pvMem) { return HeapFree(m_heap, dwFlags, pvMem); }
	private:
		HANDLE m_heap;
	};

	void heap_test()
	{
		windows_heap wh{ HEAP_GENERATE_EXCEPTIONS,2 * 1024 * 1024,4 * 1024 * 1024 };	//申请堆空间,初始2MB,最大4MB,若分配失败抛出异常
		void* addr=wh.alloc(HEAP_GENERATE_EXCEPTIONS, 1 * 1024);	//申请1KB的内存块
		cout << wh.mem_size(0, addr) << endl;
		addr = wh.realloc(HEAP_GENERATE_EXCEPTIONS, addr, 2 * 1024);//重新申请2KB的内存块
		cout << wh.mem_size(0, addr) << endl;
		wh.mem_free(0, addr);
		
	}

	void _test()
	{
		read_compare_test();
		//heap_test();
	}
}




#endif // !Windows_Memory_Explore_H


/*
	
	windows内存管理小结:
		1:物理地址和虚拟地址属于某种映射关系,具体由操作系统定义
		2:程序可见的地址都是虚拟地址,从程序的角度来看,虚拟地址的"用户"区域都可使用;
		3:虚拟地址的用户区域通常可划分(视)为"堆栈结构",常见的程序员角度,(即堆在顶端向下生长,栈从底部向上生长,栈之下是数据段代码段等等,堆之上是内核空间);
		4:当程序要访问一个虚拟地址的时候,需要操作系统替该地址进行预定和调拨,映射到实际的物理地址,否则会发生访问违规
		5:windows进程产生的时候,会创建默认堆和默认线程栈,堆空间是默认同步的,如果需要高效率使用堆空间,我们可以申请属于自己的堆空间
		6:内存文件映射从读取的角度来说的确优于标准io函数(每次进程重新启动后,read_by_file_map耗时总是小于read_by_std_io),测试程序读取了一个197MB的视频文件,其中文件映射方式在
			55-75ms左右,标准io不开启缓存(FILE_FLAG_NO_BUFFERing),每次读取在7000ms-8000ms左右,开启缓存的后序读取在200ms-300ms之间.这里采用同步标准io读取.
		7:回顾了一下标准io函数,比如在开始用文件指针的时候,卡了壳...没有将指针置回文件首部.
*/
