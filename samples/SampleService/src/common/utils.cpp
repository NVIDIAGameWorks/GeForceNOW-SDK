/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "utils.h"
#include <cassert>
#include <Shlwapi.h>
#include <sddl.h>

using namespace SampleService::Utils;

InterruptableOverlapped::InterruptableOverlapped()
{
	ZeroMemory(&m_overlapped, sizeof(m_overlapped));

	m_overlapped.hEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
	m_cancel_event = CreateEventW(NULL, TRUE, FALSE, NULL);

	m_events[0] = m_overlapped.hEvent;
	m_events[1] = m_cancel_event;
}

InterruptableOverlapped::~InterruptableOverlapped()
{
	if (m_overlapped.hEvent)
	{
		CloseHandle(m_overlapped.hEvent);
	}

	if (m_cancel_event)
	{
		CloseHandle(m_cancel_event);
	}
}

void InterruptableOverlapped::interrupt() const
{
	SetEvent(m_cancel_event);
}

bool InterruptableOverlapped::wait() const
{
	const auto wait_status = WaitForMultipleObjects(
		2,				// we have only two events:
		&m_events[0],	// one for message another for the cancel event
		false,			// wait for any of them
		INFINITE);

	return wait_status == WAIT_OBJECT_0;
}

void InterruptableOverlapped::reset() const
{
	ResetEvent(m_overlapped.hEvent);
	ResetEvent(m_cancel_event);
}


void Event::createEvent(bool manual_reset)
{
	m_event_object = CreateEventW(NULL, manual_reset, FALSE, NULL);
	if (!m_event_object)
	{
		//LOG_ERR(L"Failed to create event: {}", logger::last_error_str());
	}
}

void Event::destroyEvent() const
{
	if (m_event_object)
	{
		CloseHandle(m_event_object);
	}
}

void Event::signal() const
{
	if (m_event_object)
	{
		SetEvent(m_event_object);
	}
}

void Event::wait() const
{
	if (!m_event_object) return;
	WaitForSingleObject(m_event_object, INFINITE);
}

void Event::reset() const
{
	if (!m_event_object) return;
	ResetEvent(m_event_object);
}

bool Event::waitAlertable() const
{
	if (!m_event_object)
	{
		return true;
	}

	const DWORD result = WaitForSingleObjectEx(m_event_object, INFINITE, TRUE);
	if (result == WAIT_FAILED)
	{
		//LOG_ERR(L"Failed to wait for event: {}", logger::last_error_str());
		return true;
	}

	return result == WAIT_OBJECT_0;
}
