/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#pragma once

#include <type_traits>
namespace traits
{
	template <typename _Ty>
	constexpr bool is_integral_v = std::is_integral_v<_Ty>;

	template <typename _Ty>
	constexpr bool is_enum_v = std::is_enum_v<_Ty>;

	template <typename _Ty>
	constexpr bool is_pod_struct_v = std::is_class_v<_Ty> && std::is_pod_v<_Ty>;

	template <typename _Ty>
	constexpr bool is_unspecified_v = !is_pod_struct_v<_Ty> && !is_enum_v<_Ty> && !is_integral_v<_Ty>;

	template <bool _Test, typename _Ty = void>
	using enable_if_t = std::enable_if_t<_Test, _Ty>;

	template <typename _Ty>
	using underlying_type_t = std::underlying_type_t<_Ty>;

	template <typename _Ty>
	using remove_reference_t = std::remove_reference_t<_Ty>;

	template <typename _Ty>
	constexpr remove_reference_t<_Ty>&& move(_Ty&& _Arg)
	{
		return static_cast<remove_reference_t<_Ty>&&>(_Arg);
	}

	template <size_t I>
	struct integral_constant {
		static constexpr size_t value = I;
	};
}
