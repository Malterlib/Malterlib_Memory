// Copyright © 2015 Hansoft AB
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

namespace NMib::NMemory
{
	// Qt does not free some global pools, so we tell the debug mem manager to ignore them here.
	// Nobody should add to this list other than Mike & Erik!
	char const* gc_IgnoreFunctions[] =
		{
#ifdef DPlatformFamily_macOS
			"_xpc_dyld_image_callback", // Possible bug in macOS, investigate later
			"atexit_register",
			"__vfprintf",
			"pthread_join",
			"_pthread_struct_init",
			"__cxa_get_globals",
			"setvbuf",
			"CFLocaleCopyCurrent",
			"qt_init",
			"qt_mac_create_widget",
			"QMacInputContext",
			"qt_mac_create_window",
			"QMenuPrivate::QMacMenuPrivate::addAction",
			"QToolTip::",
			"notify_register_check",
			"dispatch_once_f",
			"getaddrinfo",
			"_NSInitializePlatform",
			"notify_register_tz",
			"getPerThreadBufferFor_dlerror",
			"gmtime",
			"gmtsub",
			"__CFInitialize",
			"ures_getLocale",
			"_objc_init",
			"dispatch_source_create",
			"_libxpc_initializer",
			"+[NSTimeZone systemTimeZone]",
			"strerror",
			"(anonymous namespace)::AutoreleasePoolPage::autoreleaseNoPage(objc_object*)",
			"CFTimeZoneGetSecondsFromGMT",
			"realizeClass(objc_class*)",
			"getElapsedWallTime",
			"cache_t::reallocate",
			"ibm_4758_cca_init",
			"xpc_connection_create_mach_service",
			"_xpc_connection_init",
			"-[OS_xpc_object _xref_dispose]",
			"net_helper_get_connection",
			"_dispatch_mach_kevent_register",
			"SecTrustSettingsCopyCertificates",
			"SecTrustCopyAnchorCertificates",
			"_fetchInitializingClassList",
			"LI_get_thread_info",
			"LI_ils_create",
			"objc_registerClassPair",
			"NSSearchPathForDirectoriesInDomains",
			"+[__NSObserver isAnObserver:]",
			"syslog",
			"objc_sync_enter",
			"_CFMachPortCreateWithPort2",
			"+[NSThread currentThread]",
			"+[NSRunLoop(NSRunLoop) currentRunLoop]",
			"-[NSRunLoop(NSRunLoop) _addPort:forMode:]",
			"FSEventStreamScheduleWithRunLoop",
			"+[NSMachPort port]",
			"+[NSAutoreleasePool allocWithZone:]",
			"CFStringEncodingGetConverter",
			"____CFUUIDCreateWithBytesPrimitive_block_invoke",
			"dlopen",
			"dlclose",
			"xpc_pipe_create",
			"Security::KeychainCore::KeychainImpl::open()",
			"Security::KeychainCore::StorageManager::defaultKeychain()",
			"+[__NSTimeZone __new:cache:]",
			"Security::KeychainCore::DynamicDLDBList::_load()",
			"Security::DLDbListCFPref::searchList()",
			"Security::KeychainCore::ItemImpl::add(Security::KeychainCore::Keychain&)",
			"__CFStringCreateImmutableFunnel3",
			"+[NSFileManager defaultManager]",
			"__CFGetConverter",
			"NMib::NDaemon::CDaemon::CDetails::f_RunAsProgram",
			"CFRunLoopRunSpecific",
			"_dispatch_client_callout",
			"CFRunLoopGetCurrent",
			"-[NSSystemStatusBar _createStatusItemControlInWindow:]",
			"CMMMemMgr::New(unsigned long)",
			"_NSEventThread",
			"_NSPopUpCarbonMenu3",
			"pthread_once",
			// macOS 10.12
			"_dyld_initializer",
			"_libtrace_init",
			"_voucher_activity_debug_channel_init",
			"fosl_filter_createReadPixel",
			"glCreateMallocZone",
			"arc4_init",
			"Security::ModifiedTable::createMutableIndexes()",
			"objc_class::demangledName",
			"Security::AppleDatabase",
			"Security::KeychainCore::",
			"CFUniCharGetBitmapPtrForPlane",
			"CFUniCharGetMappingData",
			"__CFUniCharLoadDecompositionTable",
			"tlv_allocate_and_initialize_for_key",
			"tzsetwall_basic",
			"_CFRuntimeCreateInstance",
			"CGCMSConverterCreate",
			"_objc_rootAlloc",
			"-[__NSPlaceholderArray initWithObjects:count:]",
			"-[_NSXPCConnectionClassCache addClass:]",
			"__NSFontInstanceInfoInitializeMetricsInfo",
			"_NSXPCConnectionClassCache",
			"CGPathMoveToPoint",
			"XTCopyPropertiesForFont",
			"_NSSLMPopUpCarbonMenu3",
			"_dispatch_unote_register_muxed",
			"_dispatch_timer_heap_grow",
			// macOS 10.14
			"fosl_filter_getStringForGraphWithOptions",
			"_CFRunLoopGet0",
			"_dispatch_timer_unote_arm",
			"localtime",
			"realizeClassWithoutSwift",
			"load_categories_nolock",
			"cache_fill",
			"objc_object::rootAutorelease2",
			"_os_activity_create_addr",
			// macOS 13.5
			"libSystem_initializer",
			"load_images",
			"dyld4::APIs::dlsym",
			"map_images",
			"cache_t::insert",
			"setCountLegacyAPIEnabledForThread",
			"dyld4::RuntimeState::_instantiateTLVs",
			"_dispatch_continuation_alloc_from_heap",
			"loadlocale",
			"currentlocale",
			"_CFPreferencesCopyAppValueWithContainerAndConfiguration",
			"__SCGetThreadSpecificData",
			// macOS 14
			"nw::object::vtable::register_def",
			"nw::object::class_def::register_method_def",
			"swift_copyAuxiliaryExecutablePath",
			"nw::object::class_def::setup",
			"initializeNonMetaClass",
			"_CFMachPortCreateWithPort4",
			// macOS 15
			"CFStringGetFileSystemRepresentation",
#elif defined(DPlatformFamily_Linux)
			"qt_load_library_runtime",
			"qt_set_x11_resources",
			"currentThread",
			"createDefaultEngines",
#endif
#ifdef DPlatformFamily_Windows
			"QWindowsIntegration::createPlatformTheme",
#else
			"qt_init",
#endif
			"QMutex::QMutex",
			"QMutexPool::get",
			"QFontEngine::harfbuzzFace",
			"qHBNewFace",
			"HB_OpenTypeShape",
			"QTextureGlyphCache::populate",
			"QFontEngine::QFontEngine",
			"QFontEngine::setGlyphCache",
			"QFontEngineWin::recalcAdvances",
			"QFontEngineMultiWin::loadEngine",
			"QRasterPaintEngine::drawCachedGlyphs",
			"QFontDatabase::load",
			"qShapeItem",
			"QPluginLoader::staticInstances",
			"QThreadData::current",
			"signalSlotLock",
			"QLibraryPrivate::findOrCreate",
			"QFactoryLoader::instance",
			"QProcessPrivate::initializeProcessManager",
			// Qml stuffs
			"QQmlTypePrivate::initEnums",
			"QQmlPrivate::qmlregister",
			"QQmlObjectCreator::createInstance",
			"QQmlPropertyCache::update",
			"QQuickTransitionManager::transition",
			"QQmlContextData::propertyNames",
			"QQmlMetaObject::methodParameterTypes",
			"QQuickTransitionManager::QQuickTransitionManager",
			"QQuickListViewPrivate::newViewItem",
			"QQuickItemViewTransitionableItem::startTransition",
			"QQmlMetaTypeData::propertyCache",
			"QQmlType::SingletonInstanceInfo::init",
			"QQuickItemView::setModel",
			"QQmlTypeLoader::doLoad<PlainLoader>",
			"FxViewItem::transitionNextReposition",
			"QQmlImportsPrivate::importExtension",
			"QQmlImportInstance::resolveType",
			"QQmlMetaType::registerCompositeSingletonType",
			"QQuickWidgetPrivate::createContext",
			"QQmlThreadPrivate::QQmlThreadPrivate",
			"QQmlThreadPrivate::run",
			"QQmlData::createQQmlData",
			"QSGAdaptationBackendData::QSGAdaptationBackendData",
			"QQuickEventPoint::estimatedVelocity",
			"QV4::QQmlQPointer<QObject>::init",
			"QPointer<QQmlEngine>::operator=",
			"TCMallocGuard::TCMallocGuard",
			"GoogleInitializer::GoogleInitializer",
			nullptr
		}
	;

#	if DMibConfig_Memory_Shims_Enable



#	if DMibConfig_Memory_Shims_EnableGlobal
	DMibCompilerMessage("---- Global memory shims enabled");
	constinit NAtomic::TCAtomicAggregate<CGlobalReportMemory *> g_pGlobalMemoryReporter = {0};
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
	DMibCompilerMessage("---- Local memory shims enabled");

