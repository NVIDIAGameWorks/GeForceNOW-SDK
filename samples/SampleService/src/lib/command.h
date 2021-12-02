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

#ifndef NV_CASE_RETURN_ENUM_STRING
#define NV_CASE_RETURN_ENUM_STRING(enm) case enm: return L#enm
#endif

namespace SampleService
{
	enum class command
	{
		create,
		isRunningInCloudSecure,

		max_enum_value
	};

	inline const wchar_t* enumPrinter(command enumclass)
	{
		switch (enumclass)
		{
			NV_CASE_RETURN_ENUM_STRING(command::create);
			NV_CASE_RETURN_ENUM_STRING(command::isRunningInCloudSecure);
			NV_CASE_RETURN_ENUM_STRING(command::max_enum_value);
		}
		return L"Unknown enumeration. Add me to enumPrinter";
	}
}