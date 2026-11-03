// Copyright © Unbroken AB
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#pragma once

#include <Mib/Core/Core>
#include "../../Core/Source/Malterlib_Core_General.h"
#include <Mib/Storage/Tuple>
#include <Mib/Meta/Meta>
#include "Malterlib_Memory_CaptureDefaultDelete.h"

namespace NMib
{
	namespace NPrivate
	{
		template <typename t_CTypeExplicit, typename t_CTypeImplicit>
		struct TCChooseCreateType
		{
			using CType = NMib::TCConditional<NMib::NTraits::cIsVoid<t_CTypeExplicit>, t_CTypeImplicit, t_CTypeExplicit>;
		};

#ifndef DMibPSizedDestructors
		template <typename t_CData>
		concept cHas_m_VirtualAllocSize = requires (t_CData const *_pData)
			{
				_pData->m_VirtualAllocSize;
			}
		;
#endif
	}

	template <typename tf_CObjectType, typename tf_CAllocator, typename ...tfp_CParams>
	DMibSizedConstruction tf_CObjectType *fg_ConstructObject(tf_CAllocator &&_Allocator, tfp_CParams && ...p_Params);

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObject(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject);

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObjectDefiniteType(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject);

	template <typename tf_CObjectType, typename tf_CAllocator>
	void fg_DeleteObjectDefiniteType(tf_CAllocator &&_Allocator, tf_CObjectType *_pObject, umint _Alignment);

	template <typename tf_CType, typename tf_CAllocator>
	inline_small void fg_Delete(tf_CType * &_pToDelete);

	template <typename tf_CType>
	inline_small void fg_Delete(tf_CType * &_pToDelete);

	template <typename t_CType = void, typename ...tp_CParams>
	struct TCConstruct
	{
		static constexpr aint mc_nParams = sizeof...(tp_CParams);

		NStorage::TCTuple<NMib::NTraits::TCAddLValueReference<tp_CParams>...> m_Params;

		TCConstruct(NMib::NTraits::TCAddLValueReference<tp_CParams> ...p_Params);

		template <typename tf_CType, typename tf_CAllocator>
		typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType *f_Create(tf_CAllocator &&_Allocator);

	private:
		template <typename tf_CType, typename tf_CAllocator, umint ...tfp_Indices>
		typename NMib::NPrivate::TCChooseCreateType<t_CType, tf_CType>::CType *fp_Create(tf_CAllocator &&_Allocator, NMeta::TCIndices<tfp_Indices...> const &_IndexSequnce);
	};

	template <template <typename t_COldType> class tf_TCTransformConstruct, typename tf_CType, typename ...tfp_CParams>
	TCConstruct<typename tf_TCTransformConstruct<tf_CType>::CType, tfp_CParams...> &&fg_TransformConstruct(TCConstruct<tf_CType, tfp_CParams...> &&_In);

	template <typename tf_CDefaultType, typename tf_CType, typename ...tfp_CParams>
	TCConstruct<tf_CType, tfp_CParams...> &&fg_MakeConcreteConstruct(TCConstruct<tf_CType, tfp_CParams...> &&_In);

	template <typename tf_CDefaultType, typename ...tfp_CParams>
	TCConstruct<tf_CDefaultType, tfp_CParams...> &&fg_MakeConcreteConstruct(TCConstruct<void, tfp_CParams...> &&_In);

	template <typename tf_CType, typename ...tfp_CParams>
	inline_small TCConstruct<tf_CType, tfp_CParams...> fg_Construct(tfp_CParams && ...p_Params);

	template <typename ...tfp_CParams>
	inline_small TCConstruct<void, tfp_CParams...> fg_Construct(tfp_CParams && ...p_Params);
}

#include "Malterlib_Memory_Construct.hpp"
