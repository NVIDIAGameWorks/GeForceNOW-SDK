/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#define NOMINMAX
#include <Windows.h>
#include <csignal>
#include <map>
#include <functional>
#include <iostream>
#include "instance.h"
#include "service.h"

std::wstring service_name = L"GfnSdkSampleService";

uint32_t run_as_console_application()
{
	const PHANDLER_ROUTINE handler_routine = [](DWORD dwCtrlType) -> BOOL
	{
		if (dwCtrlType == CTRL_CLOSE_EVENT || dwCtrlType == CTRL_C_EVENT)
		{
			SampleService::instance::stop(/*wait_for_finish*/ true);
			return TRUE;
		}
		return FALSE;
	};

	SetConsoleCtrlHandler(handler_routine, TRUE);
	return SampleService::instance::run();
}

using command_handler = std::function<uint32_t()>;

class command
{
public:
	command(command_handler handler, std::wstring&& help) :
		m_handler(std::move(handler)),
		m_help(std::move(help))
	{}

	uint32_t operator()() const
	{
		return m_handler();
	}

	const std::wstring& get_help() const
	{
		return m_help;
	}

private:
	const command_handler m_handler;
	const std::wstring m_help;
};

const static std::map<std::wstring, command> commands_map{
	{ L"run",     command(run_as_console_application, L"RUN - run as a console application") }
};

void displayHelp()
{
	for (const auto& command: commands_map)
	{
		std::wcout << command.second.get_help() << std::endl;
	}
}

int wmain(int argc, wchar_t** argv)
{
	try
	{
		auto it = commands_map.end();
		if (argc > 1)
		{
			CharLowerW(argv[1]);
			it = commands_map.find(argv[1]);
		}

		if (it != commands_map.end())
		{
			return it->second();
		}

		auto result = SampleService::service::run(service_name, SampleService::instance::run, SampleService::instance::stop);
		if (result != NO_ERROR)
		{
			std::cout << "Failed to start service, error: " << result << std::endl;
		}

		// This error is returned if the program is being run as a console application rather than as a service.
		if (result == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			displayHelp();
		}

		return result;
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception occurred: " << e.what() << std::endl;
		return 1;
	}
}