	struct CMemoryReportThreadInfo
	{
		CMemoryReportThreadInfo() = default;
		CMemoryReportThreadInfo(CMemoryReportThreadInfo const &_Other)
		{
			DMibLock(_Other.m_Lock);
			m_pReportTo = _Other.m_pReportTo;
			m_ReportDepth = _Other.m_ReportDepth;
		}

		mutable NThread::CMutual m_Lock;
		CReportMemory *m_pReportTo = nullptr;
		mint m_ReportDepth = 0;
	};

	constinit NStorage::TCAggregate
	<
		NThread::TCThreadLocal
		<
			CMemoryReportThreadInfo
			, NMemory::CAllocator_NonTrackedHeap
			, NThread::EThreadLocalFlag_Inherit
		>
		, 10
	> g_MemoryReporter = { DAggregateInit };
#endif

	CReportMemory *fg_ReportMemoryTo(CReportMemory *_pMemoryReporter)
	{
#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed())
			return nullptr;
		auto &Reporter = **g_MemoryReporter;
		CReportMemory *pOld = Reporter.m_pReportTo;

		DMibLock(Reporter.m_Lock);
		Reporter.m_pReportTo = _pMemoryReporter;
		return pOld;
#else
		return nullptr;
#endif
	}

	// There is no concept of stacking a global reporter.
	void fg_ReportMemoryGloballyTo(CGlobalReportMemory *_pMemoryReporter)
	{
#			if DMibConfig_Memory_Shims_EnableGlobal
			g_pGlobalMemoryReporter.f_Exchange(_pMemoryReporter);
#			endif
	}

