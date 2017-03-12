// Copyright © 2015 Hansoft AB 
// Distributed under the MIT license, see license text in LICENSE.Malterlib

#pragma once

#include <Mib/Core/Core>
#include "../../Core/Source/Malterlib_Core_General.h"
#include <Mib/Storage/Tuple>
#include <Mib/Meta/Meta>

namespace NMib
{

	namespace NPrivate
	{
		template <typename t_CTypeExplicit, typename t_CTypeImplicit>
		struct TCChooseCreateType
		{
			typedef typename NMib::TCChooseType<NMib::NTraits::TCIsVoid<t_CTypeExplicit>::mc_Value, t_CTypeImplicit, t_CTypeExplicit>::CType CType;
		};
	}

	template <typename tf_ObjectType, typename tf_CAllocator, typename... tfp_CParams>
	tf_ObjectType *fg_ConstructObject(tf_CAllocator &&_Allocator, tfp_CParams&&... p_Params)
	{
		static_assert(sizeof(tf_ObjectType) > 0, "");
		static_assert(!NTraits::TCIsAbstract<tf_ObjectType>::mc_Value || NTraits::TCHasVirtualDestructor<tf_ObjectType>::mc_Value, "");
		mint Size = sizeof(tf_ObjectType);
		auto Memory = fg_Forward<tf_CAllocator>(_Allocator).f_AllocSafe(Size, NTraits::TCAlignmentOf<tf_ObjectType>::mc_Value);
		auto pReturn = new(Memory.f_Get()) tf_ObjectType(fg_Forward<tfp_CParams>(p_Params)...);
		Memory.f_Claim();
		return pReturn;
	}

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObject(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject)
	{
		static_assert(sizeof(tf_CObjectType) > 0, "");
		static_assert(!NTraits::TCIsAbstract<tf_CObjectType>::mc_Value || NTraits::TCHasVirtualDestructor<tf_CObjectType>::mc_Value, "");
		_pObject->~tf_CObjectType();
		fg_Forward<tf_CAllocator>(_Allocator).f_Free(_pObject);
	}

	template <typename t_CType, typename t_CAllocator>
	inline_small void fg_Delete(t_CType * &_pToDelete)
	{
		if (_pToDelete)
		{
			fg_DeleteObject(t_CAllocator(), _pToDelete);
			_pToDelete = nullptr;
		}
	}

	template <typename t_CType>
	inline_small void fg_Delete(t_CType * &_pToDelete)
	{
		static_assert(!NTraits::TCIsAbstract<t_CType>::mc_Value || NTraits::TCHasVirtualDestructor<t_CType>::mc_Value, "");
		if (_pToDelete)
		{
			delete _pToDelete;
			_pToDelete = nullptr;
		}
	}

	
	template <typename t_CType = void, typename... tp_CParams>
	class TCConstruct
	{
		template <typename tf_CType, typename tf_CAllocator, mint... tp_Indices>
		typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType *fp_Create(tf_CAllocator &&_Allocator, NMeta::TCIndices<tp_Indices...> const& _IndexSequnce)
		{
			return fg_ConstructObject<typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType>
				(
					fg_Forward<tf_CAllocator>(_Allocator)
					, fg_Forward<tp_CParams>(NContainer::fg_Get<tp_Indices>(m_Params))...
				);
		}
		
	public:
		enum
		{
			mc_nParams = sizeof...(tp_CParams)
		};
		
		NContainer::TCTuple<typename NMib::NTraits::TCAddLValueReference<tp_CParams>::CType...> m_Params;
		
		TCConstruct(typename NMib::NTraits::TCAddLValueReference<tp_CParams>::CType... p_Params)
			: m_Params(p_Params...)
		{
		}
		
		template <typename tf_CType, typename tf_CAllocator>
		typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType *f_Create(tf_CAllocator &&_Allocator)
		{
			return fp_Create<tf_CType>(fg_Forward<tf_CAllocator>(_Allocator), typename NMeta::TCMakeConsecutiveIndices<mc_nParams>::CType());
		}

	};

	template 
	<
		template <typename t_COldType> class tf_TCTransformConstruct
		, typename tf_CType
		, typename... tfp_CParams
	>
	TCConstruct<typename tf_TCTransformConstruct<tf_CType>::CType, tfp_CParams...> &&
	fg_TransformConstruct(TCConstruct<tf_CType, tfp_CParams...> &&_In)
	{
		return fg_Move((TCConstruct<typename tf_TCTransformConstruct<tf_CType>::CType, tfp_CParams...> &)_In);
	}

	template 
	<
		typename tf_CDefaultType
		, typename tf_CType
		, typename... tfp_CParams
	>
	TCConstruct<tf_CType, tfp_CParams...> &&fg_MakeConcreteConstruct(TCConstruct<tf_CType, tfp_CParams...> &&_In)
	{
		return fg_Move(_In);
	}

	template 
	<
		typename tf_CDefaultType
		, typename... tfp_CParams
	>
	TCConstruct<tf_CDefaultType, tfp_CParams...> &&fg_MakeConcreteConstruct(TCConstruct<void, tfp_CParams...> &&_In)
	{
		return fg_Move((TCConstruct<tf_CDefaultType, tfp_CParams...> &)_In);
	}
		
	template <typename tf_CType, typename... tfp_CParams>
	inline_small TCConstruct<tf_CType, tfp_CParams...> fg_Construct(tfp_CParams &&... p_Params)
	{
		return TCConstruct<tf_CType, tfp_CParams...>((typename NMib::NTraits::TCAddLValueReference<tfp_CParams>::CType)p_Params...);
	}
	template <typename... tfp_CParams>
	inline_small TCConstruct<void, tfp_CParams...> fg_Construct(tfp_CParams &&... p_Params)
	{
		return TCConstruct<void, tfp_CParams...>((typename NMib::NTraits::TCAddLValueReference<tfp_CParams>::CType)p_Params...);
	}
}
