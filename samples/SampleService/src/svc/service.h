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

#include <string>
#include <functional>

namespace SampleService
{
	namespace service
	{
		using service_run = std::function<uint32_t()>;
		using service_stop = std::function<void(bool)>;

		uint32_t run(const std::wstring& service_name, service_run&& run, service_stop&& stop);
	}
}
