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

#include "traits.h"
#include "serialize_common.h"

template <typename T>
traits::enable_if_t<traits::is_pod_struct_v<T>, traits::integral_constant<cell_size * 2 + sizeof(T)>>
determine_packed_size(const T&)
{
	return {}; // returns cell_size * 2;
}

template <typename T>
traits::enable_if_t<traits::is_enum_v<T> || traits::is_integral_v<T>, traits::integral_constant<cell_size * 2>>
determine_packed_size(const T&)
{
	return {}; // returns cell_size * 2 + sizeof(T);
}

template <typename T>
traits::enable_if_t<traits::is_unspecified_v<T>, T>
determine_packed_size(const T&)
{
	static_assert(false, "We can determine sizes of nums, enums and PODs only");
	return T{};
}

template <typename T>
static constexpr size_t estimated_size = decltype(determine_packed_size(T{}))::value;

template <typename HEAD, typename ... REST>
struct estimated_buffer_size_impl {
	static constexpr size_t value =	estimated_size<HEAD> +
		estimated_buffer_size_impl<REST...>::value;
};

template <typename HEAD>
struct estimated_buffer_size_impl<HEAD>
{
	static constexpr size_t value = estimated_size<HEAD>;
};

template <typename ... ARGS>
struct estimated_buffer_size
{
	// cell_size is first 8 bytes for arg count
	static constexpr size_t value = cell_size +	estimated_buffer_size_impl<ARGS...>::value;
};

template <typename ... ARGS>
using deserialize_buffer = unsigned char[estimated_buffer_size<ARGS...>::value];
