#pragma once
#include<Windows.h>
#include<iostream>
#include<iomanip>
#include<exception>
#include<chrono>
#ifndef Windows_Memory_Explore_H
#define Windows_Memory_Explore_H

/*
	������Ҫ̽��windows�ڴ�����֪ʶ,������඼�Ǹ����Ե�֪ʶ,���������unit13-unit18�ۺ�������

	��Ҫ�ĸ���:
		1:ҳ֡(frame):�������ڴ�ֳɵȴ�С(ȡ2���ݴη�)�Ŀռ�,ÿһ�ݿռ��Ϊҳ֡(Ŀǰ32λ��������ҳ֡Ϊ4KB,64λΪ8KB);
		2:ҳ��(page):�������ڴ�ֳɵȴ�С(ȡ2���ݴη�)�Ŀռ�,ÿһ�ݿռ��Ϊҳ��,һ����ҳ֡��С��ͬ
		3:�����ڴ��ַ:�����ڴ��ַ�ǲ���ϵͳ�ĳ���,ͨ������ʵ�ʵ������ڴ��ҳ�潻���ļ�,ʹ�����ǿ��õ�"�ڴ���ռ�"������ʵ�������ڴ��С
		4:Ԥ��(reserve):���̸��߲���ϵͳϣ��Ԥ����ַ�ռ��е�ĳһ���ַ,��Ԥ��������ʱ���������κ�ʵ�ʵ������ڴ�;(VirtualAlloc)
		5:����(commit):����ϵͳ����ʵ�ʵ������ڴ�����ý���,�������ַ�ռ��е�Ԥ������ӳ�䵽ʵ�ʵ������ַ(VirtualAlloc)
		6:����(thrash):�����ڴ��ҳ�����ļ�֮���Ƶ��.Ƶ��Խ��,ϵͳ����Խ����,���е�ҲԽ��.
		7:��������/�ͷ�(decommit/free):�����ͷŸ�����,�ò�����ҳ��(page)Ϊ������λ,���հ���(addr+dwsize)���������ҳ��(VirtualFree)

	�����������ڴ�ռ�:
		1:��ַ:���������ĵ�ַ���������ڴ��ַ,�������ַӳ��������ַ�ǲ���ϵͳ�Ĺ���;
		2:�û��ռ�:�����ַ����Ϊ��ͬ�Ĺ�������(��ָ����,�û���,64K������,�ں���),�ڳ���Ա���ڴ�ģ����,ʹ�õļ��������û���,�Ѷ�(ͨ��λ�ڸߵ�ַ)��������ڴ治�ᳬ���û����ĸߵ�ַ����,�͵�ַ��ͬ
	
	�����ַ������:
		1:����A,B����������,��ôA����0x12345678�����ݺ�B����0x12345678�������ǲ����ͻ��,���ǵ������ַ���ɲ���ϵͳ�������Ӧ�������ַ���Ӹô�ȡ����Ӧ����,
		��A������������,ֻ��A�Ͳ���ϵͳ,�ǲ������B�Ĵ���.BҲ��ͬ.����A,B��û���ṩ��Ӧ�Ľ����ֶ�,���಻�ɼ�.(������)
		2:���ڳ���Aֻ�ܲ���Լ��Ͳ���ϵͳ�Ĵ���,�Ҷ����ں˿ռ������,A��û��Ȩ�޽��и��ĵ�(��Υ��),�ṩ��һ���İ�ȫ����.
		3:�����ַͻ���������ַ������,��ʹ��ҳ����������ʱ��,��ʹ�Լ���������ʵ���ڴ���������.�൱�������ڴ�����=�����ڴ�����+ҳ�潻���ļ���С;
		4:��ҳ�����ļ���Ч�ı�ʹ�õ�ʱ��,��ϵͳ����������Խ��,ϵͳ���������ܵõ����;����Ԥ�ⲻ��,��ôϵͳ����Ҳ�ᱻ�谭(�������û�ҳ��Ŀ���);

	�����ַ��Ԥ��/����/�ͷŵ�:
		1:��Ԥ��(reserve):VirtualAlloc(pvAddress,dwSize,MEM_RESERVE,fdwProtect);
		2:������(commit):VirtualAlloc(pvAddress,dwSize,MEM_COMMIT,fdwProtect);
		3:Ԥ�����ҵ���(reserve&&commit):VirtualAlloc(pvAddress,dwSize,MEM_RESERVE|MEM_COMMIT,fdwProtect);
		4:�����������ͷ�(decommit||free):VirtualFree(pvAddress,dwSize,MEM_DECOMMIT)||VirtualFree(pvAddress,0,MEM_RELEASE);
		5:�ı�ҳ�汣������:VirtualProtect(pvAddress,dwSize,flNewProtect,pflOldProtect);

	�߳�ջ:
		1:�߳�ջ��һ�������ڴ�����,��ջ�������б������Ա�־(PAGE_GUARD)���ѵ���ҳ��PG֮�����ѵ���ҳ��,��PG��ջ������Ԥ����δ������ҳ��;
		2:������ҳ���ʱ��,OS��ȥ��ǰPG�ı�������,��ָ��ָ����һ��ҳ��,Ϊ�����ҳ�沢д�ϱ�������;
		3:�ڵ���ҳ���ʱ��,�赱ǰ����ҳ��Ϊҳ��2(�ѵ���),��һ��ҳ��Ϊҳ��1(��Ԥ��),����һ��ҳ��Ϊҳ��0(��Ԥ����Ϊջ��).��ʱOS����ȥҳ��2�ı�������,ͬʱ����ҳ���ҳ��1,���׳�ջ����쳣,�������ҳ��1д�ϱ�������
		4:����3��Ϊ�˷�ֹջԽ��.�������յ��쳣��Ȼ��������ҳ��0ʱ,����δ���������ڴ�ᷢ������Υ��,��ʱOSһ����˳���ǰ����(�������߳�).

	�ڴ��ļ�ӳ��:
	��������:
		1:�������һ���ں˶���,��ʶ��Ҫӳ��Ĵ����ļ�(CreateFile����)
		2:�����ļ�ӳ���ں˶���,ָ���ļ���С�����ļ����ʷ�ʽ(CreateFileMapping����)
		3:���ļ�����ӳ�䵽���̵ĵ�ַ�ռ�(MapViewOfFile����)
		��������:
		1:������ַ�ռ��ӳ��(UnmapViewOfFile����)
		2:�ر��ļ�ӳ���ں˶����ں˶���(CloseHandle)

		��֮ǰ��ѧϰ��,��һ���׼IO���������ļ��Ķ�ȡ��д��(ReadFile/WriteFile),����ִ�����̴�Լ����:
		ReadFile/WriteFile-->�������ϵͳ����,����ϵͳ����-->�ȴ�IO�¼����,����д�����ϵͳ�ռ�-->�����ݴ�ϵͳ�����Ƶ��û���,��������(ͬ�������);
		//�����������Ƚ����ԵĿ���:ϵͳ���ú��ڴ渴��
		������ʹ���ڴ��ļ�ӳ��IO,������������:
		CreateFileMapping����ӳ���ļ�-->MapViewOfFile���ļ�����ӳ�䵽���̵ĵ�ַ�ռ�-->ʹ������
		//����Ŀ����ǽ��ļ�����һ����װ�ص��û��ڴ�ռ�,�����ļ�ӳ��Ҳ�ǻ���ҳ���,ʹ���ļ�ӳ��Ҳ�ᷢ��"ȱҳ"��"����"����,��������⼼������

	��:
		1:���ʺϷ��������С������
		2:���̴���ʱ,������һ��Ĭ�϶�,1MB��С
		3:���Ĭ�϶ѵľ��:GetProcessHeap()
		4:������:HeapCreate();
		5:�Ӷ��з����ڴ��:HeapAlloc();
		6:�����ѷ����ڴ���С:HeapReAlloc();
		7:����ڴ��Ĵ�С:HeapSize();
		8:�ͷ�:HeapFree();
		9:����:HeapDestroy();

	������Ҫ����:(�������������msdn)
		typedef struct _SYSTEM_INFO{
			union{
				struct {
					WORD wProcessorArchitecture;	//����������ϵ�ṹ
					WORD wReserved;					//����
				};
			};
			DWORD dwPageSize;						//ҳ���С
			LPVOID lpMininumApplicationAddress;		//���̿�����С�ڴ��ַ
			LPVOID lpMaxinumApplicationAddress;		//���̿�������ڴ��ַ
			DWORD_PTR dwActiveProcessorMask;		//��ʾ���ڻ״̬��CPU����
			DWORD dwNumberOfProcessors;				//CPU��������
			DWORD dwProcessorType;					//������
			DWORD dwAlloctionGranularity;			//��������
			DWORD wProcessorLevel;					//ϸ�ִ���������ϵ�ṹ
			DWORD wProcessorRevision;				//level�Ľ�һ��ϸ��
		}SYSTEM_INFO,* LPSYSTEM_INFO

		typedef struct _MEMORYSTATUS{
			DWORD deLength;							//�ṹ�Ĵ�С,�����ʼ��,ͨ��Ϊsizeof(MEMORYSTATUS);
			DWORD dwMemoryLoad;						//�ڴ�������,ʹ����,��һ�����µĹ�ֵ
			SIZE_T dwTotalPhys;						//�����ڴ�����
			SIZE_T dwAvailPhys;						//���������ڴ�����
			SIZE_T dwTotalPageFile;					//ҳ�����ļ�������
			SIZE_T dwAvailPageFile;					//����ҳ�����ļ��Ĵ�С
			SIZE_T dwTotalVirtual;					//������˽�е��ֽ���
			SIZE_T dwAvailVirtual;					//���̿���˽���ڴ���
		}MEMORYSTATUS,*LPMEMORYSTATUS;

		typedef struct _MEMORY_BASIC_INFOMATION{
			PVOID BaseAddress;						//��ʼ��ַ�ռ�
			PVOID AllocationBase;					//����Ļ���ַ
			DWORD AllocationProtect;				//����ı�������
			SIZE_T RegionSize;						//�����С
			DWORD State;							//������ҳ���״̬
			DWORD Protect;							//����ҳ��ı�������
			DWORD Type;								//����ҳ�������
		}MEMORY_BASIC_INFOMATION,*PMEMORY_BASIC_INFOMATION;

	һЩ���ܵĺ���:
		

*/


