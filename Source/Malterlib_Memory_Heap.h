// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#include <Mib/Core/Core>

#include "Malterlib_Memory_Heap_Misc.h"
#include "Malterlib_Memory_Heap_THeap.h"
#include "Malterlib_Memory_Heap_Combined.h"
#include "Malterlib_Memory_Heap_Debug.h"
#include "Malterlib_Memory_Heap_StandAlone.h"

#include "Malterlib_Memory_Heap.imp.h"
#include "Malterlib_Memory_Heap_THeap.imp.h"

template <typename t_CHeapParams>
only_parameters_aliased return_not_aliased void * operator new(mint _Size, NMib::NMem::TCHeap_StandAlone<t_CHeapParams> &_Heap)
{
	return _Heap.f_Alloc(_Size);
}

template <typename t_CHeapParams>
only_parameters_aliased void operator delete(void *_pPtr, NMib::NMem::TCHeap_StandAlone<t_CHeapParams> &_Heap) noexcept
{
	return _Heap.f_Free(_pPtr);
}

template <typename t_CHeapParams, typename t_CObject>
only_parameters_aliased void fg_Delete(t_CObject *_pObject, NMib::NMem::TCHeap_StandAlone<t_CHeapParams> &_Heap)
{
	_pObject->~t_CObject();

	_Heap.f_Free(_pObject);;
}

