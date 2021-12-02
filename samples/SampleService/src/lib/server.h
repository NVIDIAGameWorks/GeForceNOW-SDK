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

#include <lpc_pipe.h>
#include <functional>
#include <unordered_map>
#include "status.h"
#include "command.h"
#include "port_name.h"

namespace SampleService
{
	class ServiceServer : public Utils::NonCopyable
	{
		typedef status(t_handler)(
			DeserializeIterator& request,
			SerializeIterator& reply,
			LPCPipeContext& ctx);

		using dispatch_table = std::unordered_map<command, std::function<t_handler>>;

		///////////////////////////////////////////////
		LPCPipeServer m_pipe;
		dispatch_table m_root_commands;

		void receiver(
			DeserializeIterator& request,
			SerializeIterator& reply,
			LPCPipeContext& ctx);

		///////////////////////////////////////////////
		// command handlers
		status createHandler(DeserializeIterator& request, SerializeIterator& reply, LPCPipeContext& ctx);

		std::tuple<status, std::wstring> create(const std::wstring& name);

		status isRunningInCloudRequestHandler(DeserializeIterator& request, SerializeIterator& reply, LPCPipeContext& ctx);

		std::tuple<status, std::wstring, std::wstring> isRunningInCloudSecure();

		void registerCommands();

	public:
		ServiceServer();
		status start();
	};
}