namespace window_memory_explore {
	using namespace std;

	const int attr_len = 15;
	const int val_len = 20;

	const int mbi_column[7] = {10,15,20,15,10,10,10};	 //�б����mbi�ṹʱ,�еĴ�С;

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


	//��ӡ��ǰ���̵��ڴ���ͼ�Ŀ���
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

	//�����ַ����İ�װ��
	class virtual_alloctor {
	public:
		/*
		Ԥ����ַ�ռ�����,��ʱ���صĵ�ַ���ɷ���,δ���������ڴ�
		����Ĵ�С����ȡ����ҳ���С��������
		*/
		void* alloc_reserve(size_t size, bool top_down = false, DWORD protect_attr = PAGE_READWRITE)
		{
			DWORD alloc_type = MEM_RESERVE;
			alloc_type = top_down ? (alloc_type | MEM_TOP_DOWN) : alloc_type;
			return VirtualAlloc(nullptr, size, alloc_type, protect_attr);
		}

		/*
		��������洢���ڴ�
		*/
		void* alloc_commit(void* addr, size_t size, DWORD protect_attr = PAGE_READWRITE)
		{
			return VirtualAlloc(addr, size, MEM_COMMIT, protect_attr);
		}

		//Ԥ������������洢���ڴ�
		void* alloc(size_t size, DWORD protect_attr = PAGE_READWRITE)
		{
			return VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, protect_attr);
		}

