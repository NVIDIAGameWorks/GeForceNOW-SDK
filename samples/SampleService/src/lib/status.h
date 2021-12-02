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
#include <cstdint>

#ifndef NV_CASE_RETURN_ENUM_STRING
#define NV_CASE_RETURN_ENUM_STRING(enm) case enm: return L#enm
#endif

namespace SampleService
{
	enum class status : uint32_t
	{
		success = 0,
		failed_to_create_pipe,
		not_implemented,
		deserialization_error,
		param_validation_error,
		command_not_found,
		invalid_request,
		request_is_not_completed,
		invalid_metadata,
		help_requested,
		failed_to_start_service,
		failed_to_process_command,

		max_enum_value
	};

	inline const wchar_t* enumPrinter(status enumclass)
	{
		switch (enumclass)
		{
			NV_CASE_RETURN_ENUM_STRING(status::success);
			NV_CASE_RETURN_ENUM_STRING(status::failed_to_create_pipe);
			NV_CASE_RETURN_ENUM_STRING(status::not_implemented);
			NV_CASE_RETURN_ENUM_STRING(status::deserialization_error);
			NV_CASE_RETURN_ENUM_STRING(status::param_validation_error);
			NV_CASE_RETURN_ENUM_STRING(status::command_not_found);
			NV_CASE_RETURN_ENUM_STRING(status::invalid_request);
			NV_CASE_RETURN_ENUM_STRING(status::request_is_not_completed);
			NV_CASE_RETURN_ENUM_STRING(status::invalid_metadata);
			NV_CASE_RETURN_ENUM_STRING(status::help_requested);
			NV_CASE_RETURN_ENUM_STRING(status::failed_to_start_service);
			NV_CASE_RETURN_ENUM_STRING(status::failed_to_process_command);
			NV_CASE_RETURN_ENUM_STRING(status::max_enum_value);
		}
		return L"Unknown enumeration. Add me to enumPrinter";
	}
}

#undef NV_CASE_RETURN_ENUM_STRING
