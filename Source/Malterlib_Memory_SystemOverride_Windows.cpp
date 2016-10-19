// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#ifdef DMibConfig_OverrideSystemMalloc

#include <Mib/Core/Core>
#include <windows.h>

#ifdef DArchitecture_x86
#define DDefaultCallingConv __cdecl
#else
#define DDefaultCallingConv 
#endif

#define DMibAllowCodeStandardViolations 1

#undef new
#undef malloc
#undef free
#undef realloc
#undef calloc
#undef memalign
#undef _msize
#undef _expand

#undef _malloc_dbg
#undef _free_dbg
#undef _realloc_dbg
#undef _calloc_dbg
#undef _calloc_dbg_impl
#undef memalign_dbg
#undef _msize_dbg
#undef _expand_dbg
#undef _nh_malloc
#undef _heap_alloc
#undef _recalloc_dbg
#undef _malloc_base
#undef _malloc_dbg
#undef _nh_malloc_dbg
#undef _heap_alloc_dbg

#undef _free_base
#undef _calloc_base
#undef _realloc_base
#undef _msize_base
#undef _expand_base
#undef _nh_malloc_base
#undef _malloc_crt
#undef _calloc_crt
#undef _realloc_crt
#undef _recalloc_crt

#undef _aligned_free
#undef _aligned_malloc
#undef _aligned_offset_malloc
#undef _aligned_realloc
#undef _aligned_recalloc
#undef _aligned_offset_realloc
#undef _aligned_offset_recalloc
#undef _aligned_msize

#undef _aligned_free_dbg
#undef _aligned_malloc_dbg
#undef _aligned_offset_malloc_dbg
#undef _aligned_realloc_dbg
#undef _aligned_recalloc_dbg
#undef _aligned_offset_realloc_dbg
#undef _aligned_offset_recalloc_dbg
#undef _aligned_msize_dbg


#undef _CrtSetBreakAlloc
#undef _CrtSetDbgBlockType
#undef _CrtSetAllocHook
#undef _CrtCheckMemory
#undef _CrtSetDbgFlag
#undef _CrtDoForAllClientObjects
#undef _CrtIsValidPointer
#undef _CrtIsValidHeapPointer
#undef _CrtIsMemoryBlock
#undef _CrtSetDumpClient
#undef _CrtMemCheckpoint
#undef _CrtMemDifference
#undef _CrtMemDumpAllObjectsSince
#undef _CrtDumpMemoryLeaks
#undef _CrtMemDumpStatistics
#undef _CrtReportBlockType
#undef _CrtSetCheckCount
#undef _CrtGetCheckCount

#define DMibSystemAlignment sizeof(void *)*2


