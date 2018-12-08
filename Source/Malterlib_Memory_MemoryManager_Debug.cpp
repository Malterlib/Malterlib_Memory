// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	extern char const* gc_IgnoreFunctions[];
}

namespace NMib::NMemory::NPrivate
{
	void fg_ReportLeak(uint8 *_pMemory, mint _Size, CMibCodeAddress* _pStackTrace, mint _nStackTrace, ch8 const *_pFile, uint32 _Line, EHeapDebugFlag _Flags, mint _ThreadID, bool _bCanAllocateNonTracked)
	{
		if (_Flags & EHeapDebugFlag_Ignore)
			return;
		// Do this now, rather than when reporting the stacktrace, so we can see if we should ignore this report.
		mint LongestSource = 0;
		mint LongestModule = 0;
		mint LongestFunction = 0;
		mint nStackTrace = fg_Min(_nStackTrace, mint(256));

		CStackTraceInfo StackTraceInfos[256] = {0};

		auto StackTraceCleanup
			= fg_OnScopeExit
			(
				[&]
				{
					for (mint i = 0; i < nStackTrace; ++i)
					{
						if (StackTraceInfos[i].m_pContext)
							NSys::fg_Debug_ReleaseStackTraceInfo(&StackTraceInfos[i]);
					}
				}
			)
		;

		auto fl_SkipFunction
			= [&](ch8 const *_pFunctionName) -> bool
			{
				if (!_pFunctionName)
					return false;
				return
					NMib::NStr::fg_StrStartsWith(_pFunctionName, "NMib::NMemory::TCMemoryManager")
					|| NMib::NStr::fg_StrStartsWith(_pFunctionName, "NMib::NMemory::CCrossModuleImplementation")
					|| NMib::NStr::fg_StrStartsWith(_pFunctionName, "NMib::NSys::fg_System_GetStackTrace")
				;
			}
		;

		{
			bool bCanSkip = true;
			for (mint i = 0; i < nStackTrace; ++i)
			{
				auto &TraceInfo = StackTraceInfos[i];
				if (NSys::fg_Debug_AquireStackTraceInfo(TraceInfo, _pStackTrace[i], _bCanAllocateNonTracked))
				{

					if (bCanSkip)
					{
						if (fl_SkipFunction(TraceInfo.m_pFunctionName))
							continue;
						bCanSkip = false;
					}
					if (TraceInfo.m_pFunctionName)
					{
#ifdef DPlatformFamily_OSX
						if (NStr::fg_StrMatchWildcard(TraceInfo.m_pFunctionName, "+[* load]") == NStr::EMatchWildcardResult_WholeStringMatchedAndPatternExhausted)
							return;
#endif

						for (char const** pCurIgnoreFunction = gc_IgnoreFunctions; *pCurIgnoreFunction; ++pCurIgnoreFunction)
						{
							if (NStr::fg_StrStartsWith(TraceInfo.m_pFunctionName, *pCurIgnoreFunction))
								return; // Ignore this report.
						}
					}

					if (TraceInfo.m_pSourceFileName)
					{
						uint32 Len = NStr::fg_StrLen(TraceInfo.m_pSourceFileName);
						if (Len > LongestSource)
							LongestSource = Len;
					}
					if (TraceInfo.m_pModuleName)
					{
						uint32 Len = NStr::fg_StrLen(TraceInfo.m_pModuleName);
						if (Len > LongestModule)
							LongestModule = Len;
					}
					if (TraceInfo.m_pFunctionName)
					{
						uint32 Len = NStr::fg_StrLen(TraceInfo.m_pFunctionName);
						if (Len > LongestFunction)
							LongestFunction = Len;
					}

				}

			}
		}

		const ch8 *pFile = _pFile;
		if (!pFile)
			pFile = "";

		void *pCurrentBlock = _pMemory;
		typedef NStr::CFStr1024 CFStr;
		CFStr TempStr;
		TempStr = CFStr::CFormat(DMibPFileLineFormat " ") << pFile << _Line;

		mint BlockSize = _Size;
		TempStr += (CFStr::CFormat("Memory Leak, Flags = 0x{sj*4,sf0,nh}, Address = 0x{sj*4,sf0,nh}, Size = 0x{sj*4,sf0,nh}, Thread = {}{\n}")
			<< _Flags
			<< (mint)pCurrentBlock
			<< BlockSize
			<< _ThreadID
			<< sizeof(void *)*2
			).f_GetStr();

		NSys::fg_DebugOutput(TempStr.f_GetStr());
		uint32 nBytes = fg_Min(BlockSize, (mint)128);
		NSys::fg_DebugOutput((CFStr::CFormat("Displaying first {} bytes of block{\n}") << nBytes).f_GetStr().f_GetStr());

		CFStr TraceString;
		CFStr TraceString2;
		for (uint32 i = 0; i < nBytes; ++i)
		{
			TraceString += (CFStr::CFormat("{sj2,sf0,nh}") << ((uint8 *)pCurrentBlock)[i]).f_GetStr();
			if (((uint8 *)pCurrentBlock)[i] > 31)
				TraceString2.f_AddChar(((uint8 *)pCurrentBlock)[i]);
			else
				TraceString2.f_AddChar('$');
		}

		TraceString += DMibNewLine;
		TraceString2 += DMibNewLine;
		NSys::fg_DebugOutput(TraceString.f_GetStr());
		NSys::fg_DebugOutput(TraceString2.f_GetStr());

		NSys::fg_DebugOutput("StackTrace:" DMibNewLine);

		LongestFunction = fg_Min(LongestFunction, 100u);

		bool bCanSkip = true;
		mint nSkipped = 0;
		for (mint i = 0; i < nStackTrace; ++i)
		{
			CStackTraceInfo &TraceInfo = StackTraceInfos[i];
			CMibCodeAddress pAddress = _pStackTrace[i];
			if (TraceInfo.m_pContext)
			{
				if (bCanSkip)
				{
					if (fl_SkipFunction(TraceInfo.m_pFunctionName))
					{
						++nSkipped;
						continue;
					}
					bCanSkip = false;
					if (nSkipped)
						NSys::fg_DebugOutput((CFStr::CFormat("Skipped {} stack frames{\n}") << nSkipped).f_GetStr().f_GetStr());
				}
			}

			CFStr TempStr;
			TempStr
				= CFStr::CFormat("{sj*,a-}   {sl*,a-}   {sj*,a-}   0x{}{\n}")
				<<
				(
					CFStr::CFormat(DMibPFileLineFormat)
					<< (TraceInfo.m_pSourceFileName ? TraceInfo.m_pSourceFileName : "")
					<< TraceInfo.m_SourceLine
				).f_GetStr()
				<< (LongestSource + 5)
				<< (TraceInfo.m_pFunctionName ? TraceInfo.m_pFunctionName : "")
				<< (LongestFunction)
				<< (TraceInfo.m_pModuleName ? TraceInfo.m_pModuleName : "")
				<< (LongestModule)
				<< pAddress
			;

			NSys::fg_DebugOutput(TempStr.f_GetStr());
		}
		NSys::fg_DebugOutput(DMibNewLine);
	}
}