	void fg_ReportMemoryAlloc
		(
			mint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)

	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - Alloc({}, {}, {}, {}, {}, {}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< _pAllocatorName
				<< (void *)_Address
				<< _RequestedAlignment
				<< _RequestedSize
				<< _ReturnedSize
				<< _nBytesOverhead
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Alloc(_MemoryAllocator, _Address, _RequestedAlignment, _RequestedSize, _ReturnedSize, _nBytesOverhead, _pAllocationInfo);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_Alloc(_MemoryAllocator, Info.m_ReportDepth, _pAllocatorName, _Address, _RequestedAlignment, _RequestedSize, _ReturnedSize, _nBytesOverhead, _pAllocationInfo);
#	endif
	}

	void fg_ReportMemoryResize
		(
			mint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, mint _OldAddress
			, mint _OldSize
			, void const *_pOldAllocationInfo
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - Resize({}, {}, {}, {}, {}, {}, {}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< _pAllocatorName
				<< (void *)_OldAddress
				<< (void *)_Address
				<< _RequestedAlignment
				<< _RequestedSize
				<< _ReturnedSize
				<< _nBytesOverhead
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Resize(_MemoryAllocator, _OldAddress, _OldSize, _pOldAllocationInfo, _Address, _RequestedAlignment, _RequestedSize, _ReturnedSize, _nBytesOverhead, _pAllocationInfo);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal

		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_Resize(_MemoryAllocator, Info.m_ReportDepth, _pAllocatorName, _OldAddress, _OldSize, _pOldAllocationInfo, _Address, _RequestedAlignment, _RequestedSize, _ReturnedSize, _nBytesOverhead, _pAllocationInfo);
#endif
	}

	void fg_ReportMemoryRealloc
		(
			mint _MemoryAllocator
			, ch8 const *_pAllocatorName
			, mint _OldAddress
			, mint _OldSize
			, void const *_pOldAllocationInfo
			, mint _Address
			, mint _RequestedAlignment
			, mint _RequestedSize
			, mint _ReturnedSize
			, fp32 _nBytesOverhead
			, void *_pAllocationInfo
		)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - Realloc({}, {}, {}, {}, {}, {}, {}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< _pAllocatorName
				<< (void *)_OldAddress
				<< (void *)_Address
				<< _RequestedAlignment
				<< _RequestedSize
				<< _ReturnedSize
				<< _nBytesOverhead
			)
		;
#	endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Realloc(_MemoryAllocator, _OldAddress, _OldSize, _pOldAllocationInfo, _Address, _RequestedAlignment, _RequestedSize, _ReturnedSize, _nBytesOverhead, _pAllocationInfo);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_Realloc(_MemoryAllocator, Info.m_ReportDepth, _pAllocatorName, _OldAddress, _OldSize, _pOldAllocationInfo, _Address, _RequestedAlignment, _RequestedSize, _ReturnedSize, _nBytesOverhead, _pAllocationInfo);
#	endif
	}

