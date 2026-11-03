// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

namespace NMib
{
	// The linker proves that every class constructed here has a deleting
	// destructor that returns the size fg_DeleteObject frees with.
	template <typename tf_CObjectType, typename tf_CAllocator, typename ...tfp_CParams>
	DMibSizedConstruction tf_CObjectType *fg_ConstructObject(tf_CAllocator &&_Allocator, tfp_CParams && ...p_Params)
	{
		static_assert(sizeof(tf_CObjectType) > 0);
		static_assert(!NTraits::cIsAbstract<tf_CObjectType> || NTraits::cHasVirtualDestructor<tf_CObjectType>);

#if !(defined(DPlatformFamily_Windows) && defined(DMibSanitizerEnabled_Address))
		if constexpr (NTraits::TCRemoveReference<tf_CAllocator>::mc_bIsDefault)
			return new tf_CObjectType(fg_Forward<tfp_CParams>(p_Params)...);
		else
#endif
		{
			umint Size = sizeof(tf_CObjectType);
			auto Memory = fg_Forward<tf_CAllocator>(_Allocator).f_AllocSafe(Size, alignof(tf_CObjectType));
			auto pReturn = new(Memory.f_Get()) tf_CObjectType(fg_Forward<tfp_CParams>(p_Params)...);
			Memory.f_Claim();
			return pReturn;
		}
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObject(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject)
	{
		static_assert(sizeof(tf_CObjectType) > 0);
		static_assert(!NTraits::cIsAbstract<tf_CObjectType> || NTraits::cHasVirtualDestructor<tf_CObjectType> || NTraits::cIsFinal<tf_CObjectType>);

#if !(defined(DPlatformFamily_Windows) && defined(DMibSanitizerEnabled_Address))
		if constexpr (NTraits::TCRemoveReference<tf_CAllocator>::mc_bIsDefault && NTraits::cHasOperatorDelete<tf_CObjectType>)
			delete _pObject;
		else
#endif
		{
			if constexpr (NTraits::cHasVirtualDestructor<tf_CObjectType> && !NTraits::cIsFinal<tf_CObjectType>)
			{
				static_assert(!NTraits::cHasOperatorDelete<tf_CObjectType>);
#ifdef DMibPSizedDestructors
				// TCHasVirtualDestructorOverride speaks for a type that is incomplete where it is used; here the type is complete
				static_assert(NTraits::cHasSizedDestructor<tf_CObjectType>, "TCHasVirtualDestructorOverride claims a virtual destructor the type does not have");

				DMibFastCheck(_pObject);

				// The deleting destructor of the dynamic type destroys the object
				// and returns the memory it occupies, so the size comes from a
				// constant in its code rather than from the object itself.
				void *pMemory;
				umint Size = __builtin_malterlib_destroy(_pObject, &pMemory);

				fg_Forward<tf_CAllocator>(_Allocator).f_Free(pMemory, Size);
#else
				if constexpr (NMib::NPrivate::cHas_m_VirtualAllocSize<tf_CObjectType>)
				{
					umint DeleteSize = _pObject->m_VirtualAllocSize;
					_pObject->~tf_CObjectType();

					fg_Forward<tf_CAllocator>(_Allocator).f_Free(_pObject, DeleteSize);
				}
				else
				{
#	if defined(DMibPOverrideOperatorNew)
					NMemory::CCaptureDefaultDelete Captured;
					delete _pObject;

					DMibFastCheck(Captured.m_Captured.m_pMemory);

					if (Captured.m_Captured.m_Size)
						fg_Forward<tf_CAllocator>(_Allocator).f_Free(Captured.m_Captured.m_pMemory, Captured.m_Captured.m_Size);
					else
						fg_Forward<tf_CAllocator>(_Allocator).f_FreeNoSize(Captured.m_Captured.m_pMemory);
#	else
					_pObject->~tf_CObjectType();
					fg_Forward<tf_CAllocator>(_Allocator).f_FreeNoSize(_pObject);
#	endif
				}
#endif
			}
			else
			{
				_pObject->~tf_CObjectType();
				fg_Forward<tf_CAllocator>(_Allocator).f_Free(_pObject, sizeof(tf_CObjectType));
			}
		}
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObjectDefiniteType(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject)
	{
		static_assert(sizeof(tf_CObjectType) > 0);
		static_assert(!NTraits::cIsAbstract<tf_CObjectType> || NTraits::cHasVirtualDestructor<tf_CObjectType>);

		if constexpr (NTraits::TCRemoveReference<tf_CAllocator>::mc_bIsDefault && NTraits::cHasOperatorDelete<tf_CObjectType>)
			delete _pObject;
		else
		{
			static_assert(!NTraits::cHasOperatorDelete<tf_CObjectType>);
			_pObject->~tf_CObjectType();
			fg_Forward<tf_CAllocator>(_Allocator).f_Free(_pObject, sizeof(tf_CObjectType));
		}
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObjectDefiniteType(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject, umint _Alignment)
	{
		static_assert(sizeof(tf_CObjectType) > 0);
		static_assert(!NTraits::cIsAbstract<tf_CObjectType> || NTraits::cHasVirtualDestructor<tf_CObjectType>);

		if constexpr (NTraits::TCRemoveReference<tf_CAllocator>::mc_bIsDefault && NTraits::cHasOperatorDelete<tf_CObjectType>)
			delete _pObject;
		else
		{
			static_assert(!NTraits::cHasOperatorDelete<tf_CObjectType>);
			_pObject->~tf_CObjectType();
			fg_Forward<tf_CAllocator>(_Allocator).f_Free(_pObject, fg_AlignUp(sizeof(tf_CObjectType), _Alignment));
		}
	}

	template <typename tf_CType, typename tf_CAllocator>
	inline_small void fg_Delete(tf_CType * &_pToDelete)
	{
		if (_pToDelete)
		{
			fg_DeleteObject(tf_CAllocator(), _pToDelete);
			_pToDelete = nullptr;
		}
	}

	template <typename t_CType, typename ...tp_CParams>
	TCConstruct<t_CType, tp_CParams...>::TCConstruct(NMib::NTraits::TCAddLValueReference<tp_CParams> ...p_Params)
		: m_Params(p_Params...)
	{
	}

	template <typename t_CType, typename ...tp_CParams>
	template <typename tf_CType, typename tf_CAllocator>
	typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType *TCConstruct<t_CType, tp_CParams...>::f_Create(tf_CAllocator &&_Allocator)
	{
		return fp_Create<tf_CType>(fg_Forward<tf_CAllocator>(_Allocator), NMeta::TCConsecutiveIndices<mc_nParams>());
	}

	template <typename t_CType, typename ...tp_CParams>
	template <typename tf_CType, typename tf_CAllocator, umint ...tfp_Indices>
	auto TCConstruct<t_CType, tp_CParams...>::fp_Create(tf_CAllocator &&_Allocator, NMeta::TCIndices<tfp_Indices...> const &_IndexSequnce)
		-> typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType *
	{
		return fg_ConstructObject<typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType>
			(
				fg_Forward<tf_CAllocator>(_Allocator)
				, fg_Forward<tp_CParams>(fg_Get<tfp_Indices>(m_Params))...
			)
		;
	}

	template <template <typename t_COldType> class tf_TCTransformConstruct, typename tf_CType, typename ...tfp_CParams>
	TCConstruct<typename tf_TCTransformConstruct<tf_CType>::CType, tfp_CParams...> &&fg_TransformConstruct(TCConstruct<tf_CType, tfp_CParams...> &&_In)
	{
		return fg_Move((TCConstruct<typename tf_TCTransformConstruct<tf_CType>::CType, tfp_CParams...> &)_In);
	}

	template <typename tf_CDefaultType, typename tf_CType, typename ...tfp_CParams>
	TCConstruct<tf_CType, tfp_CParams...> &&fg_MakeConcreteConstruct(TCConstruct<tf_CType, tfp_CParams...> &&_In)
	{
		return fg_Move(_In);
	}

	template <typename tf_CDefaultType, typename ...tfp_CParams>
	TCConstruct<tf_CDefaultType, tfp_CParams...> &&fg_MakeConcreteConstruct(TCConstruct<void, tfp_CParams...> &&_In)
	{
		return fg_Move((TCConstruct<tf_CDefaultType, tfp_CParams...> &)_In);
	}

	template <typename tf_CType, typename ...tfp_CParams>
	inline_small TCConstruct<tf_CType, tfp_CParams...> fg_Construct(tfp_CParams && ...p_Params)
	{
		return TCConstruct<tf_CType, tfp_CParams...>((NMib::NTraits::TCAddLValueReference<tfp_CParams>)p_Params...);
	}

	template <typename ...tfp_CParams>
	inline_small TCConstruct<void, tfp_CParams...> fg_Construct(tfp_CParams && ...p_Params)
	{
		return TCConstruct<void, tfp_CParams...>((NMib::NTraits::TCAddLValueReference<tfp_CParams>)p_Params...);
	}
}
