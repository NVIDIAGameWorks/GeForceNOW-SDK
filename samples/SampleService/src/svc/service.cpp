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
#include <cassert>
#include <Shlobj.h>
#include <codecvt>
#include <sstream>
#include <iostream>
#include <fstream>
#include "service.h"
#include "utils.h"

namespace SampleService
{
	namespace service
	{
		static SERVICE_STATUS g_service_status{};
		static SERVICE_STATUS_HANDLE g_service_status_handle = nullptr;
		static std::wstring g_service_name;
		static service_run g_run;
		static service_stop g_stop;
		static std::streambuf *g_coutOriginalBuffer;

		static uint32_t reportStatus(uint32_t current_status, uint32_t exit_code = NO_ERROR)
		{
			g_service_status.dwWin32ExitCode = exit_code;
			g_service_status.dwCurrentState = current_status;
			g_service_status.dwControlsAccepted = current_status == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;

			if (current_status == SERVICE_RUNNING || current_status == SERVICE_STOPPED)
			{
				g_service_status.dwCheckPoint = 0;
				g_service_status.dwWaitHint = 0;
			}
			else
			{
				++g_service_status.dwCheckPoint;
				g_service_status.dwWaitHint = 90 * 1000;
			}

			if (!SetServiceStatus(g_service_status_handle, &g_service_status))
			{
				const uint32_t result = GetLastError();
				std::cout << "Failed to set service status, error: " << result << std::endl;
				return result;
			}

			return NO_ERROR;
		}

		static uint32_t initializeService()
		{
			g_service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
			return reportStatus(SERVICE_START_PENDING);
		}

		static DWORD WINAPI serviceControl(
			__in  DWORD dwControl,
			__in  DWORD dwEventType,
			__in  LPVOID lpEventData,
			__in  LPVOID lpContext)
		{
			UNREFERENCED_PARAMETER(lpContext);
			UNREFERENCED_PARAMETER(lpEventData);
			UNREFERENCED_PARAMETER(dwEventType);

			if (dwControl == SERVICE_CONTROL_STOP || dwControl == SERVICE_CONTROL_SHUTDOWN)
			{
				reportStatus(SERVICE_STOP_PENDING);
				const bool wait_for_finish = dwControl == SERVICE_CONTROL_SHUTDOWN;
				assert(g_stop);
				g_stop(wait_for_finish);
				std::cout << "Service Stopping" << std::endl;
				//Restore cout's original buffer back.
				std::cout.rdbuf(g_coutOriginalBuffer);
			}
			else
			{
				reportStatus(g_service_status.dwCurrentState);
			}

			return NO_ERROR;
		}

		static void WINAPI serviceMain(DWORD, LPWSTR*)
		{
			// Initialize Service logs, note that Service cannot write into user's localappdata
			// Create logging directory if it doesn't exist.
			wchar_t* programDataPath = NULL;
			auto hr = SHGetKnownFolderPath(FOLDERID_ProgramData, KF_FLAG_DEFAULT, NULL, &programDataPath);
			if (FAILED(hr))
			{
				std::cout << "Could not get path to local app data folder" << std::endl;
				return;
			}

			std::wstringstream pathSS;
			pathSS << programDataPath << L"\\NVIDIA Corporation\\GfnRuntimeSdk";
			CoTaskMemFree(programDataPath);

			int createDirResult = SHCreateDirectoryExW(NULL, pathSS.str().c_str(), NULL);
			if (createDirResult != ERROR_SUCCESS && createDirResult != ERROR_FILE_EXISTS && createDirResult != ERROR_ALREADY_EXISTS)
			{
				std::cout << "Could not create logging directory" << std::endl;
				return;
			}

			pathSS << "\\GfnSdkSampleService.log";

			// To write to a file using cout, first save the std::cout buffer and then assign it to log file
			g_coutOriginalBuffer = std::cout.rdbuf();
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			std::ofstream out(converter.to_bytes(pathSS.str()).c_str());
			//Write to log through std::cout 
    		std::cout.rdbuf(out.rdbuf());

			int result = NO_ERROR;
			g_service_status_handle = RegisterServiceCtrlHandlerExW(g_service_name.c_str(), serviceControl, NULL);
			if (g_service_status_handle == nullptr)
			{
				std::cout << "Failed to register service control handler, error: " << GetLastError() << std::endl;
				return;
			}
			result = initializeService();
			if (result == NO_ERROR)
			{
				result = reportStatus(SERVICE_RUNNING);
			}
			if (result == NO_ERROR)
			{
				// This code execited in running state
				assert(g_run);
				result = g_run();
			}
			reportStatus(SERVICE_STOPPED, result);
		}

		uint32_t run(const std::wstring& service_name, service_run&& run, service_stop&& stop)
		{
			g_service_name = service_name;
			g_run = run;
			g_stop = stop;

			const SERVICE_TABLE_ENTRYW service_dispatch_table[] = {
				{ const_cast<wchar_t*>(g_service_name.c_str()), serviceMain },
				{ nullptr, nullptr }
			};

			if (!StartServiceCtrlDispatcherW(service_dispatch_table))
			{
				const uint32_t result = GetLastError();
				return result;
			}
			return ERROR_SUCCESS;
		}
	}
}