		//�ı�����ı�������
		bool protect_attr_reset(void* addr, size_t size, DWORD protect_attr_new, PDWORD protect_attr_old)
		{
			return VirtualProtect(addr, size, protect_attr_new, protect_attr_old);
		}

		//�ͷŲ�������洢��
		bool decommit(void* addr, size_t size)
		{
			return VirtualFree(addr, size, MEM_DECOMMIT);
		}

		//�ͷ���������
		bool free(void* addr)
		{
			return VirtualFree(addr, 0, MEM_FREE);
		}

	};

	//�ļ�ӳ��İ�װ��
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

	
	//�Ƚϱ�׼io���ļ�ӳ��Ķ�ȡ�ٶ�
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
			SetFilePointerEx(hFile, offset, nullptr, FILE_BEGIN);	//��ָ���ƶ����ļ��ײ�
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
				������һ������:�����Ҫ��ȡ�ļ����¸�����,��Ҫ���ϵ�map/unmap��?
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

	
	//����ͳ��read_comp�ĳ�Ա�����ĺ�ʱ
	void time_statistics(read_comp* prc,void(read_comp::*func)())
	{
		using namespace std::chrono;
		auto t1 = steady_clock::now();
		(prc->*func)();
		auto t2 = steady_clock::now();
		auto used_time = t2 - t1;
		cout << dec << duration_cast<chrono::milliseconds>(used_time).count() << "ms" << endl;
	}

	//��ȡ�Ƚϲ���
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

	//windows�ѵİ�װ��
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
		windows_heap wh{ HEAP_GENERATE_EXCEPTIONS,2 * 1024 * 1024,4 * 1024 * 1024 };	//����ѿռ�,��ʼ2MB,���4MB,������ʧ���׳��쳣
		void* addr=wh.alloc(HEAP_GENERATE_EXCEPTIONS, 1 * 1024);	//����1KB���ڴ��
		cout << wh.mem_size(0, addr) << endl;
		addr = wh.realloc(HEAP_GENERATE_EXCEPTIONS, addr, 2 * 1024);//��������2KB���ڴ��
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
	
	windows�ڴ����С��:
		1:�����ַ�������ַ����ĳ��ӳ���ϵ,�����ɲ���ϵͳ����
		2:����ɼ��ĵ�ַ���������ַ,�ӳ���ĽǶ�����,�����ַ��"�û�"���򶼿�ʹ��;
		3:�����ַ���û�����ͨ���ɻ���(��)Ϊ"��ջ�ṹ",�����ĳ���Ա�Ƕ�,(�����ڶ�����������,ջ�ӵײ���������,ջ֮�������ݶδ���εȵ�,��֮�����ں˿ռ�);
		4:������Ҫ����һ�������ַ��ʱ��,��Ҫ����ϵͳ��õ�ַ����Ԥ���͵���,ӳ�䵽ʵ�ʵ������ַ,����ᷢ������Υ��
		5:windows���̲�����ʱ��,�ᴴ��Ĭ�϶Ѻ�Ĭ���߳�ջ,�ѿռ���Ĭ��ͬ����,�����Ҫ��Ч��ʹ�öѿռ�,���ǿ������������Լ��Ķѿռ�
		6:�ڴ��ļ�ӳ��Ӷ�ȡ�ĽǶ���˵��ȷ���ڱ�׼io����(ÿ�ν�������������,read_by_file_map��ʱ����С��read_by_std_io),���Գ����ȡ��һ��197MB����Ƶ�ļ�,�����ļ�ӳ�䷽ʽ��
			55-75ms����,��׼io����������(FILE_FLAG_NO_BUFFERing),ÿ�ζ�ȡ��7000ms-8000ms����,��������ĺ����ȡ��200ms-300ms֮��.�������ͬ����׼io��ȡ.
		7:�ع���һ�±�׼io����,�����ڿ�ʼ���ļ�ָ���ʱ��,���˿�...û�н�ָ���û��ļ��ײ�.
*/