extern "C"
{
#if _MSC_VER >= 1400
#define MemDeclNaR __declspec(noalias) __declspec(restrict)
#define MemDeclNa __declspec(noalias)
#else
#define MemDeclNaR 
#define MemDeclNa 
#endif
	size_t __crtDebugFillThreshold = SIZE_MAX;
	
	MemDeclNaR void * DDefaultCallingConv malloc(size_t);
	MemDeclNa void DDefaultCallingConv free(void *);
	MemDeclNaR void * DDefaultCallingConv realloc(void *, size_t);
	MemDeclNaR void * DDefaultCallingConv calloc(size_t, size_t);
	MemDeclNaR void * DDefaultCallingConv memalign(size_t, size_t);
	
	/*************************************************************************************************\
	|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
	| malloc
	|__________________________________________________________________________________________________
	\*************************************************************************************************/

	MemDeclNaR void * DDefaultCallingConv malloc (size_t sz)
	{		
		sz = NMib::fg_AlignUp(sz, DMibSystemAlignment);
		return NMib::NMem::fg_Alloc(sz);
	}

	void * DDefaultCallingConv _malloc_base (size_t sz)
	{		
		sz = NMib::fg_AlignUp(sz, DMibSystemAlignment);
		return NMib::NMem::fg_Alloc(sz);
	}

	void * DDefaultCallingConv _malloc_dbg (size_t sz, int BlockType, const char *Filename, int Line) 
	{
		sz = NMib::fg_AlignUp(sz, DMibSystemAlignment);
		return NMib::NMem::fg_AllocDebug(sz, Filename, Line, (BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
	}
	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| calloc
|__________________________________________________________________________________________________
\*************************************************************************************************/
	
	MemDeclNaR void * DDefaultCallingConv calloc (size_t nelem, size_t elsize)
	{
		mint Size = nelem * elsize;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Alloc(Size);
		memset (addr, 0, nelem * elsize);
		return addr;
	}
	void * DDefaultCallingConv _calloc_dbg (size_t nelem, size_t elsize, int BlockType, const char *Filename, int Line) 
	{
		mint Size = nelem * elsize;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_AllocDebug(Size, Filename, Line, (BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
		memset (addr, 0, nelem * elsize);
		return addr;
	}

	void * __cdecl _calloc_dbg_impl(
		size_t nNum,
		size_t nSize,
		int nBlockUse,
		const char * szFileName,
		int nLine,
		int * errno_tmp
		)
	{
		mint Size = nNum * nSize;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_AllocDebug(Size, szFileName, nLine, (nBlockUse == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
		memset (addr, 0, nNum * nSize);
		return addr;
	}


	void * DDefaultCallingConv _calloc_impl (size_t nelem, size_t elsize, int * errno_tmp)
	{
		mint Size = nelem * elsize;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Alloc(Size);
		memset (addr, 0, nelem * elsize);
		return addr;
	}

	void * DDefaultCallingConv _calloc_base (size_t nelem, size_t elsize)
	{
		mint Size = nelem * elsize;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Alloc(Size);
		memset (addr, 0, nelem * elsize);
		return addr;
	}

	_CRTIMP __checkReturn __bcount_opt(_Size) void * __cdecl _malloc_crt(__in size_t _Size)
	{
		_Size = NMib::fg_AlignUp(_Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Alloc(_Size);
		return addr;
	}

	_CRTIMP void * __cdecl _calloc_crt(size_t count, size_t size)
	{
		mint Size = count * size;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Alloc(Size);
		memset (addr, 0, count * size);
		return addr;
	}
	_CRTIMP __checkReturn __bcount_opt(_Size) void * __cdecl _realloc_crt(__inout_opt void *_Ptr, __in size_t _Size)
	{
		_Size = NMib::fg_AlignUp(_Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Resize(_Ptr, _Size);
		return addr;
	}
	_CRTIMP __checkReturn __bcount_opt(_Size*_Count) void * __cdecl _recalloc_crt(__inout_opt void *_Ptr, __in size_t _Count, __in size_t _Size)
	{
		mint Size = _Count * _Size;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		void * addr = NMib::NMem::fg_Resize(_Ptr, Size);
		return addr;
	}

		
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| free
|__________________________________________________________________________________________________
\*************************************************************************************************/

	MemDeclNa void DDefaultCallingConv free (void * ptr)
	{
		NMib::NMem::fg_Free(ptr);
	}

	void DDefaultCallingConv _free_base (void * ptr)
	{
		NMib::NMem::fg_Free(ptr);
	}

	void DDefaultCallingConv _free_dbg (void * ptr, int) 
	{
		NMib::NMem::fg_Free(ptr);
	}	
	
	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| memalign
|__________________________________________________________________________________________________
\*************************************************************************************************/

	MemDeclNaR void * DDefaultCallingConv memalign (size_t alignment, size_t size)
	{
		return NMib::NMem::fg_AllocAligned(size, alignment);
	}	
	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| realloc
|__________________________________________________________________________________________________
\*************************************************************************************************/

	MemDeclNaR void * DDefaultCallingConv realloc (void * ptr, size_t sz)
	{
		sz = NMib::fg_AlignUp(sz, DMibSystemAlignment);
		return NMib::NMem::fg_Resize(ptr, sz);
	}
	
	void * DDefaultCallingConv _realloc_base (void * ptr, size_t sz)
	{
		sz = NMib::fg_AlignUp(sz, DMibSystemAlignment);
		return NMib::NMem::fg_Resize(ptr, sz);
	}
	
	void * DDefaultCallingConv _realloc_dbg (void * ptr, size_t sz, int BlockType, const char *Filename, int Line) 
	{
		sz = NMib::fg_AlignUp(sz, DMibSystemAlignment);
		return NMib::NMem::fg_ResizeDebug(ptr, sz, Filename, Line, (BlockType == 2 ? NMib::EHeapDebugFlag_Ignore : NMib::EHeapDebugFlag_None));
	}

	MemDeclNaR void * DDefaultCallingConv _recalloc(void * memblock,size_t count,size_t size)
	{
		mint Size = size * count;
		Size = NMib::fg_AlignUp(Size, DMibSystemAlignment);
		return NMib::NMem::fg_Resize(memblock, Size);

	}

//#ifndef DArchX86_64
//	MemDeclNaR
//#endif
		void * DDefaultCallingConv _recalloc_dbg
	(
		void * memblock,
		size_t count,
		size_t size,
		int nBlockUse,
		const char * szFileName,
		int nLine
	)
	{
		size_t  size_orig=0;

		size_orig = size * count;

		return _realloc_dbg(memblock, size_orig, nBlockUse, szFileName, nLine);
	}


	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| _msize
|__________________________________________________________________________________________________
\*************************************************************************************************/
	
	mint DDefaultCallingConv _msize(void *mem)
	{
		return NMib::NMem::fg_Size(mem);
	}

	mint DDefaultCallingConv _msize_base(void *mem)
	{
		return NMib::NMem::fg_Size(mem);
	}

	mint DDefaultCallingConv _msize_dbg (void * mem, int blockType) 
	{
		return NMib::NMem::fg_Size(mem);
	}

/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| _expand
|__________________________________________________________________________________________________
\*************************************************************************************************/
	
	void* DDefaultCallingConv _expand(void *mem , size_t size)
	{
		// Just fail :P
		DMibPDebugBreak;
		return mem;
	}
	
	void* DDefaultCallingConv _expand_base(void *mem , size_t size)
	{
		// Just fail :P
		DMibPDebugBreak;
		return mem;
	}
	
	void* DDefaultCallingConv _expand_dbg(void *userData, mint newSize, int blockType, const char *filename, int linenumber)
	{
		// Just fail :P
		DMibPDebugBreak;
		return userData;
	}
	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| _nh_malloc
|__________________________________________________________________________________________________
\*************************************************************************************************/
	
	_Check_return_  _Ret_opt_bytecap_(_Size) void * __cdecl _nh_malloc(_In_ size_t _Size, _In_ int _NhFlag)
	{
		return malloc (_Size);
	}
	
	void * DDefaultCallingConv _nh_malloc_base (mint nSize,int nhFlag)
	{
		return malloc (nSize);
	}
	
	void * DDefaultCallingConv _nh_malloc_dbg (mint nSize,int nhFlag,int nBlockUse,const char * szFileName,int nLine)
	{
		return _malloc_dbg (nSize, nBlockUse, szFileName, nLine);
	}
	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| _heap_alloc
|__________________________________________________________________________________________________
\*************************************************************************************************/
	
	_Check_return_  _Ret_opt_bytecap_(_Size) void * __cdecl _heap_alloc(_In_ size_t _Size)
	{
		return malloc (_Size);
	}
	
	void * DDefaultCallingConv _heap_alloc_base(mint nSize)
	{
		return malloc (nSize);
	}
	
	void * DDefaultCallingConv _heap_alloc_dbg(mint nSize,int nBlockUse,const char * szFileName,int nLine)
	{
		return _malloc_dbg (nSize, nBlockUse, szFileName, nLine);
	}
	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| _free_lk
|__________________________________________________________________________________________________
\*************************************************************************************************/

	void DDefaultCallingConv _free_lk(void * pUserData)
	{
		free (pUserData);
	}
	
	void DDefaultCallingConv _free_lk_base(void * pUserData)
	{
		free (pUserData);
	}
	
	void DDefaultCallingConv _free_dbg_lk(void * pUserData,int nBlockUse)
	{
		free (pUserData);
	}

	__declspec(noalias) void DDefaultCallingConv _freea_s(void *ptr)
	{
//		DMibPDebugBreak;
		if (ptr != nullptr)
		{
			ptr = (char*)ptr - _ALLOCA_S_MARKER_SIZE;
			if (*((size_t*)ptr) == _ALLOCA_S_HEAP_MARKER)
			{
				free(ptr);
			}
		}
	}
				

	
/*************************************************************************************************\
|¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
| Debug routines
|__________________________________________________________________________________________________
\*************************************************************************************************/
	
	long DDefaultCallingConv _CrtSetBreakAlloc(long lNewBreakAlloc)
	{
        return 0;
	}
	
	void DDefaultCallingConv _CrtSetDbgBlockType(void * pUserData,int nBlockUse)
	{
	}
	
	typedef int (DDefaultCallingConv * _CRT_ALLOC_HOOK)(int, void *, mint, int, long, const unsigned char *, int);
	
	_CRT_ALLOC_HOOK DDefaultCallingConv _CrtSetAllocHook(_CRT_ALLOC_HOOK pfnNewHook)
	{
        return nullptr;
	}
	
	
	int DDefaultCallingConv CheckBytes(unsigned char * pb,unsigned char bCheck,mint nSize)
	{
		// We actually need to do this (not just return 1) since both positive and negative returns are used :-)
		unsigned char *pEnd = pb + nSize;
		while (pb < pEnd)
		{
			if (*pb++ != bCheck)
				return 0;
		}
        return 1;
	}
	
	
	int DDefaultCallingConv _CrtCheckMemory(void)
	{
		return NMib::fg_GetSys()->f_MemoryManager_Check(false);
	}
	
	int DDefaultCallingConv _CrtSetDbgFlag(int fNewBits)
	{
		return _CRTDBG_LEAK_CHECK_DF;
	}
	
	
	void DDefaultCallingConv _CrtDoForAllClientObjects(void (DDefaultCallingConv *  pfn)(void *, void *),void * pContext)
	{
	}
	
	
	int DDefaultCallingConv _CrtIsValidPointer(const void * pv,unsigned int nBytes,int bReadWrite)
	{
        return (pv != nullptr
#ifdef _WIN32
            && !IsBadReadPtr(pv, nBytes) &&
            (!bReadWrite || !IsBadWritePtr((LPVOID)pv, nBytes))
#endif  /* _WIN32 */
            );
	}
	
	int DDefaultCallingConv _CrtIsValidHeapPointer(const void * pUserData)
	{
		return true;
	}
	
	
	int DDefaultCallingConv _CrtIsMemoryBlock(const void * pUserData,unsigned int nBytes,long * plRequestNumber,char ** pszFileName,int * pnLine)
	{
        return false;
	}
		
	
	typedef void (DDefaultCallingConv * _CRT_DUMP_CLIENT)(void *, mint);
	
	_CRT_DUMP_CLIENT DDefaultCallingConv _CrtSetDumpClient(_CRT_DUMP_CLIENT pfnNewDump)
	{
        return nullptr;
	}
	
	void DDefaultCallingConv _CrtMemCheckpoint(_CrtMemState * state)
	{
		
	}
	
	int DDefaultCallingConv _CrtMemDifference(_CrtMemState * state,const _CrtMemState * oldState,const _CrtMemState * newState)
	{
        return false;
	}
	
	
	void DDefaultCallingConv _CrtMemDumpAllObjectsSince(const _CrtMemState * state)
	{
		
	}
	
	int DDefaultCallingConv _CrtDumpMemoryLeaks(void)
	{
		return false;
	}
	
	void DDefaultCallingConv _CrtMemDumpStatistics(const _CrtMemState * state)
	{
	}

	int DDefaultCallingConv _CrtReportBlockType(const void * pUserData)
	{
		return 0;
	}

	int DDefaultCallingConv _CrtSetCheckCount(int fCheckCount)
	{
		return 0;
	}

	_CRTIMP int __cdecl _CrtGetCheckCount(
			void
			)
	{
		return 0;
	}


	int DDefaultCallingConv _heap_init()
	{
		//fg_CTRHackHeapInit();
		return true;
	}

	extern "C" HANDLE __acrt_heap = nullptr;

	void DDefaultCallingConv _heap_term(void)
	{
	}

	int DDefaultCallingConv __heap_select(void)
	{
		DMibPDebugBreak;
		return 0;
	}

	extern "C" bool __cdecl __acrt_initialize_heap()
	{
		return true;
	}

	extern "C" bool __cdecl __acrt_uninitialize_heap(bool const)
	{
		return true;
	}

	extern "C" intptr_t __cdecl _get_heap_handle()
	{
		DMibPDebugBreak;
		return 0;
	}

	extern "C" HANDLE __acrt_getheap()
	{
		DMibPDebugBreak;
		return nullptr;
	}


	void DDefaultCallingConv _heap_abort(void)
	{

	}
#ifdef DMibDebug
	#define nAlignGapSize sizeof(void *)

	typedef struct _AlignMemBlockHdr
	{
		void *pHead;
		unsigned char Gap[nAlignGapSize];
	} _AlignMemBlockHdr;

	#define IS_2_POW_N(d_Value)   (((d_Value)&(d_Value-1)) == 0)

	#define nNoMansLandSize 4

	static unsigned char _bNoMansLandFill = 0xFD;   /* fill no-man's land with this */
	static unsigned char _bAlignLandFill  = 0xED;   /* fill no-man's land for aligned routines */
	static unsigned char _bDeadLandFill   = 0xDD;   /* fill free objects with this */
	static unsigned char _bCleanLandFill  = 0xCD;   /* fill new objects with this */


	void * __cdecl _aligned_malloc(
			size_t size,
			size_t align
			)
	{
		return _aligned_offset_malloc_dbg(size, align, 0, NULL, 0);
	}


	void * __cdecl _aligned_malloc_dbg(
			size_t size,
			size_t align,
			const char * f_name,
			int line_n
			)
	{
		return _aligned_offset_malloc_dbg(size, align, 0, f_name, line_n);
	}

	void * __cdecl _aligned_realloc(
			void * memblock,
			size_t size,
			size_t align
			)
	{
		return _aligned_offset_realloc_dbg(memblock, size, align, 0, NULL, 0);
	}

	void * __cdecl _aligned_realloc_dbg(
			void *memblock,
			size_t size,
			size_t align,
			const char * f_name,
			int line_n
			)
	{
		return _aligned_offset_realloc_dbg(memblock, size, align, 0, f_name, line_n);
	}

	void * __cdecl _aligned_recalloc_dbg(
			void *memblock,
			size_t count,
			size_t size,
			size_t align,
			const char * f_name,
			int line_n
			)
	{
		return _aligned_offset_recalloc_dbg(memblock, count, size, align, 0, f_name, line_n);
	}

	void * __cdecl _aligned_offset_malloc(
			size_t size,
			size_t align,
			size_t offset
			)
	{
		return _aligned_offset_malloc_dbg(size, align, offset, NULL, 0);
	}


	void * __cdecl _aligned_offset_malloc_dbg(
			size_t size,
			size_t align,
			size_t offset,
			const char * f_name,
			int line_n
			)
	{
		uintptr_t ptr, r_ptr, t_ptr;
		_AlignMemBlockHdr *pHdr;
		size_t nonuser_size,block_size;

		/* validation section */
		align = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) -1;

		t_ptr = (0 -offset)&(sizeof(uintptr_t) -1);

		nonuser_size = t_ptr + align + sizeof(_AlignMemBlockHdr); /* cannot overflow */
		block_size = size + nonuser_size;

		if ((ptr = (uintptr_t) _malloc_dbg(block_size, _NORMAL_BLOCK, f_name, line_n)) == (uintptr_t)NULL)
			return NULL;

		r_ptr =((ptr +nonuser_size +offset)&~align)-offset;
		pHdr = (_AlignMemBlockHdr *)(r_ptr - t_ptr) -1;
		memset((void *)pHdr->Gap, _bAlignLandFill, nAlignGapSize);
		pHdr->pHead = (void *)ptr;
		return (void *) r_ptr;
	}

	void * __cdecl _aligned_offset_realloc(
			void * memblock,
			size_t size,
			size_t align,
			size_t offset
			)
	{
		return _aligned_offset_realloc_dbg(memblock, size, align, offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_recalloc(
			void * memblock,
			size_t count,
			size_t size,
			size_t align,
			size_t offset
			)
	{
		return _aligned_offset_recalloc_dbg(memblock, count, size, align, offset, NULL, 0);
	}

	void * __cdecl _aligned_offset_realloc_dbg(
			void * memblock,
			size_t size,
			size_t align,
			size_t offset,
			const char * f_name,
			int line_n
			)
	{
		uintptr_t ptr, r_ptr, t_ptr, mov_sz;
		_AlignMemBlockHdr *pHdr, *s_pHdr;
		size_t nonuser_size, block_size;

		if ( memblock == NULL)
		{
			return _aligned_offset_malloc_dbg(size, align, offset, f_name, line_n);
		}
		if ( size == 0)
		{
			_aligned_free_dbg(memblock);
			return NULL;
		}

		s_pHdr = (_AlignMemBlockHdr *)((uintptr_t)memblock & ~(sizeof(uintptr_t) -1)) -1;

		if ( CheckBytes((unsigned char *)memblock -nNoMansLandSize, _bNoMansLandFill, nNoMansLandSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "The block at 0x%p was not allocated by _aligned routines, use realloc()", memblock);
			errno = 22;
			return NULL;
		}

		if(!CheckBytes(s_pHdr->Gap, _bAlignLandFill, nAlignGapSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", memblock);
		}

		/* validation section */

		mov_sz = _msize(s_pHdr->pHead) - ((uintptr_t)memblock - (uintptr_t)s_pHdr->pHead);

		align = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) -1;

		t_ptr = (0 -offset)&(sizeof(uintptr_t) -1);

		nonuser_size = t_ptr + align + sizeof(_AlignMemBlockHdr); /* cannot overflow */
		block_size = size + nonuser_size;

		if ((ptr = (uintptr_t) _malloc_dbg(block_size, _NORMAL_BLOCK, f_name, line_n)) == (uintptr_t)NULL)
			return NULL;

		r_ptr =((ptr +nonuser_size +offset)&~align)-offset;
		pHdr = (_AlignMemBlockHdr *)(r_ptr - t_ptr) -1;
		memset((void *)pHdr->Gap, _bAlignLandFill, nAlignGapSize);
		pHdr->pHead = (void *)ptr;

		memcpy((void *)r_ptr, memblock, mov_sz > size ? size : mov_sz);
		_free_dbg(s_pHdr->pHead, _NORMAL_BLOCK);

		return (void *) r_ptr;
	}

	void * __cdecl _aligned_offset_recalloc_dbg
	(
		void * memblock,
		size_t count,
		size_t size,
		size_t align,
		size_t offset,
		const char * f_name,
		int line_n
	)
	{
		size_t user_size    = 0; /* wanted size, passed to aligned realoc */
		size_t start_fill   = 0; /* location where aligned recalloc starts to fill with 0 */
								 /* filling must start from the end of the previous user block */
								 /* and must stop at the end of the new user block */
		void * retp = NULL;      /* result of aligned recalloc*/

		/* ensure that (size * count) does not overflow */
		user_size = size * count;

		if (memblock != NULL)
		{
			start_fill = _aligned_msize(memblock, align, offset);
		}

		retp = _aligned_offset_realloc_dbg(memblock, user_size, align, offset, f_name, line_n);

		if (retp != NULL)
		{
			if (start_fill < user_size)
			{
				memset ((char*)retp + start_fill, 0, user_size - start_fill);
			}
		}
		return retp;
	}

	void __cdecl _aligned_free(
			void *memblock
			)
	{
		_aligned_free_dbg(memblock);
	}

	void __cdecl _aligned_free_dbg(
			void * memblock
			)
	{
		_AlignMemBlockHdr *pHdr;

		if ( memblock == NULL)
			return;

		pHdr = (_AlignMemBlockHdr *)((uintptr_t)memblock & ~(sizeof(uintptr_t) -1)) -1;

		if ( CheckBytes((unsigned char *)memblock -nNoMansLandSize, _bNoMansLandFill, nNoMansLandSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "The block at 0x%p was not allocated by _aligned routines, use free()", memblock);
			return;
		}

		if(!CheckBytes(pHdr->Gap, _bAlignLandFill, nAlignGapSize))
		{
			// We don't know where (file, linenum) memblock was allocated
			_RPT1(_CRT_ERROR, "Damage before 0x%p which was allocated by aligned routine\n", memblock);
		}
		_free_dbg((void *)pHdr->pHead, _NORMAL_BLOCK);
	}

	size_t __cdecl _aligned_msize(
			void *memblock,
			size_t align,
			size_t offset
			)
	{
		return _aligned_msize_dbg(memblock, align, offset);
	}

	size_t __cdecl _aligned_msize_dbg(
			void * memblock,
			size_t align,
			size_t offset
			)
	{
		size_t header_size = 0; /* Size of the header block */
		size_t footer_size = 0; /* Size of the footer block */
		size_t total_size  = 0; /* total size of the allocated block */
		size_t user_size   = 0; /* size of the user block*/
		uintptr_t gap      = 0; /* keep the alignment of the data block */
								/* after the sizeof(void*) aligned pointer */
								/* to the beginning of the allocated block */

		/* HEADER SIZE + FOOTER SIZE = GAP + ALIGN + SIZE OF A POINTER*/
		/* HEADER SIZE + USER SIZE + FOOTER SIZE = TOTAL SIZE */

		_AlignMemBlockHdr *pHdr = NULL; /* points to the beginning of the allocated block*/
		pHdr = (_AlignMemBlockHdr *)((uintptr_t)memblock & ~(sizeof(uintptr_t) - 1)) -1;
		total_size = _msize(pHdr->pHead);
		header_size = (uintptr_t) memblock - (uintptr_t) pHdr->pHead;
		gap = (0 - offset) & (sizeof(uintptr_t) - 1);
		/* The align cannot be smaller than the sizeof(uintptr_t) */
		align = (align > sizeof(uintptr_t) ? align : sizeof(uintptr_t)) -1;
		footer_size = gap + align + sizeof(_AlignMemBlockHdr) - header_size;
		user_size = total_size - header_size - footer_size;
		return user_size;
	}
#endif

}

void fg_MalterlibMallocOverride_CanStartThreads()
{
}

#endif
