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
#include <functional>
#include <vector>
#include <thread>
#include <mutex>
#include "utils.h"
#include "serialize_iterator.h"
#include "deserialize_iterator.h"

namespace SampleService
{
	namespace details
	{
		enum class connection_control : uint32_t
		{
			keep_connection = 0,
			disconnect,
			remote_disconnected,

			max_enum_value
		};

		enum class connection_result : uint32_t
		{
			success = 0,
			failure,
			busy,
			interrupt,

			max_enum_value
		};

		inline void serialize_impl(SerializeIterator& /*it*/)
		{
			/* end of variadic recursion */
		}

		template <typename T, typename ... ARGS>
		void serialize(SerializeIterator& it, const T& head)
		{
			it.put(head);
		}

		template <typename T, typename ... ARGS>
		void serialize_impl(SerializeIterator& it, const T& head, const ARGS&... tail)
		{
			serialize(it, head);
			serialize_impl(it, tail...);
		}
	}

	class LPCPipeContext : public Utils::NonCopyable
	{
		HANDLE m_pipe;
		bool m_impersonated{ false };
	public:
		LPCPipeContext(HANDLE pipe) :
			m_pipe(pipe)
		{
		}

		~LPCPipeContext()
		{
			revertToSelf();
		}

		bool impersonate();
		bool revertToSelf();
	};

	typedef void (t_incoming_message_cbk)(
		DeserializeIterator& request,
		SerializeIterator& reply,
		LPCPipeContext& ctx);

	static constexpr size_t DEFAULT_PIPE_BUFFER_SIZE = 32 * 1024;

	class LPCPipeListener;
	class LPCPipeServer : public Utils::NonCopyable
	{
		std::wstring m_name;
		std::function<t_incoming_message_cbk> m_incoming_message_callback;
		Utils::InterruptableOverlapped m_overlapped;
		size_t m_max_instances{ PIPE_UNLIMITED_INSTANCES };
		std::thread m_accepter;
		bool m_running{ false };
		const bool m_allow_non_admin;
		std::vector<LPCPipeListener*> m_listeners;
		mutable std::mutex m_stopping_mutex;

		void accepterThread();

		using accept_result = std::pair<details::connection_result, HANDLE>;
		accept_result accept() const;

		void checkFinishedListeners();
		void stopAllListeners();

	public:

		LPCPipeServer(
			const std::wstring& name,
			std::function<t_incoming_message_cbk> cbk,
			bool allow_user = false,
			size_t max_instances = PIPE_UNLIMITED_INSTANCES)
			: m_name(name)
			, m_incoming_message_callback(cbk)
			, m_allow_non_admin(allow_user)
			, m_max_instances(max_instances)
		{}

		~LPCPipeServer();

		const std::function<t_incoming_message_cbk>& callback() const;

		bool start();

		void stop();

		const std::wstring& pipeName() const;

		bool isRunning() const;
	};

	class LPCPipeListener : public Utils::NonCopyable
	{
		HANDLE m_pipe{ INVALID_HANDLE_VALUE };
		std::vector<char> m_request_buffer;
		std::vector<char> m_reply_buffer;
		Utils::InterruptableOverlapped m_overlapped;
		std::thread m_thread;
		const LPCPipeServer& m_server;

		details::connection_control receive();
		void listenerThread();

		bool initialize();

		void disconnect();

	public:

		LPCPipeListener(
			HANDLE pipe, const LPCPipeServer& server) :
			m_pipe(pipe),
			m_server(server)
		{
			m_thread = std::thread(&LPCPipeListener::listenerThread, this);
		}

		void interrupt() const
		{
			m_overlapped.interrupt();
		};

		void join()
		{
			if (m_thread.joinable())
			{
				m_thread.join();
			};
		}

		bool isConnected() const
		{
			return m_pipe != INVALID_HANDLE_VALUE;
		}
	};


	class MessageSender;
	class LPCPipeClient : public Utils::NonCopyable
	{
		std::wstring m_name;
		HANDLE m_pipe{ INVALID_HANDLE_VALUE };
		std::vector<char> m_request_buffer;
		std::vector<char> m_reply_buffer;
		std::mutex m_mutex;
		Utils::InterruptableOverlapped m_overlapped;

	private:

		bool internalSend(details::connection_control control, uint32_t size, uint32_t& reply_size) const;

		details::connection_result internalConnect();

		bool send(uint32_t request_size, uint32_t& reply_size) const;

		void lock()
		{ 
			m_mutex.lock();
		}
		void unlock()
		{ 
			m_mutex.unlock();
		}

		std::pair<void*, size_t> get_buffer();
		bool initialize();

	public:

		LPCPipeClient(const std::wstring& name) :
			m_name(name)
		{
		}

		~LPCPipeClient()
		{
			disconnect();
		}

		bool connect(size_t timeout, size_t refresh_rate = 1 /* ms */);
		void disconnect();

		bool isConnected() const;

		MessageSender getSender();

		friend class MessageSender;

		const std::wstring& pipeName() const { return m_name; };
	};

	class MessageSender
	{
		LPCPipeClient& m_transport;

	public:
		MessageSender(const MessageSender&) = delete;
		MessageSender& operator=(const MessageSender&) = delete;

		MessageSender(MessageSender&&) noexcept;
		MessageSender& operator=(MessageSender&&) = delete;

		MessageSender(LPCPipeClient& transport) : m_transport(transport)
		{}
		~MessageSender()
		{ 
			m_transport.unlock();
		}

		template <typename ... ARGS>
		DeserializeIterator send(ARGS ... args) const
		{
			const auto bufs = m_transport.get_buffer();
			if (!bufs.first || !bufs.second) return{ nullptr, 0 };

			unsigned long message_size = 0;
			SerializeIterator it(bufs.first, bufs.second, &message_size);

			details::serialize_impl(it, args...);

			uint32_t reply_size = 0;
			if (!m_transport.send(message_size, reply_size))
			{
				return DeserializeIterator(nullptr, 0);
			}

			return DeserializeIterator(bufs.first, reply_size);
		}
	};

	template<typename ... ARGS>
	void push_message(SerializeIterator& it, ARGS ... args)
	{
		details::serialize_impl(it, args...);
	}
}
