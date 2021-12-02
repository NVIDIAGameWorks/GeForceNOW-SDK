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
#include "instance.h"
#include "utils.h"
#include <server.h>
#include <status.h>

namespace SampleService
{
	namespace instance
	{
		Utils::Event m_exit_event;
		Utils::Event m_finished_event;

		uint32_t run()
		{
			auto result = status::failed_to_start_service;

			try
			{
				std::unique_ptr<ServiceServer> server = std::make_unique<SampleService::ServiceServer>();
				result = server->start();
				if (result == status::success)
				{
					m_exit_event.wait();
				}
			}
			catch (const std::exception&)
			{
				result = status::failed_to_start_service;
			}

			m_finished_event.signal();
			return static_cast<uint32_t>(result);
		}

		void stop(bool wait_for_finish)
		{
			m_exit_event.signal();
			if (wait_for_finish)
			{
				m_finished_event.wait();
			}
		}
	}
}
