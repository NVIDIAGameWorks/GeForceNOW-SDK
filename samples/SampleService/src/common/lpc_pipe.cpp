/*
* Copyright (c) 2016-2021, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/
#include "lpc_pipe.h"
#include <assert.h>
#include <algorithm>
#include <AclAPI.h>
#include <iostream>

using namespace SampleService;

static const DWORD STATUS_PIPE_BROKEN = 0xc000014b;

class SecurityAttributes
{
public:
	SecurityAttributes()
		: m_full_access(false)
	{}

	~SecurityAttributes()
	{
		freeFullAccess();
	}

	bool makeFullAccess()
	{
		// Create a well-known SID for the Everyone group.
		SID_IDENTIFIER_AUTHORITY sid_auth_world = SECURITY_WORLD_SID_AUTHORITY;
		if (!AllocateAndInitializeSid(&sid_auth_world, 1,
			SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &m_sid))
		{
			std::cout << "Failed to initialized SID: " << GetLastError() << std::endl;
			return false;
		}

		// Initialize an EXPLICIT_ACCESS structure for an ACE.
		// The ACE will allow Everyone full access to the file.
		EXPLICIT_ACCESS explicit_access = {};
		explicit_access.grfAccessPermissions = GENERIC_ALL;
		explicit_access.grfAccessMode = GRANT_ACCESS;
		explicit_access.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
		explicit_access.Trustee.TrusteeForm = TRUSTEE_IS_SID;
		explicit_access.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		explicit_access.Trustee.ptstrName = reinterpret_cast<LPTSTR>(m_sid);

		const auto result = SetEntriesInAcl(1, &explicit_access, nullptr, &m_acl);
		if (result != ERROR_SUCCESS)
		{
			std::cout << "Failed to set entries in ACL: " << result << std::endl;
			freeFullAccess();
			return false;
		}

		if (InitializeSecurityDescriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION) != TRUE)
		{
			std::cout << "Failed to initialize security descriptor: " << GetLastError() << std::endl;
			freeFullAccess();
			return false;
		}

		if (SetSecurityDescriptorDacl(&descriptor, true, m_acl, false) != TRUE)
		{
			std::cout << "Failed to set dacl to security descriptor: " << GetLastError() << std::endl;
			freeFullAccess();
			return false;
		}

		attributes.nLength = sizeof(attributes);
		attributes.lpSecurityDescriptor = &descriptor;
		attributes.bInheritHandle = false;
		m_full_access = true;
		return true;
	}

	void freeFullAccess()
	{
		if (m_sid != nullptr)
		{
			FreeSid(m_sid);
			m_sid = nullptr;
		}
		if (m_acl != nullptr)
		{
			LocalFree(m_acl);
			m_acl = nullptr;
		}
		m_full_access = false;
	}

	SECURITY_ATTRIBUTES* get()
	{
		return m_full_access ? &attributes : nullptr;
	}

private:
	PSID m_sid = nullptr;
	PACL m_acl = nullptr;
    SECURITY_DESCRIPTOR descriptor = {};
	bool m_full_access;
	SECURITY_ATTRIBUTES attributes;
};

struct transfered_pipe_message
{
	details::connection_control control;
	char payload[1];
};

static constexpr size_t CONTROL_SIZE = sizeof(transfered_pipe_message) - sizeof(char);

bool LPCPipeContext::impersonate()
{
	if (m_impersonated)
	{
		return true;
	}

	const auto result = ImpersonateNamedPipeClient(m_pipe);
	if (!result)
	{
		std::cout << "ImpersonateNamedPipeClient() failed with: " << GetLastError() << std::endl;
		return false;
	}

	m_impersonated = true;
	std::cout << "Impersonation is enabled for " << std::this_thread::get_id() << std::endl;
	return true;
}

bool LPCPipeContext::revertToSelf()
{
	if (!m_impersonated)
	{
		return true;
	}

	const auto result = RevertToSelf();
	if (!result)
	{
		std::cout << "RevertToSelf() failed with: " << GetLastError() << std::endl;
		return false;
	}

	m_impersonated = false;
	std::cout << "Impersonation is reverted for " << std::this_thread::get_id() << std::endl;
	return true;
}

void LPCPipeServer::checkFinishedListeners()
{
	if (m_listeners.empty()) return;

	for (auto& listener : m_listeners)
	{
		if (!listener->isConnected())
		{
			std::cout << "Found a finished listener: " << (void*)listener << std::endl;
			listener->join();
			delete listener;
			listener = nullptr;
		}
	}

	const auto to_remove = std::remove(std::begin(m_listeners), std::end(m_listeners), nullptr);

	if (to_remove != m_listeners.end()) m_listeners.erase(to_remove, std::end(m_listeners));
}

void LPCPipeServer::stopAllListeners()
{
	std::cout << "Stopping all listeners" << std::endl;
	m_running = false;

	for (auto& listener : m_listeners)
	{
		listener->interrupt();
		listener->join();
		delete listener;
		listener = nullptr;
	}

	m_listeners.clear();
}

void LPCPipeServer::accepterThread()
{
	while (m_running)
	{
		checkFinishedListeners();
		const auto result = accept();

		switch(result.first)
		{
		case details::connection_result::failure:
		case details::connection_result::interrupt:
			std::cout << "Interrupt" << std::endl;
			m_running = false;
			break;
		case details::connection_result::busy:
			Sleep(10);
			break;
		case details::connection_result::success:
			try
			{
				m_listeners.push_back(new LPCPipeListener{ result.second, *this });
			}
			catch (const std::exception& e)
			{
				std::cout << "Exception when try to add listener: " << e.what() << std::endl;
				CloseHandle(result.second);
			}
			break;
		default: break;
		}
	}

	stopAllListeners();
}

LPCPipeServer::accept_result LPCPipeServer::accept() const
{
	SecurityAttributes attributes;
	if (m_allow_non_admin)
	{
		if (!attributes.makeFullAccess())
		{
			return { details::connection_result::failure, INVALID_HANDLE_VALUE };
		}
	}

	auto pipe = CreateNamedPipeW(
		m_name.c_str(),                            // pipe name
		PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED, // read/write access
		PIPE_TYPE_MESSAGE |                        // message type pipe
		PIPE_READMODE_MESSAGE |                    // message-read mode
		PIPE_WAIT,                                 // blocking mode
		static_cast<DWORD>(m_max_instances),       // max. instances
		DEFAULT_PIPE_BUFFER_SIZE,                  // output buffer size
		DEFAULT_PIPE_BUFFER_SIZE,                  // input buffer size
		0,                                         // client time-out
		attributes.get());                         // default security attribute
	auto last_error = GetLastError();

	if (pipe == INVALID_HANDLE_VALUE)
	{
		if (last_error == ERROR_PIPE_BUSY)
		{
			return{ details::connection_result::busy, INVALID_HANDLE_VALUE };
		}

		std::cout << "CreateNamedPipeW() failed with: " << last_error << std::endl;
		return{ details::connection_result::failure, INVALID_HANDLE_VALUE };
	}

	std::cout << "Successfully created pipe named: " << m_name.c_str() << std::endl;

	auto result = ConnectNamedPipe(pipe, m_overlapped.get()) > 0;
	last_error = GetLastError();

	if (!result)
	{
		if (last_error == ERROR_IO_PENDING)
		{
			m_overlapped.wait();

			if (!HasOverlappedIoCompleted(m_overlapped.get()))
			{
				CloseHandle(pipe);
				return { details::connection_result::interrupt, INVALID_HANDLE_VALUE };
			}

			result = true;
		}
		else if (last_error == ERROR_PIPE_CONNECTED)
		{
			// already connected
			result = true;
		}
	}

	if (!result)
	{
		std::cout << "ConnectNamedPipe() failed with: " << last_error << std::endl;
		return { details::connection_result::failure, INVALID_HANDLE_VALUE };
	}

	return { details::connection_result::success, pipe };
}

LPCPipeServer::~LPCPipeServer()
{
	stop();
}

const std::function<t_incoming_message_cbk>& LPCPipeServer::callback() const
{
	return m_incoming_message_callback;
}

bool LPCPipeServer::start()
{
	m_overlapped.reset();

	if (m_running)
	{
		std::cout << "Server is already running" << std::endl;
		return true;
	}

	m_running = true;
	m_accepter = std::thread(&LPCPipeServer::accepterThread, this);

	return true;
}

void LPCPipeServer::stop()
{
	std::lock_guard<std::mutex> lock(m_stopping_mutex);
	m_running = false;
	m_overlapped.interrupt();
	if (m_accepter.joinable())
	{
		m_accepter.join();
	}
	stopAllListeners();
}

const std::wstring& LPCPipeServer::pipeName() const
{
	return m_name;
}

bool LPCPipeServer::isRunning() const
{
	return m_running;
}

void LPCPipeListener::listenerThread()
{
	if (!initialize())
	{
		disconnect();
		return;
	}

	while (m_server.isRunning())
	{
		const auto control = receive();

		if (control == details::connection_control::remote_disconnected)
		{
			break;
		}
	}

	disconnect();
}

bool LPCPipeListener::initialize()
{
	try
	{
		m_request_buffer.resize(DEFAULT_PIPE_BUFFER_SIZE);
		m_reply_buffer.resize(DEFAULT_PIPE_BUFFER_SIZE);
	}
	catch (std::exception& e)
	{
		std::cout << "Exception occurred: " << e.what() << std::endl;
		return false;
	}

	return true;
}

details::connection_control LPCPipeListener::receive()
{
	auto& request_buffer = *reinterpret_cast<transfered_pipe_message*>(m_request_buffer.data());
	auto& reply_buffer = *reinterpret_cast<transfered_pipe_message*>(m_reply_buffer.data());

	DWORD bytes_read = 0;
	auto result = ReadFile(
		m_pipe,                                      // handle to pipe
		&request_buffer,                             // buffer to receive data
		static_cast<DWORD>(m_request_buffer.size()), // size of buffer
		&bytes_read,                                 // number of bytes read
		m_overlapped.get());                         // not overlapped I/O

	auto last_error = GetLastError();

	if (!result && last_error == ERROR_IO_PENDING)
	{
		m_overlapped.wait();
		result = true;
		bytes_read = static_cast<DWORD>(m_overlapped.get()->InternalHigh);

		if (!HasOverlappedIoCompleted(m_overlapped.get()))
		{
			// NOTE: actually it's not that remote side is disconnected, but
			// the IO has been interrupted
			return details::connection_control::remote_disconnected;
		}

		const auto ntstatus = static_cast<DWORD>(m_overlapped.get()->Internal);

		if (ntstatus == STATUS_PIPE_BROKEN)
		{
			return details::connection_control::remote_disconnected;;
		}
	}
	else if (last_error == ERROR_BROKEN_PIPE)
	{
		return details::connection_control::remote_disconnected;
	}

	if (bytes_read == 0)
	{
		std::cout << "ReadFile() failed with " << last_error << std::endl;
		return details::connection_control::keep_connection;
	}

	if (request_buffer.control == details::connection_control::disconnect)
	{
		return details::connection_control::remote_disconnected;
	}

	DeserializeIterator request(&request_buffer.payload[0], bytes_read - CONTROL_SIZE);
	unsigned long reply_size_ul = 0;
	SerializeIterator reply(&reply_buffer.payload[0], m_reply_buffer.size() - CONTROL_SIZE, &reply_size_ul);

	try 
	{
		LPCPipeContext ctx(m_pipe);
		m_server.callback()(request, reply, ctx);
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception while process message in lpc callback: " << e.what() << std::endl;
	}

	DWORD bytes_written = 0;
	result = WriteFile(
		m_pipe,                       // handle to pipe
		&reply_buffer,                // buffer to write from
		reply_size_ul + CONTROL_SIZE, // number of bytes to write
		&bytes_written,               // number of bytes written
		m_overlapped.get());          // not overlapped I/O

	last_error = GetLastError();

	if (!result && last_error == ERROR_IO_PENDING)
	{
		m_overlapped.wait();
	}

	return details::connection_control::keep_connection;
}

void LPCPipeListener::disconnect()
{
	assert(m_pipe != INVALID_HANDLE_VALUE);
	std::cout << "Disconnecting client from: " << m_server.pipeName().c_str() << std::endl;
	FlushFileBuffers(m_pipe);
	DisconnectNamedPipe(m_pipe);
	CloseHandle(m_pipe);
	m_pipe = INVALID_HANDLE_VALUE;
	m_overlapped.interrupt();
	m_overlapped.reset();
}

bool LPCPipeClient::internalSend(
	const details::connection_control control,
	const uint32_t size,
	uint32_t& reply_size) const
{
	auto& buffer = *(transfered_pipe_message*)m_request_buffer.data();

	if (size > m_request_buffer.size() - CONTROL_SIZE)
	{
		std::cout << "size > m_request_buffer.size() - control_size" << std::endl;
		return false;
	}

	buffer.control = control;

	DWORD bytes_written = 0;
	auto result = WriteFile(
		m_pipe,              // handle to pipe
		&buffer,             // buffer to write from
		size + CONTROL_SIZE, // number of bytes to write
		&bytes_written,      // number of bytes written
		m_overlapped.get()); // not overlapped I/O

	auto last_error = GetLastError();

	if (!result && last_error == ERROR_IO_PENDING)
	{
		m_overlapped.wait();
		result = true;
		bytes_written = (DWORD)m_overlapped.get()->InternalHigh;
		last_error = GetLastError();
	}

	if (result == FALSE)
	{
		assert(L"this shouldn't happen if it does the pipe is probably broken(closed), handle this case");
		// and check the last_error here
		return false;
	}

	DWORD bytes_read = 0;
	result = ReadFile(
		m_pipe,                         // handle to pipe
		(void*)&buffer,                 // buffer to receive data
		(DWORD)m_request_buffer.size(), // size of buffer
		&bytes_read,                    // number of bytes read
		m_overlapped.get());            // not overlapped I/O

	last_error = GetLastError();

	if (!result && last_error == ERROR_IO_PENDING)
	{
		m_overlapped.wait();
		result = true;
		bytes_read = (DWORD)m_overlapped.get()->InternalHigh;
		last_error = GetLastError();
	}

	reply_size = bytes_read - CONTROL_SIZE;

	return true;
}

bool LPCPipeClient::send(uint32_t request_size, uint32_t& reply_size) const
{
	return internalSend(details::connection_control::keep_connection, request_size, reply_size);
}

details::connection_result LPCPipeClient::internalConnect()
{
	using namespace details;

	if (!initialize())
	{
		return connection_result::failure;
	}

	if (isConnected())
	{ 
		return connection_result::success;
	}

	m_pipe = CreateFileW(
		m_name.c_str(),      // pipe name
		GENERIC_READ |       // read and write access
		GENERIC_WRITE,
		0,                   // no sharing
		NULL,                // default security attributes
		OPEN_EXISTING,       // opens existing pipe
		FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IMPERSONATION,// attributes
		NULL);               // no template file

	const auto last_error = GetLastError();

	// TODO: NOTE that ERROR_FILE_NOT_FOUND is treated as a busy pipe
	// because when max amount of pipe instances is reached CreateFile returns ERROR_FILE_NOT_FOUND
	if (last_error == ERROR_PIPE_BUSY || last_error == ERROR_FILE_NOT_FOUND)
	{
		return connection_result::busy;
	}

	if (m_pipe == INVALID_HANDLE_VALUE)
	{
		std::cout << "CreateFileW() failed with " << last_error << std::endl;
		return connection_result::failure;
	}

	DWORD mode = PIPE_READMODE_MESSAGE;
	SetNamedPipeHandleState(m_pipe, &mode, NULL, NULL);

	std::wcout << "Successfully connected pipe named: " << m_name << std::endl;
	return connection_result::success;
}

bool LPCPipeClient::connect(size_t _timeout, size_t refresh_rate)
{
	if (isConnected())
	{
		std::cout << "Pipe" << m_name.c_str() << " is already connected" << std::endl;
		return false;
	}

	std::wcout << "Connecting to port: " << m_name << std::endl;

	m_overlapped.reset();

	using system_time_point = std::chrono::time_point<std::chrono::system_clock>;
	using ms = std::chrono::milliseconds;
	const auto time_started = std::chrono::system_clock::now();
	auto time_now = time_started;

	const auto timeout = ms(_timeout);
	auto diff = ms(0);

	while (diff < timeout)
	{
		const auto result = internalConnect();

		switch (result)
		{
		case details::connection_result::failure:
			return false;
		case details::connection_result::success:
			return true;
		case details::connection_result::busy: /* retry */
		default:
			break;
		}

		Sleep(static_cast<DWORD>(refresh_rate));

		time_now = std::chrono::system_clock::now();
		diff = std::chrono::duration_cast<ms>(time_now - time_started);
	}

	std::wcout << "Timed out while connecting to " << m_name << std::endl;
	return false;
}

void LPCPipeClient::disconnect()
{
	if (isConnected())
	{
		CloseHandle(m_pipe);
		m_overlapped.interrupt();
		m_pipe = INVALID_HANDLE_VALUE;
	}
}

MessageSender LPCPipeClient::getSender()
{
	lock();
	return MessageSender(*this);
}

bool LPCPipeClient::isConnected() const
{
	return m_pipe != INVALID_HANDLE_VALUE;
}

std::pair<void*, size_t> LPCPipeClient::get_buffer()
{
	auto& request_ref = const_cast<std::vector<char>&>(m_request_buffer);
	transfered_pipe_message* request_message = reinterpret_cast<transfered_pipe_message*>(request_ref.data());

	return{	&request_message->payload[0], request_ref.size() - sizeof(transfered_pipe_message) };
}

bool LPCPipeClient::initialize()
{
	try
	{
		m_request_buffer.resize(DEFAULT_PIPE_BUFFER_SIZE);
	}
	catch (std::exception& e)
	{
		std::cout << "Exception occurred: " << e.what() << std::endl;
		return false;
	}

	return true;
}
