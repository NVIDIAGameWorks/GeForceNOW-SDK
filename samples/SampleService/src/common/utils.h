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
#include <Windows.h>
#include <string>
#include <memory>

namespace SampleService
{
	namespace Utils
	{
		class Event
		{
			HANDLE m_event_object;

			void createEvent(bool manual_reset);
			void destroyEvent() const;

			Event& operator= (const Event&) = delete;
			Event(const Event&) = delete;

		public:

			Event(bool manual_reset = true) : m_event_object(NULL)
			{ 
				createEvent(manual_reset);
			};
			~Event()
			{
				destroyEvent();
			};

			operator bool() const { return m_event_object != nullptr; }
			void signal() const;
			void wait() const;
			bool waitAlertable() const;
			void reset() const;
		};

		class InterruptableOverlapped
		{
			OVERLAPPED m_overlapped;
			HANDLE m_cancel_event;
			HANDLE m_events[2];

			InterruptableOverlapped& operator=(const InterruptableOverlapped&);
			InterruptableOverlapped(const InterruptableOverlapped&);

		public:
			InterruptableOverlapped();
			~InterruptableOverlapped();

			OVERLAPPED* get() const { return const_cast<OVERLAPPED*>(&m_overlapped); }
			void interrupt() const;
			bool wait() const;
			void reset() const;
		};

		class NonCopyable
		{
		public:
			NonCopyable() {};
			NonCopyable(const NonCopyable&) = delete;
			NonCopyable(NonCopyable&&) = delete;
			NonCopyable& operator=(const NonCopyable&) = delete;
		};
	}
}
