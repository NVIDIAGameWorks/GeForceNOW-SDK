/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include <string>
#include <tuple>
#include "lpc_pipe.h"
#include "client.h"
#include "command.h"
#include "port_name.h"
#include <transport.h>

namespace SampleService
{
	using namespace Transport;
	static const size_t connect_timeout_ms = 10 * 1000; // 10 seconds

	ServiceClient::ServiceClient()
		: m_pipe(interface_port_name)
	{}

	std::tuple<status, std::wstring> ServiceClient::create(const std::wstring& name)
	{
		return send_impl<std::wstring>(m_pipe, connect_timeout_ms, command::create, name);
	}

	std::tuple<status, std::wstring, std::wstring> ServiceClient::isRunningInCloudSecure()
	{
		return send_impl<std::wstring, std::wstring>(m_pipe, connect_timeout_ms, command::isRunningInCloudSecure);
	}
}
