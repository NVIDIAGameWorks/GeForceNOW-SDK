/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "server.h"
#include <cassert>
#include <sstream>
#include <iostream>
#include "GfnRuntimeSdk_Wrapper.h"

namespace SampleService
{
	ServiceServer::ServiceServer()
		: m_pipe(
			interface_port_name,
			std::bind(
				&ServiceServer::receiver,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3),
			true /* allow non-admin users */,
			1 /* max instances */ )
	{
		registerCommands();
	}

	void ServiceServer::receiver(DeserializeIterator& request, SerializeIterator& reply, LPCPipeContext& ctx)
	{
		const auto cmd = request.get<command>();
		const auto handler = m_root_commands.find(cmd);

		auto* status = reply.reserve(status::failed_to_process_command);
		if (handler == std::end(m_root_commands))
		{
			*status = status::command_not_found;
			return;
		}

		try
		{
			*status = m_root_commands[cmd](request, reply, ctx);
		}
		catch (const std::exception& e)
		{
			std::cout << "Failed to process command " <<  " exception: " << e.what() << std::endl;
			*status = status::failed_to_process_command;
		}
	}

	status ServiceServer::start()
	{
		try {
			// Initialize Geforce NOW Runtime SDK using the C calling convention.
			GfnRuntimeError err = GfnInitializeSdk(gfnDefaultLanguage);
			if (err != gfnSuccess)
			{
				std::cout << "Error initializing the sdk: " << err << std::endl;
			}

			return m_pipe.start() ? status::success : status::failed_to_start_service;
		}
		catch (const std::exception& e)
		{
			std::cout << "Failed to start, exception occurred: " << e.what() << std::endl;
			return status::failed_to_start_service;
		}
	}

	std::tuple<status, std::wstring> ServiceServer::create(const std::wstring& name)
	{
		return{ status::success, name + L"_out" };
	}

	status ServiceServer::createHandler(DeserializeIterator& request, SerializeIterator& reply, LPCPipeContext& /*ctx*/)
	{
		const auto name = request.get<std::wstring>();

		if (!request.finalize())
		{
			std::cout << "Failed to deserialize request @ create_handler" << std::endl;
			return status::deserialization_error;
		}

		const auto& [result, new_id] = create(name);
		reply.put(new_id);
		return result;
	}

	std::tuple<status, std::wstring, std::wstring> ServiceServer::isRunningInCloudSecure()
	{
		GfnIsRunningInCloudAssurance assurance = GfnIsRunningInCloudAssurance::gfnNotCloud;
		GfnError err = GfnIsRunningInCloudSecure(&assurance);
		if (err != GfnError::gfnSuccess)
		{
			std::cout << "Failed to get if running in cloud. Error: " << err << std::endl;
			return{ status::success, std::to_wstring(err), L"Failed to get if running in cloud.Error: " + err};
		}
		std::cout << "GfnIsRunningInCloudSecure assurance " << assurance << "\n";

		return{ status::success, std::to_wstring(err), std::to_wstring(assurance)};
	}

	status ServiceServer::isRunningInCloudRequestHandler(DeserializeIterator& request, SerializeIterator& reply, LPCPipeContext& /*ctx*/)
	{
		if (!request.finalize())
		{
			std::cout << "Failed to deserialize request @ handleisRunningInCloudRequest" << std::endl;
			return status::deserialization_error;
		}
		
		const auto& [result, gfnerror, response] = isRunningInCloudSecure();
		reply.put(gfnerror);
		reply.put(response);
		return result;
	}

	void ServiceServer::registerCommands()
	{
		namespace p = std::placeholders;
		m_root_commands.emplace(command::create, std::bind(&ServiceServer::createHandler, this, p::_1, p::_2, p::_3));
		m_root_commands.emplace(command::isRunningInCloudSecure, std::bind(&ServiceServer::isRunningInCloudRequestHandler, this, p::_1, p::_2, p::_3));
	}
}