	void fg_ReportMemoryFree(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, void const *_pAllocationInfo)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - Free({}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< (void *)_Address
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Free(_MemoryAllocator, _Address, _Size, _pAllocationInfo);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_Free(_MemoryAllocator, _pAllocatorName, Info.m_ReportDepth, _Address, _Size, _pAllocationInfo);
#endif
	}

	void fg_ReportMemoryGetSize(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size, void const *_pAllocationInfo)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - GetSize({}, {}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< (void *)_Address
				<< _Size
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_GetSize(_MemoryAllocator, _Address, _Size, _pAllocationInfo);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_GetSize(_MemoryAllocator, _pAllocatorName, Info.m_ReportDepth, _Address, _Size, _pAllocationInfo);
#endif
	}

	void fg_ReportMemoryCommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - Commit({}, {}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< (void *)_Address
				<< _Size
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Commit(_MemoryAllocator, _Address, _Size);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_Commit(_MemoryAllocator, _pAllocatorName, Info.m_ReportDepth, _Address, _Size);
#endif
	}

	void fg_ReportMemoryDecommit(mint _MemoryAllocator, ch8 const *_pAllocatorName, mint _Address, mint _Size)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - Decommit({}, {}, {})" DMibNewLine
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< (void *)_Address
				<< _Size
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Decommit(_MemoryAllocator, _Address, _Size);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_Decommit(_MemoryAllocator, _pAllocatorName, Info.m_ReportDepth, _Address, _Size);
#endif
	}

	void fg_ReportMemoryAllocatorDelete(mint _MemoryAllocator, ch8 const *_pAllocatorName)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - AllocatorDelete({}){\n}"
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_AllocatorDelete(_MemoryAllocator);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;
		CReportMemory *pReportTo = Info.m_pReportTo;
		if (pReportTo)
			pReportTo->f_AllocatorDelete(_MemoryAllocator, _pAllocatorName, Info.m_ReportDepth);
#endif
	}


	void fg_ReportMemoryAllocatorName(mint _MemoryAllocator, ch8 const* _pAllocatorName)
	{
#if DEnableMemoryTrace
		DMibTraceSafe
			(
				"{} - AllocatorName({}, {})\r\n"
				, !g_MemoryReporter.f_WasDestructed()
				<< (void *)_MemoryAllocator
				<< _pAllocatorName
			)
		;
#endif
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_AllocatorName(_MemoryAllocator, _pAllocatorName);
#	endif
	}

	void fg_ReportMemoryGoingToReportEnter(mint _MemoryAllocator)
	{
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_ScopeEnter(_MemoryAllocator);
#	endif

#	if DMibConfig_Memory_Shims_EnableLocal
		if (g_MemoryReporter.f_WasDestructed() || !g_MemoryReporter.f_IsConstructed() || fg_GetSys()->f_ThreadDestroyed())
			return;
		CMemoryReportThreadInfo &Info = **g_MemoryReporter;

		DMibLock(Info.m_Lock);
		++Info.m_ReportDepth;
#endif
	}

	void fg_ReportMemoryGoingToReportExit(mint _MemoryAllocator)
	{
#		if DMibConfig_Memory_Shims_EnableLocal
		if (!g_MemoryReporter.f_WasDestructed() && g_MemoryReporter.f_IsConstructed() && !fg_GetSys()->f_ThreadDestroyed())
		{
			CMemoryReportThreadInfo &Info = **g_MemoryReporter;
			DMibLock(Info.m_Lock);
			--Info.m_ReportDepth;
		}
#		endif

#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_ScopeExit(_MemoryAllocator);
#	endif
	}


	void fg_ReportMemoryReportStatistics(bool _bFullReport)
	{
#	if DMibConfig_Memory_Shims_EnableGlobal
		if (g_pGlobalMemoryReporter)
			g_pGlobalMemoryReporter.f_Load()->f_Report(_bFullReport);
#	endif		
	}

#	endif
}
