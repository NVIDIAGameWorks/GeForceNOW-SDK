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

using cell_type = unsigned long long;
static constexpr auto cell_size = sizeof(cell_type);
using disposal_holder = cell_type[3];

static constexpr cell_type integral_type = 0x0077777777777700ULL;
static constexpr cell_type enum_type = 0x0055555555555500ULL;
static constexpr cell_type pod_struct_type = 0x0033333333333300ULL;
static constexpr cell_type string_type = 0x0011111111111100ULL;
static constexpr cell_type wstring_type = 0x0022222222222200ULL;
static constexpr cell_type buffer_type = 0x00AAAAAAAAAAAA00ULL;
static constexpr cell_type variable_buffer_type = 0x00ABBACCCCABBA00ULL;
static constexpr cell_type error_type = 0x0ULL;

#include <string>
#include <vector>

using array_t = std::vector<char>;
using string_t = std::string;
using wstring_t = std::wstring;

inline bool safe_memcpy(void* dst, const void* src, size_t size)
{
	bool success{ true };
	__try {
		memcpy_s(dst, size, src, size);
	} __except (1) { // fix dpanin
		success = false;
	}
	return success;
}
